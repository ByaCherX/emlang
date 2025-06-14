//===--- emlang_export.h - Global DLL Export/Import Macros ------*- C++ -*-===//
//
// Part of the EMLang Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Global DLL export/import macros for all EMLang components
//
// This file provides a unified DLL export/import system that should be
// included by all EMLang header files that need to export symbols.
//===----------------------------------------------------------------------===//

#ifndef EMLANG_EXPORT_H
#define EMLANG_EXPORT_H

#pragma once

// ======================== DLL Export/Import Macros ========================
#ifdef _WIN32
    #ifdef EMLANG_EXPORTS
        #define EMLANG_API __declspec(dllexport)
    #elif defined(EMLANG_DLL)
        #define EMLANG_API __declspec(dllimport)
    #else
        #define EMLANG_API
    #endif
#else
    // Unix/Linux platforms
    #ifdef EMLANG_EXPORTS
        #define EMLANG_API __attribute__((visibility("default")))
    #else
        #define EMLANG_API
    #endif
#endif

// ======================== Template Export Helper ========================
// For template classes that need explicit instantiation
#ifdef _WIN32
    #ifdef EMLANG_EXPORTS
        #define EMLANG_TEMPLATE_API
    #else
        #define EMLANG_TEMPLATE_API extern
    #endif
#else
    #define EMLANG_TEMPLATE_API
#endif

// ======================== C Export/Import Macros ========================
#ifdef __cplusplus
    #define EMLANG_C_API extern "C" EMLANG_API
#else
    #define EMLANG_C_API EMLANG_API
#endif

#endif // EMLANG_EXPORT_H
