// LLVM Code Generator implementation
#include "../include/codegen.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <stdexcept>

namespace emlang {

CodeGenerator::CodeGenerator(const std::string& moduleName) 
    : currentFunction(nullptr), currentValue(nullptr) {
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
    if (typeName == "number" || typeName == "int") {
        return llvm::Type::getInt32Ty(*context);
    } else if (typeName == "float" || typeName == "double") {
        return llvm::Type::getDoubleTy(*context);
    } else if (typeName == "boolean" || typeName == "bool") {
        return llvm::Type::getInt1Ty(*context);
    } else if (typeName == "string") {
        return llvm::Type::getInt8PtrTy(*context);
    } else if (typeName == "void") {
        return llvm::Type::getVoidTy(*context);
    }
    
    error("Unknown type: " + typeName);
    return nullptr;
}

llvm::Type* CodeGenerator::getNumberType() {
    return llvm::Type::getInt32Ty(*context);
}

llvm::Type* CodeGenerator::getStringType() {
    return llvm::Type::getInt8PtrTy(*context);
}

llvm::Type* CodeGenerator::getBooleanType() {
    return llvm::Type::getInt1Ty(*context);
}

llvm::Value* CodeGenerator::createEntryBlockAlloca(llvm::Function* function, const std::string& varName, llvm::Type* type) {
    llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tempBuilder.CreateAlloca(type, nullptr, varName);
}

void CodeGenerator::generateIR(Program& program) {
    program.accept(*this);
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
        case LiteralExpression::LiteralType::BOOLEAN:
            currentValue = llvm::ConstantInt::get(*context, llvm::APInt(1, node.value == "true" ? 1 : 0));
            break;
        case LiteralExpression::LiteralType::NULL_LITERAL:
            currentValue = llvm::Constant::getNullValue(llvm::Type::getInt8PtrTy(*context));
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
    
    currentValue = builder->CreateCall(calleeF, argsV, "calltmp");
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
        function->getBasicBlockList().push_back(elseBB);
        builder->SetInsertPoint(elseBB);
        node.elseBranch->accept(*this);
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(mergeBB);
        }
        elseBB = builder->GetInsertBlock();
    }
    
    // Generate merge block
    function->getBasicBlockList().push_back(mergeBB);
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

} // namespace emlang
