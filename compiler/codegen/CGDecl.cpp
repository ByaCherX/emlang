//===--- CGDecl.cpp - Declaration Code Generation ---------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declaration code generation implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/CGDecl.h"
#include "codegen/CGStmt.h"
#include "codegen/CGExpr.h"
#include "ast.h"
#include <llvm/IR/Verifier.h>

namespace emlang {
namespace codegen {

CGDecl::CGDecl(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter, CGStmt* stmtGenerator, CGExpr* exprGenerator)
    : CGBase(contextManager, valueMap, errorReporter), 
    currentFunction(nullptr), currentExpressionType(""), stmtGenerator(stmtGenerator), exprGenerator(exprGenerator) {
}

void CGDecl::visit(VariableDecl& node) {
    generateVariableDecl(node);
}

void CGDecl::visit(FunctionDecl& node) {
    generateFunctionDecl(node);
}

void CGDecl::visit(ExternFunctionDecl& node) {
    generateExternFunctionDecl(node);
}

void CGDecl::setCurrentValue(llvm::Value* value, const std::string& type) {
    currentValue = value;
    currentExpressionType = type;
}

llvm::Value* CGDecl::generateVariableDecl(VariableDecl& node) {
    std::cerr << "DEBUG: CGDecl generating VariableDecl: " << node.name << std::endl;
    // Get LLVM type using value map
    std::string typeStr = node.type.value_or("i32"); // Default to i32 if no type specified
    llvm::Type* llvmType = valueMap.getLLVMType(typeStr, contextManager);
    std::cerr << "DEBUG: Variable type: " << typeStr << std::endl;
    
    // Track the type for multi-level pointer support
    currentExpressionType = typeStr;
    
    // Check if we're in global scope (no current function)
    if (!currentFunction) {
        std::cerr << "DEBUG: Creating global variable" << std::endl;
        // Global variable
        llvm::Constant* initVal = nullptr;
        if (node.initializer) {
            node.initializer->accept(*this);
            // Convert currentValue to constant if it's not already
            if (auto constVal = llvm::dyn_cast<llvm::Constant>(currentValue)) {
                initVal = constVal;
            } else {
                error(CodegenErrorType::InternalError, 
                      "Global variable initializer must be a constant: " + node.name);
                return nullptr;
            }
        } else {
            // Default initialization for global variables - use null constants for now
            if (llvmType->isPointerTy()) {
                initVal = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(llvmType));
            } else if (llvmType->isIntegerTy()) {
                initVal = llvm::ConstantInt::get(llvmType, 0);
            } else if (llvmType->isFloatingPointTy()) {
                initVal = llvm::ConstantFP::get(llvmType, 0.0);
            } else {
                initVal = llvm::UndefValue::get(llvmType);
            }
        }          
        // Create global variable directly using LLVM
        auto globalVar = new llvm::GlobalVariable(
            *contextManager.getModule(),
            llvmType,
            node.isConstant,
            llvm::GlobalValue::PrivateLinkage,
            initVal,
            node.name
        );
        // Remember the global variable in value map
        valueMap.addVariable(node.name, globalVar, typeStr);
        currentValue = globalVar;
    } else {
        std::cerr << "DEBUG: Creating local variable" << std::endl;
        // Local variable
        llvm::Value* initVal = nullptr;
        if (node.initializer) {
            std::cerr << "DEBUG: Processing variable initializer" << std::endl;
            if (exprGenerator) {
                node.initializer->accept(*exprGenerator);
                initVal = exprGenerator->getCurrentValue();
            } else {
                node.initializer->accept(*this);
                initVal = currentValue;
            }
            std::cerr << "DEBUG: Initializer processed" << std::endl;
        } 
        else {
            std::cerr << "DEBUG: Using default initialization" << std::endl;
            // Default initialization for local variables - use null/zero constants
            if (llvmType->isPointerTy()) {
                initVal = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(llvmType));
            } else if (llvmType->isIntegerTy()) {
                initVal = llvm::ConstantInt::get(llvmType, 0);
            } else if (llvmType->isFloatingPointTy()) {
                initVal = llvm::ConstantFP::get(llvmType, 0.0);
            } else {
                initVal = llvm::UndefValue::get(llvmType);
            }
        }
        if (!initVal) {
            std::cerr << "DEBUG: ERROR - Failed to generate initial value" << std::endl;
            error(CodegenErrorType::InternalError, 
                  "Failed to generate initial value for variable: " + node.name);
            return nullptr;
        } else {
            std::cerr << "DEBUG: Initial value generated successfully" << std::endl;
        }
          
        // Create alloca for the local variable - simplified approach
        llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
        llvm::Value* alloca = tmpBuilder.CreateAlloca(llvmType, nullptr, node.name);
        std::cerr << "DEBUG: Created alloca for variable: " << node.name << std::endl;
        
        // Store initial value using context manager's builder
        contextManager.getBuilder().CreateStore(initVal, alloca);
        std::cerr << "DEBUG: Stored initial value" << std::endl;
        // Remember the variable in value map
        valueMap.addVariable(node.name, alloca, typeStr);
        std::cerr << "DEBUG: Added variable to value map: " << node.name << std::endl;
        currentValue = alloca;
    }
    return currentValue;
}

llvm::Function* CGDecl::generateFunctionDecl(FunctionDecl& node) {
    // Create function type using value map
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = valueMap.getLLVMType(param.type, contextManager);        if (!paramType) {
            error(CodegenErrorType::UnknownType, "Unknown parameter type: " + param.type + " in function: " + node.name);
            return nullptr;
        }
        paramTypes.push_back(paramType);
    }
    
    std::string returnTypeStr = node.returnType.value_or("void"); // Default to void if no return type
    llvm::Type* returnType = valueMap.getLLVMType(returnTypeStr, contextManager);
    if (!returnType) {
        error(CodegenErrorType::UnknownType, "Unknown return type: " + returnTypeStr + " in function: " + node.name);
        return nullptr;
    }
    
    // Create function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node.name,
        contextManager.getModule()
    );
    
    // Set parameter names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        if (idx < node.parameters.size()) {
            arg.setName(node.parameters[idx].name);
        }
        idx++;
    }
      // Create entry basic block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(
        contextManager.getContext(),
        "entry",
        function
    );
    contextManager.getBuilder().SetInsertPoint(bb);

    // Save previous state
    llvm::Function* prevFunction = currentFunction;    
    auto prevNamedValues = valueMap.saveScope(); // Use value map scoping
    
    currentFunction = function;
    
    // Create allocas for parameters
    unsigned paramIdx = 0;
    for (auto& arg : function->args()) {
        // Create alloca for parameter - simplified approach
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        llvm::Value* alloca = tmpBuilder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        
        contextManager.getBuilder().CreateStore(&arg, alloca);
        
        // Store parameter in value map
        if (paramIdx < node.parameters.size()) {
            valueMap.addVariable(std::string(arg.getName()), alloca, node.parameters[paramIdx].type);
        }
        paramIdx++;
    }
    
    // Generate function body
    if (node.body) {
        std::cerr << "DEBUG: Generating function body for: " << node.name << std::endl;
        if (stmtGenerator) {
            // Set the current function in statement generator (if it needs it)
            std::cerr << "DEBUG: Using statement generator" << std::endl;
            node.body->accept(*stmtGenerator);
            std::cerr << "DEBUG: Function body generation completed" << std::endl;
        } else {
            // Fallback to self - this shouldn't normally happen since we need CGStmt for proper statement handling
            std::cerr << "DEBUG: ERROR - Statement generator not available" << std::endl;
            error(CodegenErrorType::InternalError, "Statement generator not available for function body: " + node.name);
            return nullptr;
        }
        
        // Add return if missing for void functions
        if (returnType->isVoidTy()) {
            // Check if the last instruction is already a terminator
            llvm::BasicBlock* currentBB = contextManager.getBuilder().GetInsertBlock();
            if (!currentBB->getTerminator()) {
                contextManager.getBuilder().CreateRetVoid();
            }
        }
    }
    
    // Verify function - simplified approach
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        std::cerr << "DEBUG: Function verification failed for: " << node.name << std::endl;
        // For debugging, don't erase the function immediately
        // function->eraseFromParent();
        // Instead, let's see what's in the function
        function->print(llvm::errs());
    } else {
        std::cerr << "DEBUG: Function verification passed for: " << node.name << std::endl;
    }
      // Restore previous state
    currentFunction = prevFunction;
    valueMap.restoreScope(prevNamedValues); // Restore scope in value map
    currentValue = function;
    return function;
}

llvm::Function* CGDecl::generateExternFunctionDecl(ExternFunctionDecl& node) {
    // Create function type using value map
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        paramTypes.push_back(valueMap.getLLVMType(param.type, contextManager));
    }
    
    llvm::Type* returnType = valueMap.getLLVMType(node.returnType, contextManager);
    // Create external function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create external function declaration
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node.name,
        contextManager.getModule()
    );
    
    // Set parameter names for better IR readability
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        if (idx < node.parameters.size()) {
            arg.setName(node.parameters[idx].name);
        }
        idx++;
    }
      currentValue = function;
    return function;
}

void CGDecl::createFunctionParameters(llvm::Function* function, FunctionDecl& node) {
    // TODO: Implement function parameter creation
    error("Function parameter creation not yet implemented");
}

bool CGDecl::validateFunctionSignature(FunctionDecl& node) {
    // TODO: Implement function signature validation
    error("Function signature validation not yet implemented");
    return false;
}

bool CGDecl::validateExternFunctionDecl(ExternFunctionDecl& node) {
    // TODO: Implement extern function declaration validation
    error("Extern function declaration validation not yet implemented");
    return false;
}

} // namespace codegen
} // namespace emlang
