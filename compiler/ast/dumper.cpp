//===--- dumper.cpp - AST Debug Dumper --------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// AST debug dumper with colored output
//===----------------------------------------------------------------------===//

#include "ast/dumper.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace emlang {

// ANSI color codes
namespace Colors {
    const char* RESET   = "\033[0m";
    const char* PURPLE  = "\033[35m";  // For Stmt nodes
    const char* GREEN   = "\033[32m";  // For Decl nodes
    const char* CYAN    = "\033[36m";  // For Expr nodes
    const char* YELLOW  = "\033[33m";  // For values
    const char* GRAY    = "\033[90m";  // For addresses
}

ASTDumper::ASTDumper(bool useColors) : useColors_(useColors), indent_(0) {
    // Auto-detect color support if not explicitly set
    if (useColors_) {
#ifdef _WIN32
        // Enable ANSI color support on Windows 10+
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
#endif
        // Check if stdout is a terminal
        useColors_ = isatty(fileno(stdout));
    }
}

void ASTDumper::dump(const ASTNode& node) {
    const_cast<ASTNode&>(node).accept(*this);
}

std::string ASTDumper::getIndent() const {
    return std::string(indent_ * 2, ' ');
}

std::string ASTDumper::colorize(const std::string& text, const char* color) const {
    if (!useColors_) return text;
    return std::string(color) + text + Colors::RESET;
}

std::string ASTDumper::formatNodeHeader(const std::string& nodeType, const ASTNode& node) const {
    std::stringstream ss;
    ss << nodeType << " " << std::hex << &node;
    if (node.line > 0) {
        ss << " <line:" << std::dec << node.line;
        if (node.column > 0) {
            ss << ", col:" << node.column;
        }
        ss << ">";
    }
    return ss.str();
}

// Expression visitors (cyan color)
void ASTDumper::visit(LiteralExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("LiteralExpr", node), Colors::CYAN);
    std::cout << " " << colorize("value='" + node.value + "'", Colors::YELLOW) << std::endl;
}

void ASTDumper::visit(IdentifierExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("IdentifierExpr", node), Colors::CYAN);
    std::cout << " " << colorize("name='" + node.name + "'", Colors::YELLOW) << std::endl;
}

void ASTDumper::visit(BinaryOpExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("BinaryOpExpr", node), Colors::CYAN);
    std::cout << " " << colorize("op='" + node.operator_ + "'", Colors::YELLOW) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "├─left: ";
    indent_++;
    node.left->accept(*this);
    indent_--;
    
    std::cout << getIndent() << "└─right: ";
    indent_++;
    node.right->accept(*this);
    indent_--;
    indent_--;
}

void ASTDumper::visit(UnaryOpExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("UnaryOpExpr", node), Colors::CYAN);
    std::cout << " " << colorize("op='" + node.operator_ + "'", Colors::YELLOW) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "└─operand: ";
    indent_++;
    node.operand->accept(*this);
    indent_--;
    indent_--;
}

void ASTDumper::visit(AssignmentExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("AssignmentExpr", node), Colors::CYAN) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "├─target: ";
    indent_++;
    node.target->accept(*this);
    indent_--;
    
    std::cout << getIndent() << "└─value: ";
    indent_++;
    node.value->accept(*this);
    indent_--;
    indent_--;
}

void ASTDumper::visit(FunctionCallExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("FunctionCallExpr", node), Colors::CYAN);
    std::cout << " " << colorize("name='" + node.functionName + "'", Colors::YELLOW) << std::endl;
    
    if (!node.arguments.empty()) {
        indent_++;
        for (size_t i = 0; i < node.arguments.size(); ++i) {
            bool isLast = (i == node.arguments.size() - 1);
            std::cout << getIndent() << (isLast ? "└─arg" : "├─arg") << i << ": ";
            indent_++;
            node.arguments[i]->accept(*this);
            indent_--;
        }
        indent_--;
    }
}

void ASTDumper::visit(DereferenceExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("DereferenceExpr", node), Colors::CYAN) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "└─operand: ";
    indent_++;
    node.operand->accept(*this);
    indent_--;
    indent_--;
}

void ASTDumper::visit(AddressOfExpr& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("AddressOfExpr", node), Colors::CYAN) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "└─operand: ";
    indent_++;
    node.operand->accept(*this);
    indent_--;
    indent_--;
}

// Statement visitors (purple color)
void ASTDumper::visit(BlockStmt& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("BlockStmt", node), Colors::PURPLE) << std::endl;
    
    if (!node.statements.empty()) {
        indent_++;
        for (size_t i = 0; i < node.statements.size(); ++i) {
            bool isLast = (i == node.statements.size() - 1);
            std::cout << getIndent() << (isLast ? "└─stmt" : "├─stmt") << i << ": ";
            indent_++;
            node.statements[i]->accept(*this);
            indent_--;
        }
        indent_--;
    }
}

void ASTDumper::visit(IfStmt& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("IfStmt", node), Colors::PURPLE) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "├─condition: ";
    indent_++;
    node.condition->accept(*this);
    indent_--;
    
    std::cout << getIndent() << (node.elseBranch ? "├─then: " : "└─then: ");
    indent_++;
    node.thenBranch->accept(*this);
    indent_--;
    
    if (node.elseBranch) {
        std::cout << getIndent() << "└─else: ";
        indent_++;
        node.elseBranch->accept(*this);
        indent_--;
    }
    indent_--;
}

void ASTDumper::visit(WhileStmt& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("WhileStmt", node), Colors::PURPLE) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "├─condition: ";
    indent_++;
    node.condition->accept(*this);
    indent_--;
    
    std::cout << getIndent() << "└─body: ";
    indent_++;
    node.body->accept(*this);
    indent_--;
    indent_--;
}

void ASTDumper::visit(ReturnStmt& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("ReturnStmt", node), Colors::PURPLE);
    
    if (node.value) {
        std::cout << std::endl;
        indent_++;
        std::cout << getIndent() << "└─value: ";
        indent_++;
        node.value->accept(*this);
        indent_--;
        indent_--;
    } else {
        std::cout << " " << colorize("void", Colors::YELLOW) << std::endl;
    }
}

void ASTDumper::visit(ExpressionStmt& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("ExprStmt", node), Colors::PURPLE) << std::endl;
    
    indent_++;
    std::cout << getIndent() << "└─expr: ";
    indent_++;
    node.expression->accept(*this);
    indent_--;
    indent_--;
}

// Declaration visitors (green color)
void ASTDumper::visit(VariableDecl& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("VarDecl", node), Colors::GREEN);
    std::cout << " " << colorize((node.isConstant ? "const " : "let ") + node.name, Colors::YELLOW);
    if (!node.type.empty()) {
        std::cout << " " << colorize("type='" + node.type + "'", Colors::YELLOW);
    }
    std::cout << std::endl;
    
    if (node.initializer) {
        indent_++;
        std::cout << getIndent() << "└─init: ";
        indent_++;
        node.initializer->accept(*this);
        indent_--;
        indent_--;
    }
}

void ASTDumper::visit(FunctionDecl& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("FunctionDecl", node), Colors::GREEN);
    std::cout << " " << colorize("name='" + node.name + "'", Colors::YELLOW);
    if (!node.returnType.empty()) {
        std::cout << " " << colorize("return='" + node.returnType + "'", Colors::YELLOW);
    }
    std::cout << std::endl;
    
    indent_++;
    if (!node.parameters.empty()) {
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            const auto& param = node.parameters[i];
            bool isLast = (i == node.parameters.size() - 1) && !node.body;
            std::cout << getIndent() << (isLast ? "└─param" : "├─param") << i << ": ";
            std::cout << colorize(param.name + ": " + param.type, Colors::YELLOW) << std::endl;
        }
    }
    
    if (node.body) {
        std::cout << getIndent() << "└─body: ";
        indent_++;
        node.body->accept(*this);
        indent_--;
    }
    indent_--;
}

void ASTDumper::visit(ExternFunctionDecl& node) {
    std::cout << getIndent() << colorize(formatNodeHeader("ExternFunctionDecl", node), Colors::GREEN);
    std::cout << " " << colorize("name='" + node.name + "'", Colors::YELLOW);
    if (!node.returnType.empty()) {
        std::cout << " " << colorize("return='" + node.returnType + "'", Colors::YELLOW);
    }
    std::cout << std::endl;
    
    if (!node.parameters.empty()) {
        indent_++;
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            const auto& param = node.parameters[i];
            bool isLast = (i == node.parameters.size() - 1);
            std::cout << getIndent() << (isLast ? "└─param" : "├─param") << i << ": ";
            std::cout << colorize(param.name + ": " + param.type, Colors::YELLOW) << std::endl;
        }
        indent_--;
    }
}

void ASTDumper::visit(Program& node) {
    std::cout << colorize(formatNodeHeader("Program", node), Colors::PURPLE) << std::endl;
    
    if (!node.statements.empty()) {
        indent_++;
        for (size_t i = 0; i < node.statements.size(); ++i) {
            bool isLast = (i == node.statements.size() - 1);
            std::cout << getIndent() << (isLast ? "└─stmt" : "├─stmt") << i << ": ";
            indent_++;
            node.statements[i]->accept(*this);
            indent_--;
        }
        indent_--;
    }
}

} // namespace emlang
