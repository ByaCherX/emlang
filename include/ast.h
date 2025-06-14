//===--- ast.h - AST interface ----------------------------------*- C++ -*-===//
//
// Part of the RNR Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Main AST interface for EMLang
//
// This file provides a unified interface to all AST components.
//===----------------------------------------------------------------------===//

#ifndef EM_LANG_AST_H
#define EM_LANG_AST_H

#pragma once

// Include all AST components
#include "ast/ast_base.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/visitor.h"
#include "ast/dumper.h"

#endif // EM_LANG_AST_H