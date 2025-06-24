//===--- CGBase.cpp - Base Code Generation Visitor -----------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Base code generation visitor implementation for EMLang
//===----------------------------------------------------------------------===//

#include "codegen/CGBase.h"
#include "codegen/CGExpr.h"
#include "codegen/CGDecl.h"
#include "codegen/CGStmt.h"

namespace emlang {
namespace codegen {

/****************************** 
* Constructor and Destructor
******************************/

CGBase::CGBase(
    ContextManager& contextManager, 
    ValueMap& valueMap, 
    CodegenErrorReporter& errorReporter
) : contextManager(contextManager), 
    valueMap(valueMap), 
    errorReporter(errorReporter), 
    currentValue(nullptr),
    exprVisitor(nullptr),
    declVisitor(nullptr),
    stmtVisitor(nullptr) {}

CGBase::CGBase(
    ContextManager& contextManager,
    ValueMap& valueMap, 
    CodegenErrorReporter& errorReporter,
    CGExpr* exprVisitor,
    CGDecl* declVisitor,
    CGStmt* stmtVisitor
) : contextManager(contextManager), 
    valueMap(valueMap), 
    errorReporter(errorReporter), 
    currentValue(nullptr),
    exprVisitor(exprVisitor),
    declVisitor(declVisitor),
    stmtVisitor(stmtVisitor) {}

llvm::Value* CGBase::getCurrentValue() const {
    return currentValue;
}

void CGBase::setCurrentValue(llvm::Value* value) {
    currentValue = value;
}

void CGBase::setCurrentValue(llvm::Value* value, const std::string& type) {
    currentValue = value;
    // Base class doesn't handle type, derived classes can override
}

/****************************** 
* AST Visitor - Program
******************************/

void CGBase::visit(Program& node) {
    // Process all statements in the program
    for (auto& stmt : node.statements) {
        // Route to appropriate specialized visitor if available
        if (auto funcDecl = dynamic_cast<FunctionDecl*>(stmt.get())) {
            if (declVisitor) {
                funcDecl->accept(*declVisitor);
            } else {
                funcDecl->accept(*this);
            }
        }
        else if (auto externDecl = dynamic_cast<ExternFunctionDecl*>(stmt.get())) {
            if (declVisitor) {
                externDecl->accept(*declVisitor);
            } else {
                externDecl->accept(*this);
            }
        }
        else if (auto varDecl = dynamic_cast<VariableDecl*>(stmt.get())) {
            if (declVisitor) {
                varDecl->accept(*declVisitor);
            } else {
                varDecl->accept(*this);
            }
        }
        else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt.get())) {
            if (stmtVisitor) {
                blockStmt->accept(*stmtVisitor);
            } else {
                blockStmt->accept(*this);
            }
        }
        else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
            if (stmtVisitor) {
                ifStmt->accept(*stmtVisitor);
            } else {
                ifStmt->accept(*this);
            }
        }
        else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
            if (stmtVisitor) {
                whileStmt->accept(*stmtVisitor);
            } else {
                whileStmt->accept(*this);
            }
        }
        else if (auto forStmt = dynamic_cast<ForStmt*>(stmt.get())) {
            if (stmtVisitor) {
                forStmt->accept(*stmtVisitor);
            } else {
                forStmt->accept(*this);
            }
        }
        else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt.get())) {
            if (stmtVisitor) {
                returnStmt->accept(*stmtVisitor);
            } else {
                returnStmt->accept(*this);
            }
        }
        else if (auto exprStmt = dynamic_cast<ExpressionStmt*>(stmt.get())) {
            if (stmtVisitor) {
                exprStmt->accept(*stmtVisitor);
            } else {
                exprStmt->accept(*this);
            }
        }
        else {
            // Fallback to self
            stmt->accept(*this);
        }
    }
}

/****************************** 
* Error Handling
******************************/

void CGBase::error(const std::string& message) {
    errorReporter.error(CodegenErrorType::InternalError, message);
    currentValue = nullptr;
}

void CGBase::error(CodegenErrorType type, const std::string& message, const std::string& context) {
    errorReporter.error(type, message, context);
    currentValue = nullptr;
}

} // namespace codegen
} // namespace emlang
