//===--- CGStmt.cpp - Statement Code Generation -----------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Statement code generation implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/CGStmt.h"
#include "codegen/CGExpr.h"
#include "ast.h"

namespace emlang {
namespace codegen {

CGStmt::CGStmt(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter, CGExpr& exprGenerator)
    : CGBase(contextManager, valueMap, errorReporter), 
      exprGenerator(exprGenerator), currentBlock(nullptr) {
}

void CGStmt::visit(BlockStmt& node) {
    generateBlock(node);
}

void CGStmt::visit(IfStmt& node) {
    generateIf(node);
}

void CGStmt::visit(WhileStmt& node) {
    generateWhile(node);
}

void CGStmt::visit(ForStmt& node) {
    generateFor(node);
}

void CGStmt::visit(ReturnStmt& node) {
    generateReturn(node);
}

void CGStmt::visit(ExpressionStmt& node) {
    generateExpressionStmt(node);
}

llvm::BasicBlock* CGStmt::generateBlock(BlockStmt& node) {
    auto prevNamedValues = valueMap.saveScope(); // Enter new scope for block
    
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        
        // Check if we've already generated a terminator
        if (contextManager.getBuilder().GetInsertBlock()->getTerminator()) {
            break; // Stop generating code after return, etc.
        }
    }
    valueMap.restoreScope(prevNamedValues); // Exit scope for block
    return contextManager.getBuilder().GetInsertBlock();
}

llvm::BasicBlock* CGStmt::generateIf(IfStmt& node) {
    // Generate condition
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;    if (!condV) {
        error(CodegenErrorType::InternalError, "Invalid condition in if statement");
        return nullptr;
    }
    // Convert condition to boolean - simplified approach
    auto& builder = contextManager.getBuilder();
    if (condV->getType() != llvm::Type::getInt1Ty(contextManager.getContext())) {
        condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "tobool");
    }
    
    // Create basic blocks
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(contextManager.getContext(), "then", function);
    llvm::BasicBlock* elseBB = node.elseBranch ? llvm::BasicBlock::Create(contextManager.getContext(), "else") : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(contextManager.getContext(), "ifcont");
    
    // Branch
    if (elseBB) {
        builder.CreateCondBr(condV, thenBB, elseBB);
    } else {
        builder.CreateCondBr(condV, thenBB, mergeBB);
    }
    
    // Generate then block
    builder.SetInsertPoint(thenBB);
    node.thenBranch->accept(*this);
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(mergeBB);
    }
    thenBB = builder.GetInsertBlock(); // Update in case of nested blocks
    
    // Generate else block if present
    if (elseBB) {
        function->insert(function->end(), elseBB);
        builder.SetInsertPoint(elseBB);
        node.elseBranch->accept(*this);
        if (!builder.GetInsertBlock()->getTerminator()) {
            builder.CreateBr(mergeBB);
        }
        elseBB = builder.GetInsertBlock();
    }
    // Generate merge block
    function->insert(function->end(), mergeBB);
    builder.SetInsertPoint(mergeBB);
    return mergeBB;
}

llvm::BasicBlock* CGStmt::generateWhile(WhileStmt& node) {
    auto& builder = contextManager.getBuilder();
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(contextManager.getContext(), "whilecond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(contextManager.getContext(), "whileloop", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(contextManager.getContext(), "afterloop", function);
    
    // Jump to condition
    builder.CreateBr(condBB);
    
    // Generate condition block
    builder.SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;    
    if (!condV) {
        error(CodegenErrorType::InternalError, "Invalid condition in while statement");
        return nullptr;
    }
    // Convert condition to boolean - simplified approach
    if (condV->getType() != llvm::Type::getInt1Ty(contextManager.getContext())) {
        condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "tobool");
    }
    
    // Branch to loop or after
    builder.CreateCondBr(condV, loopBB, afterBB);
    // Generate loop body
    builder.SetInsertPoint(loopBB);
    auto prevNamedValues = valueMap.saveScope(); // Enter scope for loop body
    node.body->accept(*this);
    valueMap.restoreScope(prevNamedValues); // Exit scope for loop body
    
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(condBB); // Jump back to condition
    }
      // Continue after loop
    builder.SetInsertPoint(afterBB);
    return afterBB;
}

llvm::BasicBlock* CGStmt::generateFor(ForStmt& node) {
    auto& builder = contextManager.getBuilder();
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* initBB = llvm::BasicBlock::Create(contextManager.getContext(), "forinit", function);
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(contextManager.getContext(), "forcond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(contextManager.getContext(), "forloop", function);
    llvm::BasicBlock* incBB = llvm::BasicBlock::Create(contextManager.getContext(), "forinc", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(contextManager.getContext(), "afterfor", function);
    
    // Enter new scope for loop variables
    auto prevNamedValues = valueMap.saveScope();
    
    // Jump to initialization
    builder.CreateBr(initBB);
    
    // Generate initialization
    builder.SetInsertPoint(initBB);
    if (node.initializer) {
        node.initializer->accept(*this);
    }
    builder.CreateBr(condBB);
    
    // Generate condition block
    builder.SetInsertPoint(condBB);
    llvm::Value* condV = nullptr;
    if (node.condition) {
        node.condition->accept(*this);
        condV = currentValue;
        if (condV && condV->getType() != llvm::Type::getInt1Ty(contextManager.getContext())) {
            condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "forcond");
        }
    } else {
        // Infinite loop if no condition
        condV = llvm::ConstantInt::getTrue(contextManager.getContext());
    }
    
    if (condV) {
        builder.CreateCondBr(condV, loopBB, afterBB);
    } else {
        builder.CreateBr(afterBB);
    }
    
    // Generate loop body
    builder.SetInsertPoint(loopBB);
    if (node.body) {
        node.body->accept(*this);
    }
    
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(incBB);
    }
    
    // Generate increment
    builder.SetInsertPoint(incBB);
    if (node.increment) {
        node.increment->accept(*this);
    }
    builder.CreateBr(condBB);
    
    // Continue after loop
    builder.SetInsertPoint(afterBB);
      // Exit scope for loop variables
    valueMap.restoreScope(prevNamedValues);
    return afterBB;
}

llvm::Value* CGStmt::generateReturn(ReturnStmt& node) {
    auto& builder = contextManager.getBuilder();
    
    if (node.value) {
        node.value->accept(*this);        
        if (!currentValue) {
            error(CodegenErrorType::InternalError, "Invalid return expression");
            return nullptr;
        }
        currentValue = builder.CreateRet(currentValue);    
    } else {
        currentValue = builder.CreateRetVoid();
    }
    return currentValue;
}

llvm::Value* CGStmt::generateExpressionStmt(ExpressionStmt& node) {
    node.expression->accept(*this);
    return currentValue;
}

llvm::BasicBlock* CGStmt::createBasicBlock(const std::string& name) {
    // TODO: Implement basic block creation
    error("Basic block creation not yet implemented");
    return nullptr;
}

void CGStmt::ensureBlockTermination(llvm::BasicBlock* block) {
    // TODO: Implement block termination check
    error("Block termination check not yet implemented");
}

bool CGStmt::validateCondition(llvm::Value* conditionValue) {
    // TODO: Implement condition validation
    error("Condition validation not yet implemented");
    return false;
}

} // namespace codegen
} // namespace emlang
