//===--- CGExpr.cpp - Expression Code Generation Visitor -----*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Expression code generation visitor implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/CGExpr.h"
#include "ast.h"

namespace emlang {
namespace codegen {

CGExpr::CGExpr(ContextManager& contextManager, ValueMap& valueMap, CodegenErrorReporter& errorReporter)
    : CGBase(contextManager, valueMap, errorReporter), currentExpressionType("") {
}

void CGExpr::visit(Program& node) {
    // CGExpr doesn't handle Program nodes - this is for specialized expression generation
    error("CGExpr cannot handle Program nodes");
}

void CGExpr::visit(LiteralExpr& node) {
    switch (node.literalType) {
        case LiteralType::INT:
            // Integer literal
            try {
                int intValue = std::stoi(node.value);
                currentValue = llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(32, intValue, true));
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
                currentValue = llvm::ConstantFP::get(contextManager.getContext(), llvm::APFloat(floatValue));
                currentExpressionType = "f64";
            } catch (const std::exception& e) {
                error(CodegenErrorType::TypeMismatch, "Invalid floating point number: " + node.value);
                return;
            }
            break;
        case LiteralType::STR:
            currentValue = contextManager.getBuilder().CreateGlobalStringPtr(node.value, "str");
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
            
            currentValue = llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(32, static_cast<int>(charValue), true));
            currentExpressionType = "char";
            break;
        }
        case LiteralType::BOOL:
            currentValue = llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(1, node.value == "true" ? 1 : 0, false));
            currentExpressionType = "bool";
            break;
        case LiteralType::NULL_LITERAL:
            currentValue = llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(contextManager.getContext()));
            currentExpressionType = "null";
            break;
        default:
            error(CodegenErrorType::UnknownType, "Unknown literal type");
            break;
    }
}

void CGExpr::visit(IdentifierExpr& node) {
    llvm::Value* value = valueMap.getVariable(node.name);
    if (!value) {
        error(CodegenErrorType::UndefinedSymbol, "Unknown variable name: " + node.name);
        return;
    }
    
    // Set the current expression type from value map for proper type tracking
    currentExpressionType = valueMap.getVariableType(node.name);
    
    // Load the value using context manager's builder with proper type
    llvm::Type* loadType = value->getType();
    currentValue = contextManager.getBuilder().CreateLoad(loadType, value, node.name);
}

void CGExpr::visit(BinaryOpExpr& node) {
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
    auto& builder = contextManager.getBuilder();
    
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

void CGExpr::visit(UnaryOpExpr& node) {
    // Generate operand
    node.operand->accept(*this);
    llvm::Value* operandValue = currentValue;
    
    if (!operandValue) {
        error(CodegenErrorType::InternalError, "Invalid operand in unary expression");
        return;
    }
    
    auto& builder = contextManager.getBuilder();
    
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

void CGExpr::visit(AssignmentExpr& node) {
    // First, determine where we're storing the value (target)
    llvm::Value* targetPtr = nullptr;
    
    if (auto* identExpr = dynamic_cast<IdentifierExpr*>(node.target.get())) {
        // Target is a simple variable
        targetPtr = valueMap.getVariable(identExpr->name);
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
    auto& builder = contextManager.getBuilder();
    builder.CreateStore(valueToStore, targetPtr);
    
    // The value of the assignment expression is the value assigned
    currentValue = valueToStore;
}

void CGExpr::visit(FunctionCallExpr& node) {
    // Look up function using context manager
    llvm::Function* calleeF = contextManager.getModule()->getFunction(node.functionName);
    
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
    auto& builder = contextManager.getBuilder();
    if (calleeF->getReturnType()->isVoidTy()) {
        currentValue = builder.CreateCall(calleeF, argsV);
        currentExpressionType = "void";
    } else {
        currentValue = builder.CreateCall(calleeF, argsV, "calltmp");
        // Set the expression type to a default for now - type conversion needs improvement
        currentExpressionType = "i32"; // Fallback type
    }
}

void CGExpr::visit(MemberExpr& node) {
    // Generate object expression
    node.object->accept(*this);
    llvm::Value* objectValue = currentValue;
    
    if (!objectValue) {
        error(CodegenErrorType::InternalError, "Invalid object in member access");
        return;
    }
    
    // TODO: Implement proper struct/object member access
    error(CodegenErrorType::UnknownType, "Member access not yet fully implemented");
}

#ifdef EMLANG_FEATURE_CASTING
void CGExpr::visit(CastExpr& node) {
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
    llvm::Type* targetLLVMType = valueMap.getLLVMType(node.targetType, contextManager);
    
    if (!targetLLVMType) {
        error(CodegenErrorType::UnknownType, "Unknown target type in cast: " + node.targetType);
        return;
    }
    
    auto& builder = contextManager.getBuilder();
    
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

void CGExpr::visit(IndexExpr& node) {
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
    
    auto& builder = contextManager.getBuilder();
    
    // Create GEP (GetElementPtr) instruction for array access
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(32, 0, true))); // First index (array base)
    indices.push_back(indexValue); // Second index (element)
    
    // Get array element type
    llvm::Type* elementType = arrayValue->getType();
    if (auto ptrType = llvm::dyn_cast<llvm::PointerType>(elementType)) {
        // In LLVM 20+, use opaque pointers - we need type information from our type system
        elementType = valueMap.getLLVMType("i32", contextManager); // Default fallback
        if (!arrayType.empty() && arrayType.back() == ']') {
            size_t bracketPos = arrayType.find('[');
            if (bracketPos != std::string::npos) {
                std::string baseType = arrayType.substr(0, bracketPos);
                elementType = valueMap.getLLVMType(baseType, contextManager);
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

void CGExpr::visit(ArrayExpr& node) {
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
    llvm::Type* llvmElementType = valueMap.getLLVMType(elementType, contextManager);
    if (!llvmElementType) {
        llvmElementType = llvm::Type::getInt32Ty(contextManager.getContext());
    }
    
    // Create array type
    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvmElementType, elementValues.size());
    
    // Create alloca for array
    auto& builder = contextManager.getBuilder();
    llvm::Function* currentFunction = builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::Value* arrayAlloca = tmpBuilder.CreateAlloca(arrayType, nullptr, "arraytmp");
    
    // Store each element
    for (size_t i = 0; i < elementValues.size(); ++i) {
        std::vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(32, 0, true)));
        indices.push_back(llvm::ConstantInt::get(contextManager.getContext(), llvm::APInt(32, i, true)));
        
        llvm::Value* elementPtr = builder.CreateGEP(arrayType, arrayAlloca, indices, "arrayelem");
        builder.CreateStore(elementValues[i], elementPtr);
    }
    
    currentValue = arrayAlloca;
    currentExpressionType = elementType + "[" + std::to_string(elementValues.size()) + "]";
}

void CGExpr::visit(ObjectExpr& node) {
    // TODO: Implement proper object literal support
    error(CodegenErrorType::UnknownType, "Object literals not yet implemented");
}

#ifdef EMLANG_FEATURE_POINTERS
void CGExpr::visit(DereferenceExpr& node) {
    // Visit the operand to get the pointer value
    node.operand->accept(*this);
    llvm::Value* ptrValue = currentValue;
    std::string operandType = currentExpressionType;
    
    if (!ptrValue) {
        error(CodegenErrorType::InternalError, "Invalid pointer value for dereference");
        return;
    }
    
    // Determine the element type from the pointer type using value map
    llvm::Type* elementType = valueMap.getElementTypeFromPointer(ptrValue, operandType, contextManager);
    
    if (!elementType) {
        // Fallback to int32 for backward compatibility
        elementType = llvm::Type::getInt32Ty(contextManager.getContext());
        currentExpressionType = "i32";
    } else {
        // Update current expression type to the pointee type using value map
        currentExpressionType = valueMap.getPointeeType(operandType);
    }
    
    // Create load instruction to dereference pointer using context manager's builder
    auto& builder = contextManager.getBuilder();
    currentValue = builder.CreateLoad(elementType, ptrValue, "deref");
}

void CGExpr::visit(AddressOfExpr& node) {
    // For address-of operation, we need the address of a variable
    if (auto identifier = dynamic_cast<IdentifierExpr*>(node.operand.get())) {
        llvm::Value* value = valueMap.getVariable(identifier->name);
        if (value) {
            // value map stores alloca instructions (addresses)
            currentValue = value;
            // Set the expression type to pointer of the variable type using value map
            std::string variableType = valueMap.getVariableType(identifier->name);
            currentExpressionType = variableType + "*";
        } else {
            error(CodegenErrorType::UndefinedSymbol, "Undefined variable for address-of: " + identifier->name);
        }
    } else {
        error(CodegenErrorType::TypeMismatch, "Address-of operation only supported for variables");
    }
}
#endif // EMLANG_FEATURE_POINTERS

void CGExpr::setCurrentValue(llvm::Value* value, const std::string& type) {
    currentValue = value;
    currentExpressionType = type;
}

} // namespace codegen
} // namespace emlang
