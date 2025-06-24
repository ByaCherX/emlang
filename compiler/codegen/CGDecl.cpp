//===--- CGDecl.cpp - Declaration Code Generation ---------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Declaration code generation implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/CGDecl.h"
#include "ast.h"
#include <llvm/IR/Verifier.h>

namespace emlang {
namespace codegen {

CGDecl::CGDecl(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter)
    : CGBase(contextManager, valueMap, errorReporter), 
    currentFunction(nullptr), currentExpressionType("") {
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
    // Get LLVM type using value map
    std::string typeStr = node.type.value_or("i32"); // Default to i32 if no type specified
    llvm::Type* llvmType = valueMap.getLLVMType(typeStr, contextManager);
    
    // Track the type for multi-level pointer support
    currentExpressionType = typeStr;
    
    // Check if we're in global scope (no current function)
    if (!currentFunction) {
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
        // Local variable
        llvm::Value* initVal = nullptr;
        if (node.initializer) {
            node.initializer->accept(*this);
            initVal = currentValue;        
        } 
        else {
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
            error(CodegenErrorType::InternalError, 
                  "Failed to generate initial value for variable: " + node.name);
            return nullptr;
        }
          // Create alloca for the local variable - simplified approach
        llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
        llvm::Value* alloca = tmpBuilder.CreateAlloca(llvmType, nullptr, node.name);
        
        // Store initial value using context manager's builder
        contextManager.getBuilder().CreateStore(initVal, alloca);          // Remember the variable in value map
        valueMap.addVariable(node.name, alloca, typeStr);
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
        node.body->accept(*this);
        
        // Add return if missing for void functions
        if (returnType->isVoidTy()) {
            contextManager.getBuilder().CreateRetVoid();
        }
    }
    
    // Verify function - simplified approach
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        error(CodegenErrorType::InternalError, "Function verification failed for: " + node.name);
        function->eraseFromParent();
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
