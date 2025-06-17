<div align="center">

![LLVM Logo](https://llvm.org/img/LLVMWyvernSmall.png)

# EMLang

**A Modern Systems Programming Language with LLVM Backend**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](#building)
[![LLVM](https://img.shields.io/badge/LLVM-17+-blue.svg)](https://llvm.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](#prerequisites)
[![v1.0.0](https://img.shields.io/badge/EMLang_Version-1.0.0.alpha.2-purple.svg)](version)

EMLang is a statically-typed systems programming language designed for high-performance applications with ahead-of-time (AOT) compilation using LLVM. It combines modern language features with C-like performance and memory control.

</div>

> [!Warning]
> **This language is not designed for production use, it is created for testing and learning purposes only.**
> - ⚠️ It is not recommended to use it in production systems.
> - 🔬 It is a work in progress and may not be fully functional or stable.

---

## 🚀 Project Overview

EMLang provides a complete compiler pipeline with:
- **Advanced Type System**: Comprehensive primitive types with Unicode support
- **Memory Safety**: C-style pointers with semantic analysis
- **LLVM Integration**: Native code generation with optimization
- **Rich Standard Library**: I/O, string manipulation, math operations, and utilities

### 📁 Project Structure

```
emlang/
├── compiler/              # Complete compiler implementation
│   ├── lexer/             # Lexical analyzer with Unicode support
│   ├── parser/            # Recursive descent parser
│   ├── ast/               # Abstract Syntax Tree with visitor pattern
│   ├── semantic/          # Semantic analyzer with type checking
│   └── codegen/           # LLVM IR code generator
├── include/               # Public header files
├── library/               # Comprehensive standard library
│   ├── include/           # Library headers
│   └── src/               # Library implementations
├── src/                   # Main compiler and analysis tools
├── tests/                 # Comprehensive test suite
└── build/                 # Build artifacts and executables
```

## ✨ Language Features

#### ✅ **Primitive Types**
```emlang
// Integer types with explicit sizing
let byte: int8 = 127;
let short: int16 = 32767;
let normal: int32 = 2147483647;
let large: int64 = 9223372036854775807;

// Unsigned variants
let ubyte: uint8 = 255;
let ushort: uint16 = 65535;
let uint: uint32 = 4294967295;
let ulong: uint64 = 18446744073709551615;

// Floating point precision
let precision: float = 3.14159;
let highPrecision: double = 2.718281828459045;

// Boolean and character types
let flag: bool = true;
let character: char = 'A';
```

#### ✅ **Unicode and String Support**
```emlang
// Unicode character literals
let emoji: char = '😀';
let greek: char = '\u{03C0}';  // π
let euro: char = '\u{20AC}';   // €

// String literals with escape sequences
let message: str = "Hello, World!\n";
let path: str = "C:\\Users\\Name\\Documents";
let unicode: str = "Café Münü with symbols: ★ ♠ ♥";
```

#### ✅ **C-Style Pointer System**
```emlang
function pointerDemo(): int32 {
    let value: int32 = 42;
    let ptr: int32* = &value;        // Address-of operator
    let deref: int32 = *ptr;         // Dereference operator
    
    // Multi-level pointers
    let ptrToPtr: int32** = &ptr;
    let final: int32 = **ptrToPtr;
    
    return final;
}
```

#### ✅ **Function Declarations**
```emlang
function fibonacci(n: int32): int32 {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n-1) + fibonacci(n-2);
}

// External function declarations
extern function printf(format: str): int32;
```

#### ✅ **Control Flow**
```emlang
function controlFlow(): void {
    let x: int32 = 10;
    
    // Conditional statements
    if (x > 5) {
        // True branch
    } else {
        // False branch
    }
    
    // Loop constructs
    while (x > 0) {
        x = x - 1;
    }
    
    for (let i: int32 = 0; i < 10; i = i + 1) {
        // Loop body
    }
}
```

## 🏗️ Architecture & Implementation

### ✅ **Lexical Analysis**
- **Unicode-aware tokenization** with full UTF-8 support
- **Escape sequence processing** for strings and characters
- **Multi-line and single-line comments**
- **Comprehensive operator recognition**

### ✅ **Syntax Analysis** 
- **Recursive descent parser** with operator precedence
- **Complete AST node hierarchy** with visitor pattern
- **Error recovery** and synchronization
- **Lookahead disambiguation**

### ✅ **Semantic Analysis**
- **Hierarchical symbol tables** with scope management
- **Static type checking** with type inference
- **Pointer type validation** and safety analysis
- **Function signature verification**

### ✅ **Code Generation**
- **LLVM IR generation** for all language constructs
- **Type mapping** from EMLang to LLVM types
- **Memory management** with stack allocation
- **Optimization passes** integration

### 📚 **Standard Library**
> [!Warning]
> The emlang standard library is not available at the moment. It will be available in beta.
The library provides essential functionality across multiple domains:

- **I/O Operations**: `emlang_print_*`, `emlang_read_*`, console control
- **String Manipulation**: `emlang_strlen`, `emlang_strcmp`, case conversion
- **Mathematical Functions**: `emlang_pow`, `emlang_sqrt`, trigonometry
- **Memory Management**: `emlang_malloc`, `emlang_free`, `emlang_memset`
- **Utility Functions**: Array operations, bit manipulation, hashing

## 🔧 Building & Installation

### Prerequisites
- **CMake** 3.10 or higher
- **C++17** compatible compiler (GCC 9+, Clang 8+, MSVC 2017+)
- **LLVM** 14+ (automatically detected, enables code generation)
- **Git** for cloning and version control

### 🏗️ Build Process

#### Windows (Visual Studio)
```powershell
# Clone the repository
git clone https://github.com/ByaCherX/emlang.git
cd emlang

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build the project
cmake --build . --config Release

# Run tests
./Release/emlang_check.exe ../tests/simple_types_test.em
```

#### Linux/macOS
```bash
# Clone and build
git clone https://github.com/ByaCherX/emlang.git
cd emlang

# Create and enter build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install (optional)
sudo make install
```

### 📦 Build Targets

- **`emlang`** - Main compiler executable
- **`emlang_check`** - AST and token analysis tool
- **`emlang_compiler`** - Compiler library (DLL/shared object)
- **`emlang_lib`** - Standard library (optional, requires LLVM)

## 🚀 Usage & Examples

### Basic Compilation
```bash
# Compile EMLang source to object file
./emlang source.em

# Generate LLVM IR
./emlang --emit-llvm source.em -o output.ll

# Analyze source code structure
./emlang_check --ast --tokens source.em
```

### 📝 Language Examples

#### Complete Program
```emlang
extern function printf(format: str): int32;

function factorial(n: int32): int32 {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

function main(): int32 {
    let number: int32 = 5;
    let result: int32 = factorial(number);
    
    printf("Factorial of %d is %d\n");
    return 0;
}
```

#### Pointer Operations
```emlang
function swapIntegers(a: int32*, b: int32*): void {
    let temp: int32 = *a;
    *a = *b;
    *b = temp;
}

function main(): int32 {
    let x: int32 = 10;
    let y: int32 = 20;
    
    swapIntegers(&x, &y);
    // x is now 20, y is now 10
    
    return 0;
}
```

#### String and Character Processing
```emlang
function stringDemo(): void {
    let greeting: str = "Hello, 世界! 🌍";
    let firstChar: char = 'H';
    let unicode: char = '\u{1F30D}';  // 🌍
    
    // String escape sequences
    let formatted: str = "Line 1\nLine 2\t\"Quoted\"";
    let path: str = "C:\\Program Files\\EMLang";
}
```

### 🔍 Analysis Tools

#### Token Analysis
```bash
$ ./emlang_check --tokens simple_test.em
=== TOKENS ===
FUNCTION function 1:1
IDENTIFIER main 1:10
LEFT_PAREN ( 1:14
RIGHT_PAREN ) 1:15
COLON : 1:16
INT32 int32 1:18
LEFT_BRACE { 1:24
...
```

#### AST Visualization
```bash
$ ./emlang_check --ast simple_test.em
=== AST ===
Program
  FunctionDeclaration: main
    ReturnType: int32
    Parameters: []
    Body: BlockStatement
      ReturnStatement
        LiteralExpression: 0 (int32)
```

## 📊 Compiler Pipeline

The EMLang compiler implements a complete compilation pipeline:

1. **📝 Lexical Analysis** → Token stream generation
2. **🌳 Syntax Analysis** → Abstract Syntax Tree construction  
3. **🔍 Semantic Analysis** → Type checking and symbol resolution
4. **⚡ Code Generation** → LLVM IR emission
5. **🎯 Optimization** → LLVM optimization passes
6. **🔧 Linking** → Native executable generation

### 🚨 Error Reporting

EMLang provides comprehensive error diagnostics:

```
Semantic Error at 7:15 - Type mismatch: cannot assign 'str' to 'int32'
Semantic Error at 12:8 - Variable 'undeclared' not found in current scope  
Semantic Error at 18:23 - Function 'unknown' is not declared
Parse Error at 25:1 - Expected ';' after expression statement
```

## 🧪 Testing & Validation

### Test Suite Coverage
- **✅ Basic Types**: `simple_types_test.em`
- **✅ Unicode Characters**: `char_string_test.em`, `advanced_char_test.em`
- **✅ Pointer Operations**: `simple_pointer_test.em`, `multilevel_pointer_test.em`
- **✅ External Functions**: `extern_function_test.em`, `simple_extern_test.em`
- **✅ Integration**: `phase2_integration_test.em`, `phase3_pointer_test.em`
- **✅ Error Detection**: `error_test.em`

### Running Tests
```bash
# Run all tests
cd tests
for test in *.em; do
    echo "Testing $test"
    ../build/Debug/emlang_check "$test"
done

# Individual test analysis
./build/Debug/emlang_check --all tests/phase3_pointer_test.em
```

## 🤝 Contributing

We welcome contributions to EMLang! Here's how you can help:

### 🐛 **Bug Reports**
- Use GitHub Issues to report bugs
- Include minimal reproduction code
- Specify your operating system and compiler version

### ✨ **Feature Requests**
- Propose new language features through GitHub Issues
- Provide use cases and examples
- Consider implementation complexity

### 💻 **Code Contributions**
1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### 📋 **Development Guidelines**
- Follow the existing code style and conventions
- Add tests for new features
- Update documentation for API changes
- Ensure all tests pass before submitting

### 🔍 **Areas Needing Help**
- **Standard Library**: Expand mathematical and utility functions
- **Error Messages**: Improve error reporting and suggestions
- **Optimization**: LLVM optimization pass integration
- **Documentation**: API documentation and tutorials
- **Testing**: Additional test cases and edge cases

## 📄 License
EMLang is released under the **MIT License**. See [LICENSE](LICENSE) file for details.

---

## 🔗 Links & Resources

- **🐛 Issue Tracker**: [GitHub Issues](https://github.com/ByaCherX/emlang/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/ByaCherX/emlang/discussions)

---

<div align="center">

**Built with ❤️ using LLVM**

*EMLang - Where Performance Meets Productivity*

[⭐ Star us on GitHub](https://github.com/ByaCherX/emlang) | [🍴 Fork the Project](https://github.com/ByaCherX/emlang/fork) | [📝 Report Issues](https://github.com/ByaCherX/emlang/issues)

</div>
