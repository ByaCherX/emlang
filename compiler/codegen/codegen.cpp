//===--- codegen.cpp - Main CodeGenerator Implementation --------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main code generator implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen.h"

#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/GlobalVariable.h>

#include <iostream>

namespace emlang {
namespace codegen {

/******************************
* CONSTRUCTION AND LIFECYCLE
******************************/

CodeGenerator::CodeGenerator(const std::string& moduleName, OptimizationLevel optLevel) 
    : currentValue(nullptr) {
    // Initialize modular components
    contextManager = std::make_unique<ContextManager>(moduleName, optLevel);
    valueMap = std::make_unique<ValueMap>();
    errorReporter = std::make_unique<CodegenErrorReporter>();
    
    currentFunction = nullptr;
    currentExpressionType.clear();
}

/******************************
* PRIMARY CODE GENERATION
******************************/

void CodeGenerator::generateIR(Program& program) {
    program.accept(*this);
    
    // Run optimization passes if requested
    if (contextManager->getOptimizationLevel() != OptimizationLevel::None) {
        contextManager->runOptimizationPasses();
    }
}

void CodeGenerator::printIR() const {
    contextManager->printIR();
}

/******************************
* EXECUTION
******************************/

int CodeGenerator::executeMain() {
    return contextManager->executeMain();
}

void CodeGenerator::writeCodeToFile(const std::string& filename, bool emitLLVM) {
    if (emitLLVM) {
        contextManager->writeIRToFile(filename);
    }
    else {
        contextManager->writeObjectFile(filename);
    }
}

// ========================================================
// AST Visitor implementations
// ========================================================

void CodeGenerator::visit(Program& node) {
    // TODO: Register built-in functions - needs to be implemented
    // BuiltinsIntegration builtins(*contextManager, *valueMap);
    // builtins.registerBuiltinFunctions();
    
    // Process all statements in the program
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    
    // Verify the module - simplified approach
    if (llvm::verifyModule(*contextManager->getModule(), &llvm::errs())) {
        error(CodegenErrorType::InternalError, "Module verification failed");
    }
}

/****************************************
* Visitor - EXPRESSION
****************************************/

void CodeGenerator::visit(LiteralExpr& node) {
    switch (node.literalType) {
        case LiteralType::INT:
            // Integer literal
            try {
                int intValue = std::stoi(node.value);
                currentValue = llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(32, intValue, true));
                currentExpressionType = "i32";
            } catch (const std::exception& e) {
                error(CodegenErrorType::TypeMismatch, "Invalid integer number: " + node.value);
                return;
            }
            break;
        case LiteralType::FLOAT:
            // Floating point literal
            try {
                double floatValue = std::stod(node.value);
                currentValue = llvm::ConstantFP::get(contextManager->getContext(), llvm::APFloat(floatValue));
                currentExpressionType = "f64";
            } catch (const std::exception& e) {
                error(CodegenErrorType::TypeMismatch, "Invalid floating point number: " + node.value);
                return;
            }
            break;
        case LiteralType::STR:
            currentValue = contextManager->getBuilder().CreateGlobalStringPtr(node.value, "str");
            currentExpressionType = "string";
            break;
        case LiteralType::CHAR: {
            uint32_t charValue = 0;
            if (node.value.length() == 1) {
                charValue = static_cast<uint32_t>(node.value[0]);
            } else if (node.value.substr(0, 3) == "\\u{" && node.value.back() == '}') {
                std::string hexCode = node.value.substr(3, node.value.length() - 4);
                try {
                    charValue = std::stoul(hexCode, nullptr, 16);
                } catch (const std::exception& e) {
                    error(CodegenErrorType::TypeMismatch, "Invalid Unicode escape sequence: " + node.value);
                    return;
                }            
            } else if (node.value.length() == 2 && node.value[0] == '\\') {
                switch (node.value[1]) {
                    case 'n': charValue = '\n'; break;
                    case 't': charValue = '\t'; break;
                    case 'r': charValue = '\r'; break;
                    case '\\': charValue = '\\'; break;
                    case '\'': charValue = '\''; break;
                    case '\"': charValue = '\"'; break;
                    case '0': charValue = '\0'; break;
                    default:
                        error(CodegenErrorType::TypeMismatch, "Invalid escape sequence: " + node.value);
                        return;
                }
            } else {
                error(CodegenErrorType::TypeMismatch, "Invalid character literal: " + node.value);
                return;
            }
            
            currentValue = llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(32, static_cast<int>(charValue), true));
            currentExpressionType = "char";
            break;
        }
        case LiteralType::BOOL:
            currentValue = llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(1, node.value == "true" ? 1 : 0, false));
            currentExpressionType = "bool";
            break;
        case LiteralType::NULL_LITERAL:
            currentValue = llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(contextManager->getContext()));
            currentExpressionType = "null";
            break;
        default:
            error(CodegenErrorType::UnknownType, "Unknown literal type");
            break;
    }
}

void CodeGenerator::visit(IdentifierExpr& node) {
    llvm::Value* value = valueMap->getVariable(node.name);
    if (!value) {
        error(CodegenErrorType::UndefinedSymbol, "Unknown variable name: " + node.name);
        return;
    }
    
    // Set the current expression type from value map for proper type tracking
    currentExpressionType = valueMap->getVariableType(node.name);
      // Load the value using context manager's builder with proper type
    llvm::Type* loadType = value->getType()/*->getPointerTo()*/;
    currentValue = contextManager->getBuilder().CreateLoad(loadType, value, node.name);
}

void CodeGenerator::visit(BinaryOpExpr& node) {
    // Generate left operand
    node.left->accept(*this);
    llvm::Value* leftValue = currentValue;
    std::string leftType = currentExpressionType;
    
    // Generate right operand  
    node.right->accept(*this);
    llvm::Value* rightValue = currentValue;
    std::string rightType = currentExpressionType;
    
    if (!leftValue || !rightValue) {
        error(CodegenErrorType::InternalError, "Invalid operands in binary expression");
        return;
    }
    
    // Generate appropriate instruction based on operator
    auto& builder = contextManager->getBuilder();
    
    switch (node.operator_) {
        case BinaryOpExpr::BinOp::ADD:
            currentValue = builder.CreateAdd(leftValue, rightValue, "addtmp");
            break;
        case BinaryOpExpr::BinOp::SUB:
            currentValue = builder.CreateSub(leftValue, rightValue, "subtmp");
            break;
        case BinaryOpExpr::BinOp::MUL:
            currentValue = builder.CreateMul(leftValue, rightValue, "multmp");
            break;
        case BinaryOpExpr::BinOp::DIV:
            currentValue = builder.CreateSDiv(leftValue, rightValue, "divtmp");
            break;
        case BinaryOpExpr::BinOp::MOD:
            currentValue = builder.CreateSRem(leftValue, rightValue, "modtmp");
            break;
        case BinaryOpExpr::BinOp::LT:
            currentValue = builder.CreateICmpSLT(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::GT:
            currentValue = builder.CreateICmpSGT(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::LE:
            currentValue = builder.CreateICmpSLE(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::GE:
            currentValue = builder.CreateICmpSGE(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::EQ:
            currentValue = builder.CreateICmpEQ(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::NE:
            currentValue = builder.CreateICmpNE(leftValue, rightValue, "cmptmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::LAND:
            currentValue = builder.CreateAnd(leftValue, rightValue, "landtmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::LOR:
            currentValue = builder.CreateOr(leftValue, rightValue, "lortmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::AND:
            currentValue = builder.CreateAnd(leftValue, rightValue, "andtmp");
            break;
        case BinaryOpExpr::BinOp::OR:
            currentValue = builder.CreateOr(leftValue, rightValue, "ortmp");
            break;
        case BinaryOpExpr::BinOp::XOR:
            currentValue = builder.CreateXor(leftValue, rightValue, "xortmp");
            break;
        case BinaryOpExpr::BinOp::SHL:
            currentValue = builder.CreateShl(leftValue, rightValue, "shltmp");
            break;
        case BinaryOpExpr::BinOp::SHR:
            currentValue = builder.CreateLShr(leftValue, rightValue, "shrtmp");
            break;
        default:
            error(CodegenErrorType::TypeMismatch, "Unknown binary operator");
            break;
    }
}

void CodeGenerator::visit(UnaryOpExpr& node) {
    // Generate operand
    node.operand->accept(*this);
    llvm::Value* operandValue = currentValue;
    
    if (!operandValue) {
        error(CodegenErrorType::InternalError, "Invalid operand in unary expression");
        return;
    }
    
    auto& builder = contextManager->getBuilder();
    
    switch (node.operator_) {
        case BinaryOpExpr::BinOp::SUB:  // Negation uses SUB operator
            currentValue = builder.CreateNeg(operandValue, "negtmp");
            break;
        case BinaryOpExpr::BinOp::LNOT:  // Logical NOT
            currentValue = builder.CreateNot(operandValue, "nottmp");
            currentExpressionType = "bool";
            break;
        case BinaryOpExpr::BinOp::INV:   // Bitwise NOT
            currentValue = builder.CreateNot(operandValue, "invtmp");
            break;
        default:
            error(CodegenErrorType::TypeMismatch, "Unknown unary operator");
            break;
    }
}

void CodeGenerator::visit(AssignmentExpr& node) {
    // First, determine where we're storing the value (target)
    // This depends on the type of target - it could be a variable name or a dereferenced pointer
    llvm::Value* targetPtr = nullptr;
    
    if (auto* identExpr = dynamic_cast<IdentifierExpr*>(node.target.get())) {
        // Target is a simple variable
        targetPtr = valueMap->getVariable(identExpr->name);
        if (!targetPtr) {
            error(CodegenErrorType::UndefinedSymbol, "Unknown variable name in assignment: " + identExpr->name);
            return;
        }
#ifdef EMLANG_FEATURE_POINTERS
    } else if (auto* derefExpr = dynamic_cast<DereferenceExpr*>(node.target.get())) {
        // Target is a dereference expression (*ptr) - we need to get the pointer value
        derefExpr->operand->accept(*this);
        targetPtr = currentValue;
        if (!targetPtr) {
            error(CodegenErrorType::InternalError, "Invalid pointer dereference in assignment");
            return;
        }
#endif // EMLANG_FEATURE_POINTERS
    } else {
        error(CodegenErrorType::TypeMismatch, "Invalid assignment target type");
        return;
    }
    
    // Now evaluate the right-hand side to get the value to store
    node.value->accept(*this);
    llvm::Value* valueToStore = currentValue;
    if (!valueToStore) {
        error(CodegenErrorType::InternalError, "Invalid expression in assignment");
        return;
    }
    
    // Create a store instruction to assign the value using context manager's builder
    auto& builder = contextManager->getBuilder();
    builder.CreateStore(valueToStore, targetPtr);
    
    // The value of the assignment expression is the value assigned
    currentValue = valueToStore;
}

void CodeGenerator::visit(FunctionCallExpr& node) {
    // Look up function using context manager
    llvm::Function* calleeF = contextManager->getModule()->getFunction(node.functionName);
    
    if (!calleeF) {
        error(CodegenErrorType::UndefinedSymbol, "Unknown function referenced: " + node.functionName);
        return;
    }
    
    // Check argument count
    if (calleeF->arg_size() != node.arguments.size()) {
        error(CodegenErrorType::TypeMismatch, 
              "Incorrect number of arguments passed to " + node.functionName + 
              ": expected " + std::to_string(calleeF->arg_size()) + 
              ", got " + std::to_string(node.arguments.size()));
        return;
    }
    
    // Generate arguments
    std::vector<llvm::Value*> argsV;
    for (auto& arg : node.arguments) {
        arg->accept(*this);
        if (!currentValue) {
            error(CodegenErrorType::InternalError, "Invalid argument in function call");
            return;
        }
        argsV.push_back(currentValue);
    }
    
    // Create call instruction using context manager's builder
    auto& builder = contextManager->getBuilder();
    if (calleeF->getReturnType()->isVoidTy()) {
        currentValue = builder.CreateCall(calleeF, argsV);
        currentExpressionType = "void";
    } else {
        currentValue = builder.CreateCall(calleeF, argsV, "calltmp");
        // Set the expression type to a default for now - type conversion needs improvement
        currentExpressionType = "i32"; // Fallback type
    }
}

void CodeGenerator::visit(MemberExpr& node) {
    // Generate object expression
    node.object->accept(*this);
    llvm::Value* objectValue = currentValue;
    
    if (!objectValue) {
        error(CodegenErrorType::InternalError, "Invalid object in member access");
        return;
    }
    
    // TODO: Implement proper struct/object member access
    // For now, this is a placeholder implementation
    error(CodegenErrorType::UnknownType, "Member access not yet fully implemented");
    currentValue = nullptr;
}

#ifdef EMLANG_FEATURE_CASTING
void CodeGenerator::visit(CastExpr& node) {
    // Generate operand expression
    node.operand->accept(*this);
    llvm::Value* operandValue = currentValue;
    std::string sourceType = currentExpressionType;
    
    if (!operandValue) {
        error(CodegenErrorType::InternalError, "Invalid operand in cast expression");
        return;
    }
    
    // Get source and target LLVM types
    llvm::Type* sourceLLVMType = operandValue->getType();
    llvm::Type* targetLLVMType = valueMap->getLLVMType(node.targetType, *contextManager);
    
    if (!targetLLVMType) {
        error(CodegenErrorType::UnknownType, "Unknown target type in cast: " + node.targetType);
        return;
    }
    
    auto& builder = contextManager->getBuilder();
    
    // Perform appropriate cast based on types
    if (sourceLLVMType == targetLLVMType) {
        // No cast needed
        currentValue = operandValue;
    } else if (sourceLLVMType->isIntegerTy() && targetLLVMType->isIntegerTy()) {
        // Integer to integer cast
        unsigned sourceBits = sourceLLVMType->getIntegerBitWidth();
        unsigned targetBits = targetLLVMType->getIntegerBitWidth();
        
        if (sourceBits < targetBits) {
            // Sign extend
            currentValue = builder.CreateSExt(operandValue, targetLLVMType, "sext");
        } else if (sourceBits > targetBits) {
            // Truncate
            currentValue = builder.CreateTrunc(operandValue, targetLLVMType, "trunc");
        } else {
            // Bitcast
            currentValue = builder.CreateBitCast(operandValue, targetLLVMType, "bitcast");
        }
    } else if (sourceLLVMType->isIntegerTy() && targetLLVMType->isFloatingPointTy()) {
        // Integer to float
        currentValue = builder.CreateSIToFP(operandValue, targetLLVMType, "sitofp");
    } else if (sourceLLVMType->isFloatingPointTy() && targetLLVMType->isIntegerTy()) {
        // Float to integer
        currentValue = builder.CreateFPToSI(operandValue, targetLLVMType, "fptosi");
    } else if (sourceLLVMType->isFloatingPointTy() && targetLLVMType->isFloatingPointTy()) {
        // Float to float
        unsigned sourceBits = sourceLLVMType->getPrimitiveSizeInBits();
        unsigned targetBits = targetLLVMType->getPrimitiveSizeInBits();
        
        if (sourceBits < targetBits) {
            currentValue = builder.CreateFPExt(operandValue, targetLLVMType, "fpext");
        } else if (sourceBits > targetBits) {
            currentValue = builder.CreateFPTrunc(operandValue, targetLLVMType, "fptrunc");
        } else {
            currentValue = operandValue;
        }
    } else if (sourceLLVMType->isPointerTy() && targetLLVMType->isPointerTy()) {
        // Pointer to pointer cast
        currentValue = builder.CreateBitCast(operandValue, targetLLVMType, "ptrcast");
    } else if (sourceLLVMType->isIntegerTy() && targetLLVMType->isPointerTy()) {
        // Integer to pointer (dangerous but allowed in explicit casts)
        currentValue = builder.CreateIntToPtr(operandValue, targetLLVMType, "inttoptr");
    } else if (sourceLLVMType->isPointerTy() && targetLLVMType->isIntegerTy()) {
        // Pointer to integer
        currentValue = builder.CreatePtrToInt(operandValue, targetLLVMType, "ptrtoint");
    } else {
        // Unsupported cast
        error(CodegenErrorType::TypeMismatch, "Unsupported cast from " + sourceType + " to " + node.targetType);
        return;
    }
      currentExpressionType = node.targetType;
}
#endif // EMLANG_FEATURE_CASTING

void CodeGenerator::visit(IndexExpr& node) {
    // Generate array expression
    node.array->accept(*this);
    llvm::Value* arrayValue = currentValue;
    std::string arrayType = currentExpressionType;
    
    // Generate index expression
    node.index->accept(*this);
    llvm::Value* indexValue = currentValue;
    
    if (!arrayValue || !indexValue) {
        error(CodegenErrorType::InternalError, "Invalid array or index in array access");
        return;
    }
    
    auto& builder = contextManager->getBuilder();
    
    // Create GEP (GetElementPtr) instruction for array access
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(32, 0, true))); // First index (array base)
    indices.push_back(indexValue); // Second index (element)
    
    // Get array element type
    llvm::Type* elementType = arrayValue->getType();
    if (auto ptrType = llvm::dyn_cast<llvm::PointerType>(elementType)) {
        // In LLVM 20+, use opaque pointers - we need type information from our type system
        elementType = valueMap->getLLVMType("i32", *contextManager); // Default fallback
        if (!arrayType.empty() && arrayType.back() == ']') {
            size_t bracketPos = arrayType.find('[');
            if (bracketPos != std::string::npos) {
                std::string baseType = arrayType.substr(0, bracketPos);
                elementType = valueMap->getLLVMType(baseType, *contextManager);
            }
        }
    }
    
    // Create GEP and load
    llvm::Value* elementPtr = builder.CreateGEP(elementType, arrayValue, indices, "arrayidx");
    currentValue = builder.CreateLoad(elementType, elementPtr, "arrayload");
    
    // Update expression type - remove array brackets if present
    if (arrayType.back() == ']') {
        size_t bracketPos = arrayType.find('[');
        if (bracketPos != std::string::npos) {
            currentExpressionType = arrayType.substr(0, bracketPos);
        }
    } else {
        currentExpressionType = "i32"; // Fallback type
    }
}

void CodeGenerator::visit(ArrayExpr& node) {
    if (node.elements.empty()) {
        error(CodegenErrorType::TypeMismatch, "Empty array literals not supported");
        return;
    }
    
    // Generate all elements and determine common type
    std::vector<llvm::Value*> elementValues;
    std::string elementType;
    
    for (auto& element : node.elements) {
        element->accept(*this);
        if (currentValue) {
            elementValues.push_back(currentValue);
            if (elementType.empty()) {
                elementType = currentExpressionType;
            }
        } else {
            error(CodegenErrorType::InternalError, "Invalid element in array literal");
            return;
        }
    }
    
    // Get LLVM element type
    llvm::Type* llvmElementType = valueMap->getLLVMType(elementType, *contextManager);
    if (!llvmElementType) {
        llvmElementType = llvm::Type::getInt32Ty(contextManager->getContext());
    }
    
    // Create array type
    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvmElementType, elementValues.size());
    
    // Create alloca for array
    auto& builder = contextManager->getBuilder();
    llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::Value* arrayAlloca = tmpBuilder.CreateAlloca(arrayType, nullptr, "arraytmp");
    
    // Store each element
    for (size_t i = 0; i < elementValues.size(); ++i) {
        std::vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(32, 0, true)));
        indices.push_back(llvm::ConstantInt::get(contextManager->getContext(), llvm::APInt(32, i, true)));
        
        llvm::Value* elementPtr = builder.CreateGEP(arrayType, arrayAlloca, indices, "arrayelem");
        builder.CreateStore(elementValues[i], elementPtr);
    }
    
    currentValue = arrayAlloca;
    currentExpressionType = elementType + "[" + std::to_string(elementValues.size()) + "]";
}

void CodeGenerator::visit(ObjectExpr& node) {
    // TODO: Implement proper object literal support
    // For now, this is a placeholder implementation
    error(CodegenErrorType::UnknownType, "Object literals not yet implemented");
    currentValue = nullptr;
}

#ifdef EMLANG_FEATURE_POINTERS
void CodeGenerator::visit(DereferenceExpr& node) {
    // Visit the operand to get the pointer value
    node.operand->accept(*this);
    llvm::Value* ptrValue = currentValue;
    std::string operandType = currentExpressionType;
    
    if (!ptrValue) {
        error(CodegenErrorType::InternalError, "Invalid pointer value for dereference");
        return;
    }
    
    // Determine the element type from the pointer type using value map
    llvm::Type* elementType = valueMap->getElementTypeFromPointer(ptrValue, operandType, *contextManager);
    
    if (!elementType) {
        // Fallback to int32 for backward compatibility
        elementType = llvm::Type::getInt32Ty(contextManager->getContext());
        currentExpressionType = "i32";
    } else {
        // Update current expression type to the pointee type using value map
        currentExpressionType = valueMap->getPointeeType(operandType);
    }
    
    // Create load instruction to dereference pointer using context manager's builder
    auto& builder = contextManager->getBuilder();
    currentValue = builder.CreateLoad(elementType, ptrValue, "deref");
}

void CodeGenerator::visit(AddressOfExpr& node) {
    // For address-of operation, we need the address of a variable
    if (auto identifier = dynamic_cast<IdentifierExpr*>(node.operand.get())) {
        llvm::Value* value = valueMap->getVariable(identifier->name);
        if (value) {
            // value map stores alloca instructions (addresses)
            currentValue = value;
              // Set the expression type to pointer of the variable type using value map
            std::string variableType = valueMap->getVariableType(identifier->name);
            currentExpressionType = variableType + "*";
        } else {
            error(CodegenErrorType::UndefinedSymbol, "Undefined variable for address-of: " + identifier->name);
            currentValue = nullptr;
        }
    } else {
        error(CodegenErrorType::TypeMismatch, "Address-of operation only supported for variables");
        currentValue = nullptr;
    }
}
#endif // EMLANG_FEATURE_POINTERS

/****************************************
* Visitor - DECLARATIONS
****************************************/

void CodeGenerator::visit(VariableDecl& node) {
    // Get LLVM type using value map
    std::string typeStr = node.type.value_or("i32"); // Default to i32 if no type specified
    llvm::Type* llvmType = valueMap->getLLVMType(typeStr, *contextManager);
    
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
                return;
            }        } else {
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
            *contextManager->getModule(),
            llvmType,
            node.isConstant,
            llvm::GlobalValue::PrivateLinkage,
            initVal,
            node.name
        );
          // Remember the global variable in value map
        valueMap->addVariable(node.name, globalVar, typeStr);
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
            return;
        }
          // Create alloca for the local variable - simplified approach
        llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
        llvm::Value* alloca = tmpBuilder.CreateAlloca(llvmType, nullptr, node.name);
        
        // Store initial value using context manager's builder
        contextManager->getBuilder().CreateStore(initVal, alloca);
          // Remember the variable in value map
        valueMap->addVariable(node.name, alloca, typeStr);
        currentValue = alloca;
    }
}

void CodeGenerator::visit(FunctionDecl& node) {    
    // Create function type using value map
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = valueMap->getLLVMType(param.type, *contextManager);
        if (!paramType) {
            error(CodegenErrorType::UnknownType, "Unknown parameter type: " + param.type + " in function: " + node.name);
            return;
        }
        paramTypes.push_back(paramType);
    }
    
    std::string returnTypeStr = node.returnType.value_or("void"); // Default to void if no return type
    llvm::Type* returnType = valueMap->getLLVMType(returnTypeStr, *contextManager);
    
    if (!returnType) {
        error(CodegenErrorType::UnknownType, "Unknown return type: " + returnTypeStr + " in function: " + node.name);
        return;
    }
    
    // Create function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node.name,
        contextManager->getModule()
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
        contextManager->getContext(),
        "entry",
        function
    );
    contextManager->getBuilder().SetInsertPoint(bb);

    // Save previous state
    llvm::Function* prevFunction = currentFunction;    
    auto prevNamedValues = valueMap->saveScope(); // Use value map scoping
    
    currentFunction = function;
    
    // Create allocas for parameters
    unsigned paramIdx = 0;
    for (auto& arg : function->args()) {
        // Create alloca for parameter - simplified approach
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        llvm::Value* alloca = tmpBuilder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        
        contextManager->getBuilder().CreateStore(&arg, alloca);
        
        // Store parameter in value map
        if (paramIdx < node.parameters.size()) {
            valueMap->addVariable(std::string(arg.getName()), alloca, node.parameters[paramIdx].type);
        }
        paramIdx++;
    }
    
    // Generate function body
    if (node.body) {
        node.body->accept(*this);
        
        // Add return if missing for void functions
        if (returnType->isVoidTy()) {
            contextManager->getBuilder().CreateRetVoid();
        }
    }
    
    // Verify function - simplified approach
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        error(CodegenErrorType::InternalError, "Function verification failed for: " + node.name);
        function->eraseFromParent();
    }
    
    // Restore previous state
    currentFunction = prevFunction;
    valueMap->restoreScope(prevNamedValues); // Restore scope in value map
    currentValue = function;
}

void CodeGenerator::visit(ExternFunctionDecl& node) {
    // Create function type using value map
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        paramTypes.push_back(valueMap->getLLVMType(param.type, *contextManager));
    }
    
    llvm::Type* returnType = valueMap->getLLVMType(node.returnType, *contextManager);
    // Create external function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create external function declaration
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node.name,
        contextManager->getModule()
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
}

/****************************************
* Visitor - STATEMENTS
****************************************/

void CodeGenerator::visit(BlockStmt& node) {
    auto prevNamedValues = valueMap->saveScope(); // Enter new scope for block
    
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        
        // Check if we've already generated a terminator
        if (contextManager->getBuilder().GetInsertBlock()->getTerminator()) {
            break; // Stop generating code after return, etc.
        }
    }
    
    valueMap->restoreScope(prevNamedValues); // Exit scope for block
}

void CodeGenerator::visit(IfStmt& node) {
    // Generate condition
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;
    if (!condV) {
        error(CodegenErrorType::InternalError, "Invalid condition in if statement");
        return;
    }
    // Convert condition to boolean - simplified approach
    auto& builder = contextManager->getBuilder();
    if (condV->getType() != llvm::Type::getInt1Ty(contextManager->getContext())) {
        condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "tobool");
    }
    
    // Create basic blocks
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(contextManager->getContext(), "then", function);
    llvm::BasicBlock* elseBB = node.elseBranch ? llvm::BasicBlock::Create(contextManager->getContext(), "else") : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(contextManager->getContext(), "ifcont");
    
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
}

void CodeGenerator::visit(WhileStmt& node) {    
    auto& builder = contextManager->getBuilder();
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(contextManager->getContext(), "whilecond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(contextManager->getContext(), "whileloop", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(contextManager->getContext(), "afterloop", function);
    
    // Jump to condition
    builder.CreateBr(condBB);
    
    // Generate condition block
    builder.SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;
    if (!condV) {
        error(CodegenErrorType::InternalError, "Invalid condition in while statement");
        return;
    }
    // Convert condition to boolean - simplified approach
    if (condV->getType() != llvm::Type::getInt1Ty(contextManager->getContext())) {
        condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "tobool");
    }
    
    // Branch to loop or after
    builder.CreateCondBr(condV, loopBB, afterBB);
    // Generate loop body
    builder.SetInsertPoint(loopBB);
    auto prevNamedValues = valueMap->saveScope(); // Enter scope for loop body
    node.body->accept(*this);
    valueMap->restoreScope(prevNamedValues); // Exit scope for loop body
    
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(condBB); // Jump back to condition
    }
    
    // Continue after loop
    builder.SetInsertPoint(afterBB);
}

void CodeGenerator::visit(ForStmt& node) {
    auto& builder = contextManager->getBuilder();
    llvm::Function* function = builder.GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* initBB = llvm::BasicBlock::Create(contextManager->getContext(), "forinit", function);
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(contextManager->getContext(), "forcond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(contextManager->getContext(), "forloop", function);
    llvm::BasicBlock* incBB = llvm::BasicBlock::Create(contextManager->getContext(), "forinc", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(contextManager->getContext(), "afterfor", function);
    
    // Enter new scope for loop variables
    auto prevNamedValues = valueMap->saveScope();
    
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
        if (condV && condV->getType() != llvm::Type::getInt1Ty(contextManager->getContext())) {
            condV = builder.CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "forcond");
        }
    } else {
        // Infinite loop if no condition
        condV = llvm::ConstantInt::getTrue(contextManager->getContext());
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
    valueMap->restoreScope(prevNamedValues);
}

void CodeGenerator::visit(ReturnStmt& node) {
    auto& builder = contextManager->getBuilder();
    
    if (node.value) {
        node.value->accept(*this);
        if (!currentValue) {
            error(CodegenErrorType::InternalError, "Invalid return expression");
            return;
        }
        builder.CreateRet(currentValue);
    } else {
        builder.CreateRetVoid();
    }
}

void CodeGenerator::visit(ExpressionStmt& node) {
    node.expression->accept(*this);
}

/******************************
* ERROR HANDLING
******************************/
bool CodeGenerator::hasErrors() const {
    return errorReporter->hasErrors();
}

void CodeGenerator::error(const std::string& message) {
    errorReporter->error(CodegenErrorType::InternalError, message);
    currentValue = nullptr;
}

void CodeGenerator::error(CodegenErrorType type, const std::string& message, const std::string& context) {
    errorReporter->error(type, message, context);
    currentValue = nullptr;
}

} // namespace codegen
} // namespace emlang