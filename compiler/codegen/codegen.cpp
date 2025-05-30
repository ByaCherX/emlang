// LLVM Code Generator implementation
#include "../include/codegen.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Scalar/TailRecursionElimination.h>
#include <llvm/Transforms/IPO/GlobalOpt.h>
#include <llvm/Transforms/IPO/SCCP.h>
#include <llvm/Transforms/IPO/DeadArgumentElimination.h>
#include <llvm/Transforms/IPO/AlwaysInliner.h>
#include <iostream>
#include <stdexcept>

namespace emlang {

CodeGenerator::CodeGenerator(const std::string& moduleName, OptimizationLevel optLevel) 
    : currentFunction(nullptr), currentValue(nullptr), optimizationLevel(optLevel) {
    // Initialize LLVM
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    
    // Initialize native target for JIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

llvm::Type* CodeGenerator::getLLVMType(const std::string& typeName) {
    // Legacy types for backward compatibility
    if (typeName == "number" || typeName == "int") {
        return llvm::Type::getInt32Ty(*context);
    } else if (typeName == "boolean") {
        return llvm::Type::getInt1Ty(*context);
    } else if (typeName == "string") {
        return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
    } else if (typeName == "void") {
        return llvm::Type::getVoidTy(*context);
    }
    
    // C-style signed integer types
    else if (typeName == "int8") {
        return llvm::Type::getInt8Ty(*context);
    } else if (typeName == "int16") {
        return llvm::Type::getInt16Ty(*context);
    } else if (typeName == "int32") {
        return llvm::Type::getInt32Ty(*context);
    } else if (typeName == "int64") {
        return llvm::Type::getInt64Ty(*context);
    } else if (typeName == "isize") {
        // Platform-dependent: 32-bit on 32-bit systems, 64-bit on 64-bit systems
        return sizeof(void*) == 8 ? llvm::Type::getInt64Ty(*context) : llvm::Type::getInt32Ty(*context);
    }
    
    // C-style unsigned integer types
    else if (typeName == "uint8") {
        return llvm::Type::getInt8Ty(*context);
    } else if (typeName == "uint16") {
        return llvm::Type::getInt16Ty(*context);
    } else if (typeName == "uint32") {
        return llvm::Type::getInt32Ty(*context);
    } else if (typeName == "uint64") {
        return llvm::Type::getInt64Ty(*context);
    } else if (typeName == "usize") {
        // Platform-dependent: 32-bit on 32-bit systems, 64-bit on 64-bit systems
        return sizeof(void*) == 8 ? llvm::Type::getInt64Ty(*context) : llvm::Type::getInt32Ty(*context);
    }
    
    // C-style floating point types
    else if (typeName == "float") {
        return llvm::Type::getFloatTy(*context);
    } else if (typeName == "double") {
        return llvm::Type::getDoubleTy(*context);
    }
    
    // Other types
    else if (typeName == "char") {
        return llvm::Type::getInt32Ty(*context); // Unicode scalar value (U+0000 to U+10FFFF)
    } else if (typeName == "str") {
        return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
    } else if (typeName == "bool") {
        return llvm::Type::getInt1Ty(*context);
    }
    
    // Rust-like unit type
    else if (typeName == "()") {
        return llvm::Type::getVoidTy(*context);
    }
    
    // Check for pointer types (C-style: int32*, char*, etc.)
    else if (typeName.length() > 1 && typeName.back() == '*') {
        std::string baseTypeName = typeName.substr(0, typeName.length() - 1);
        llvm::Type* baseType = getLLVMType(baseTypeName);
        return llvm::PointerType::get(baseType, 0);
    }
    
    error("Unknown type: " + typeName);
    return nullptr;
}

llvm::Type* CodeGenerator::getNumberType() {
    return llvm::Type::getInt32Ty(*context);
}

llvm::Type* CodeGenerator::getStringType() {
    return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
}

llvm::Type* CodeGenerator::getBooleanType() {
    return llvm::Type::getInt1Ty(*context);
}

llvm::Type* CodeGenerator::getPointerType(const std::string& baseTypeName) {
    llvm::Type* baseType = getLLVMType(baseTypeName);
    return llvm::PointerType::get(baseType, 0);
}

// C-style signed integer types
llvm::Type* CodeGenerator::getInt8Type() {
    return llvm::Type::getInt8Ty(*context);
}

llvm::Type* CodeGenerator::getInt16Type() {
    return llvm::Type::getInt16Ty(*context);
}

llvm::Type* CodeGenerator::getInt32Type() {
    return llvm::Type::getInt32Ty(*context);
}

llvm::Type* CodeGenerator::getInt64Type() {
    return llvm::Type::getInt64Ty(*context);
}

llvm::Type* CodeGenerator::getIsizeType() {
    // Platform-dependent pointer-sized integer
    return sizeof(void*) == 8 ? llvm::Type::getInt64Ty(*context) : llvm::Type::getInt32Ty(*context);
}

// C-style unsigned integer types
llvm::Type* CodeGenerator::getUint8Type() {
    return llvm::Type::getInt8Ty(*context);
}

llvm::Type* CodeGenerator::getUint16Type() {
    return llvm::Type::getInt16Ty(*context);
}

llvm::Type* CodeGenerator::getUint32Type() {
    return llvm::Type::getInt32Ty(*context);
}

llvm::Type* CodeGenerator::getUint64Type() {
    return llvm::Type::getInt64Ty(*context);
}

llvm::Type* CodeGenerator::getUsizeType() {
    // Platform-dependent pointer-sized unsigned integer
    return sizeof(void*) == 8 ? llvm::Type::getInt64Ty(*context) : llvm::Type::getInt32Ty(*context);
}

// C-style floating point types
llvm::Type* CodeGenerator::getFloatType() {
    return llvm::Type::getFloatTy(*context);
}

llvm::Type* CodeGenerator::getDoubleType() {
    return llvm::Type::getDoubleTy(*context);
}

// Character type
llvm::Type* CodeGenerator::getCharType() {
    return llvm::Type::getInt32Ty(*context); // Unicode scalar value
}

// String slice type
llvm::Type* CodeGenerator::getStrType() {
    return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0); // str as i8*
}

// Unit type
llvm::Type* CodeGenerator::getUnitType() {
    return llvm::Type::getVoidTy(*context);
}

// Helper methods for type checking
bool CodeGenerator::isSignedInteger(const std::string& typeName) {
    return typeName == "int8" || typeName == "int16" || typeName == "int32" || 
           typeName == "int64" || typeName == "isize" || typeName == "int" || typeName == "number";
}

bool CodeGenerator::isUnsignedInteger(const std::string& typeName) {
    return typeName == "uint8" || typeName == "uint16" || typeName == "uint32" || 
           typeName == "uint64" || typeName == "usize";
}

bool CodeGenerator::isFloatingPoint(const std::string& typeName) {
    return typeName == "float" || typeName == "double";
}

bool CodeGenerator::isPrimitiveType(const std::string& typeName) {
    return isSignedInteger(typeName) || isUnsignedInteger(typeName) || 
           isFloatingPoint(typeName) || typeName == "bool" || typeName == "boolean" ||
           typeName == "char" || typeName == "()" || typeName == "void" || typeName == "string";
}

llvm::Value* CodeGenerator::createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type) {
    llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tempBuilder.CreateAlloca(type, nullptr, varName);
}

void CodeGenerator::generateIR(Program& program) {
    program.accept(*this);
    
    // Run optimization passes if requested
    if (optimizationLevel != OptimizationLevel::None) {
        runOptimizationPasses();
    }
}

llvm::Module* CodeGenerator::getModule() const {
    return module.get();
}

void CodeGenerator::printIR() const {
    module->print(llvm::outs(), nullptr);
}

void CodeGenerator::writeIRToFile(const std::string& filename) const {
    std::error_code errorCode;
    llvm::raw_fd_ostream file(filename, errorCode);
    if (errorCode) {
        error("Could not open file " + filename + " for writing");
        return;
    }
    module->print(file, nullptr);
}

int CodeGenerator::executeMain() {
    // Find main function
    llvm::Function* mainFunc = module->getFunction("main");
    if (!mainFunc) {
        error("No main function found");
        return -1;
    }
    
    // Create execution engine
    std::string errorStr;
    llvm::ExecutionEngine* executionEngine = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module.release()))
        .setErrorStr(&errorStr)
        .setEngineKind(llvm::EngineKind::JIT)
        .create();
    
    if (!executionEngine) {
        error("Could not create execution engine: " + errorStr);
        return -1;
    }
    
    // Execute main function
    std::vector<llvm::GenericValue> args;
    llvm::GenericValue result = executionEngine->runFunction(mainFunc, args);
    
    // Return result as integer
    return (int)result.IntVal.getSExtValue();
}

// AST Visitor implementations
void CodeGenerator::visit(LiteralExpression& node) {
    switch (node.literalType) {
        case LiteralExpression::LiteralType::NUMBER:
            currentValue = llvm::ConstantInt::get(*context, llvm::APInt(32, std::stoi(node.value)));
            break;
        case LiteralExpression::LiteralType::STRING:
            currentValue = builder->CreateGlobalStringPtr(node.value, "str");
            break;
        case LiteralExpression::LiteralType::CHAR: {
            // Handle character literal
            uint32_t charValue = 0;
            if (node.value.length() == 1) {
                // Simple ASCII character
                charValue = static_cast<uint32_t>(node.value[0]);
            } else if (node.value.substr(0, 3) == "\\u{" && node.value.back() == '}') {
                // Unicode escape sequence \u{XXXX}
                std::string hexCode = node.value.substr(3, node.value.length() - 4);
                try {
                    charValue = std::stoul(hexCode, nullptr, 16);
                } catch (const std::exception& e) {
                    error("Invalid Unicode escape sequence: " + node.value);
                    charValue = 0;
                }            
            } else if (node.value.length() == 2 && node.value[0] == '\\') {
                // Escape sequences
                switch (node.value[1]) {
                    case 'n': charValue = '\n'; break;
                    case 't': charValue = '\t'; break;
                    case 'r': charValue = '\r'; break;
                    case '\\': charValue = '\\'; break;
                    case '\'': charValue = '\''; break;
                    case '"': charValue = '"'; break;
                    case '0': charValue = '\0'; break;
                    default:
                        error("Invalid escape sequence: " + node.value);
                        charValue = 0;
                        break;
                }
            } else {
                error("Invalid character literal: " + node.value);
                charValue = 0;
            }
            currentValue = llvm::ConstantInt::get(*context, llvm::APInt(32, charValue));
            break;
        }
        case LiteralExpression::LiteralType::BOOLEAN:
            currentValue = llvm::ConstantInt::get(*context, llvm::APInt(1, node.value == "true" ? 1 : 0));
            break;        case LiteralExpression::LiteralType::NULL_LITERAL:
            currentValue = llvm::Constant::getNullValue(llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0));
            break;
    }
}

void CodeGenerator::visit(IdentifierExpression& node) {
    llvm::Value* value = namedValues[node.name];
    if (!value) {
        error("Unknown variable name: " + node.name);
        return;
    }
    
    // Load the variable (cast to AllocaInst to get the allocated type)
    if (llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(value)) {
        currentValue = builder->CreateLoad(alloca->getAllocatedType(), alloca, node.name);
    } else {
        error("Invalid variable type for: " + node.name);
    }
}

void CodeGenerator::visit(BinaryOpExpression& node) {
    // Generate left operand
    node.left->accept(*this);
    llvm::Value* leftValue = currentValue;
    
    // Generate right operand
    node.right->accept(*this);
    llvm::Value* rightValue = currentValue;
    
    if (!leftValue || !rightValue) {
        error("Invalid operands in binary expression");
        return;
    }
    
    // Generate appropriate instruction based on operator
    if (node.operator_ == "+") {
        currentValue = builder->CreateAdd(leftValue, rightValue, "addtmp");
    } else if (node.operator_ == "-") {
        currentValue = builder->CreateSub(leftValue, rightValue, "subtmp");
    } else if (node.operator_ == "*") {
        currentValue = builder->CreateMul(leftValue, rightValue, "multmp");
    } else if (node.operator_ == "/") {
        currentValue = builder->CreateSDiv(leftValue, rightValue, "divtmp");
    } else if (node.operator_ == "%") {
        currentValue = builder->CreateSRem(leftValue, rightValue, "modtmp");
    } else if (node.operator_ == "<") {
        currentValue = builder->CreateICmpSLT(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == ">") {
        currentValue = builder->CreateICmpSGT(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == "<=") {
        currentValue = builder->CreateICmpSLE(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == ">=") {
        currentValue = builder->CreateICmpSGE(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == "==") {
        currentValue = builder->CreateICmpEQ(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == "!=") {
        currentValue = builder->CreateICmpNE(leftValue, rightValue, "cmptmp");
    } else if (node.operator_ == "&&") {
        currentValue = builder->CreateLogicalAnd(leftValue, rightValue, "andtmp");
    } else if (node.operator_ == "||") {
        currentValue = builder->CreateLogicalOr(leftValue, rightValue, "ortmp");
    } else {
        error("Unknown binary operator: " + node.operator_);
    }
}

void CodeGenerator::visit(UnaryOpExpression& node) {
    // Generate operand
    node.operand->accept(*this);
    llvm::Value* operandValue = currentValue;
    
    if (!operandValue) {
        error("Invalid operand in unary expression");
        return;
    }
    
    if (node.operator_ == "-") {
        currentValue = builder->CreateNeg(operandValue, "negtmp");
    } else if (node.operator_ == "!") {
        currentValue = builder->CreateNot(operandValue, "nottmp");
    } else {
        error("Unknown unary operator: " + node.operator_);
    }
}

void CodeGenerator::visit(FunctionCallExpression& node) {
    // Look up function in module
    llvm::Function* calleeF = module->getFunction(node.functionName);
    if (!calleeF) {
        error("Unknown function referenced: " + node.functionName);
        return;
    }
    
    // Check argument count
    if (calleeF->arg_size() != node.arguments.size()) {
        error("Incorrect number of arguments passed");
        return;
    }
      // Generate arguments
    std::vector<llvm::Value*> argsV;
    for (auto& arg : node.arguments) {
        arg->accept(*this);
        if (!currentValue) {
            error("Invalid argument in function call");
            return;
        }
        argsV.push_back(currentValue);
    }
    
    // Create call instruction, only name it if it returns a value
    if (calleeF->getReturnType()->isVoidTy()) {
        currentValue = builder->CreateCall(calleeF, (llvm::ArrayRef<llvm::Value*>)argsV);
    } else {
        currentValue = builder->CreateCall(calleeF, (llvm::ArrayRef<llvm::Value*>)argsV, "calltmp");
    }
}

void CodeGenerator::visit(VariableDeclaration& node) {
    // Generate initial value
    llvm::Value* initVal = nullptr;
    if (node.initializer) {
        node.initializer->accept(*this);
        initVal = currentValue;
    } else {
        // Default initialization
        llvm::Type* llvmType = getLLVMType(node.type);
        if (llvmType->isDoubleTy()) {
            initVal = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
        } else if (llvmType->isPointerTy()) {
            initVal = llvm::ConstantPointerNull::get(llvm::PointerType::get(builder->getInt8Ty(), 0));
        } else {
            initVal = llvm::Constant::getNullValue(llvmType);
        }
    }
    
    if (!initVal) {
        error("Failed to generate initial value for variable: " + node.name);
        return;
    }
      // Create alloca for the variable
    llvm::Value* alloca = createEntryBlockAlloca(currentFunction, node.name, getLLVMType(node.type));
    
    // Store initial value
    builder->CreateStore(initVal, alloca);
      // Remember the variable
    namedValues[node.name] = alloca;
    
    currentValue = alloca;
}

void CodeGenerator::visit(FunctionDeclaration& node) {
    // Create function type
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        paramTypes.push_back(getLLVMType(param.type));
    }
    
    llvm::Type* returnType = getLLVMType(node.returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(
        funcType, 
        llvm::Function::ExternalLinkage,
        node.name,
        module.get()
    );
    
    // Set parameter names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(node.parameters[idx++].name);
    }
    
    // Create basic block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(bb);
      // Save previous state
    llvm::Function* prevFunction = currentFunction;
    std::map<std::string, llvm::Value*> prevNamedValues = namedValues;
    
    currentFunction = function;
    namedValues.clear();      // Create allocas for parameters
    for (auto& arg : function->args()) {
        llvm::Value* alloca = createEntryBlockAlloca(function, std::string(arg.getName()), arg.getType());
        builder->CreateStore(&arg, alloca);
        namedValues[std::string(arg.getName())] = alloca;
    }
    
    // Generate function body
    if (node.body) {
        node.body->accept(*this);
    }
    
    // Verify function
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        error("Function verification failed for: " + node.name);
        function->eraseFromParent();
    }
    
    // Restore previous state
    currentFunction = prevFunction;
    namedValues = prevNamedValues;
    
    currentValue = function;
}

void CodeGenerator::visit(BlockStatement& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        
        // Check if we've already generated a terminator
        if (builder->GetInsertBlock()->getTerminator()) {
            break; // Stop generating code after return, etc.
        }
    }
}

void CodeGenerator::visit(IfStatement& node) {
    // Generate condition
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;
    if (!condV) {
        error("Invalid condition in if statement");
        return;
    }
    
    // Convert condition to boolean
    if (condV->getType()->isDoubleTy()) {
        condV = builder->CreateFCmpONE(condV, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "ifcond");
    } else if (condV->getType()->isIntegerTy()) {
        condV = builder->CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "ifcond");
    }
    
    // Create basic blocks
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "then", function);
    llvm::BasicBlock* elseBB = node.elseBranch ? llvm::BasicBlock::Create(*context, "else") : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont");
    
    // Branch
    if (elseBB) {
        builder->CreateCondBr(condV, thenBB, elseBB);
    } else {
        builder->CreateCondBr(condV, thenBB, mergeBB);
    }
    
    // Generate then block
    builder->SetInsertPoint(thenBB);
    node.thenBranch->accept(*this);
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    thenBB = builder->GetInsertBlock(); // Update in case of nested blocks
      // Generate else block if present
    if (elseBB) {
        function->insert(function->end(), elseBB);
        builder->SetInsertPoint(elseBB);
        node.elseBranch->accept(*this);
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(mergeBB);
        }
        elseBB = builder->GetInsertBlock();
    }
    
    // Generate merge block
    function->insert(function->end(), mergeBB);
    builder->SetInsertPoint(mergeBB);
}

void CodeGenerator::visit(WhileStatement& node) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*context, "whilecond", function);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context, "whileloop", function);
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context, "afterloop", function);
    
    // Jump to condition
    builder->CreateBr(condBB);
    
    // Generate condition block
    builder->SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condV = currentValue;
    if (!condV) {
        error("Invalid condition in while statement");
        return;
    }
    
    // Convert condition to boolean
    if (condV->getType()->isDoubleTy()) {
        condV = builder->CreateFCmpONE(condV, llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "whilecond");
    } else if (condV->getType()->isIntegerTy()) {
        condV = builder->CreateICmpNE(condV, llvm::ConstantInt::get(condV->getType(), 0), "whilecond");
    }
    
    builder->CreateCondBr(condV, loopBB, afterBB);
    
    // Generate loop body
    builder->SetInsertPoint(loopBB);
    node.body->accept(*this);
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(condBB);
    }
    
    // Continue with after block
    builder->SetInsertPoint(afterBB);
}

void CodeGenerator::visit(ReturnStatement& node) {
    if (node.value) {
        node.value->accept(*this);
        llvm::Value* retVal = currentValue;
        if (!retVal) {
            error("Invalid return value");
            return;
        }
        builder->CreateRet(retVal);
    } else {
        builder->CreateRetVoid();
    }
}

void CodeGenerator::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void CodeGenerator::visit(Program& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

void CodeGenerator::error(const std::string& message) const {
    std::cerr << "CodeGen Error: " << message << std::endl;
    throw std::runtime_error("Code generation error: " + message);
}

void CodeGenerator::runOptimizationPasses() {
    // Create a legacy pass manager for function optimization
    auto functionPassManager = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());
    auto modulePassManager = std::make_unique<llvm::legacy::PassManager>();
    
    // Add optimization passes based on level
    switch (optimizationLevel) {
        case OptimizationLevel::O1:
            // Basic optimizations
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            functionPassManager->add(llvm::createGVNPass());
            functionPassManager->add(llvm::createCFGSimplificationPass());
            break;
              case OptimizationLevel::O2:
            // More aggressive optimizations
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            functionPassManager->add(llvm::createGVNPass());
            functionPassManager->add(llvm::createCFGSimplificationPass());
            functionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            // Note: Some legacy passes may not be available in LLVM 20
            break;
              case OptimizationLevel::O3:
            // Aggressive optimizations
            functionPassManager->add(llvm::createInstructionCombiningPass());
            functionPassManager->add(llvm::createReassociatePass());
            functionPassManager->add(llvm::createGVNPass());
            functionPassManager->add(llvm::createCFGSimplificationPass());
            functionPassManager->add(llvm::createPromoteMemoryToRegisterPass());
            functionPassManager->add(llvm::createTailCallEliminationPass());
            modulePassManager->add(llvm::createAlwaysInlinerLegacyPass());
            break;
            
        default:
            return; // No optimization
    }
    
    // Initialize and run function passes
    functionPassManager->doInitialization();
    for (auto& function : *module) {
        functionPassManager->run(function);
    }
    functionPassManager->doFinalization();
    
    // Run module passes
    modulePassManager->run(*module);
}

void CodeGenerator::writeObjectFile(const std::string& filename) const {
    // Initialize only the native target for object file generation
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        throw std::runtime_error("Failed to lookup target: " + error);
    }
    
    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    std::optional<llvm::Reloc::Model> relocModel;
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocModel);
    
    module->setDataLayout(targetMachine->createDataLayout());
    
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        throw std::runtime_error("Could not open file " + filename + " for writing: " + errorCode.message());
    }
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        throw std::runtime_error("TargetMachine can't emit a file of this type");
    }
    
    pass.run(*module);
    dest.flush();
}

void CodeGenerator::setOptimizationLevel(OptimizationLevel level) {
    optimizationLevel = level;
}

OptimizationLevel CodeGenerator::getOptimizationLevel() const {
    return optimizationLevel;
}

void CodeGenerator::visit(DereferenceExpression& node) {
    // Visit the operand to get the pointer value
    node.operand->accept(*this);
    llvm::Value* ptrValue = currentValue;
    
    if (!ptrValue) {
        error("Invalid pointer value for dereference");
        return;
    }
    // Create load instruction to dereference pointer
    // For LLVM 15+, we need to specify the type explicitly
    llvm::Type* elementType = nullptr;
    if (auto ptrType = llvm::dyn_cast<llvm::PointerType>(ptrValue->getType())) {
        // Modern LLVM - use opaque pointers and infer type from context
        // For now, assume int32 for basic pointer operations
        elementType = llvm::Type::getInt32Ty(*context);
    } else {
        elementType = llvm::Type::getInt32Ty(*context);
    }
    currentValue = builder->CreateLoad(elementType, ptrValue, "deref");
}

void CodeGenerator::visit(AddressOfExpression& node) {
    // For address-of operation, we need the address of a variable
    if (auto identifier = dynamic_cast<IdentifierExpression*>(node.operand.get())) {
        auto it = namedValues.find(identifier->name);
        if (it != namedValues.end()) {
            // namedValues stores alloca instructions (addresses)
            currentValue = it->second;
        } else {
            error("Undefined variable for address-of: " + identifier->name);
            currentValue = nullptr;
        }
    } else {
        error("Address-of operation only supported for variables");
        currentValue = nullptr;
    }
}

} // namespace emlang
