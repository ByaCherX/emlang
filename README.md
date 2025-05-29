# EMLang - Ahead-of-Time Programming Language

EMLang is a modern programming language designed for ahead-of-time (AOT) compilation using LLVM. The language features static typing, function declarations, control flow statements, and comprehensive semantic analysis.

## Project Structure

```
emlang/
├── compiler/              # Compiler implementation
│   ├── lexer/             # Lexical analyzer
│   ├── parser/            # Syntax parser with recursive descent
│   ├── ast/               # Abstract Syntax Tree implementation
│   ├── semantic/          # Semantic analyzer with type checking
│   └── codegen/           # LLVM code generator (placeholder)
├── include/               # Header files
├── src/                   # Main compiler driver
├── library/               # Standard library (placeholder)
├── tests/                 # Test files
├── build/                 # Build output directory
└── CMakeLists.txt         # Build configuration
```

## Features Implemented

### ✅ Lexer
- Complete tokenization for keywords, operators, literals, identifiers
- Comment support (single-line `//` and multi-line `/* */`)
- String literals with escape sequences
- Number literals
- Identifier and keyword recognition

### ✅ Parser
- Recursive descent parser with operator precedence
- Support for expressions, statements, and declarations
- Binary and unary expressions
- Function declarations with parameters
- Control flow statements (if/else, while)
- Variable declarations (let/const)

### ✅ AST (Abstract Syntax Tree)
- Complete node hierarchy for all language constructs
- Visitor pattern implementation for AST traversal
- Expression and Statement base classes
- Support for complex nested expressions

### ✅ Semantic Analyzer
- Symbol table management with scope hierarchy
- Type checking for all expressions and statements
- Variable and function declaration validation
- Return type checking for functions
- Comprehensive error reporting

### 🔄 Code Generation (In Progress)
- LLVM integration setup
- Placeholder implementation ready for development

## Language Syntax

### Variable Declarations
```emlang
let x: number = 42;
const message: string = "Hello, EMLang!";
```

### Function Declarations
```emlang
function fibonacci(n: number): number {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n-1) + fibonacci(n-2);
}
```

### Control Flow
```emlang
if (condition) {
    // then branch
} else {
    // else branch
}

while (condition) {
    // loop body
}
```

### Expression Support
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Logical: `&&`, `||`, `!`
- Function calls: `functionName(arg1, arg2)`

## Building

### Prerequisites
- CMake 3.10+
- C++17 compatible compiler
- LLVM 14+ (optional, will disable code generation if not found)

### Build Steps
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Usage

```bash
./emlang source_file.em
```

### Example Output
The compiler performs complete compilation pipeline:
1. **Lexical Analysis** - Tokenizes input source
2. **Syntax Analysis** - Builds Abstract Syntax Tree
3. **Semantic Analysis** - Type checking and symbol resolution
4. **Code Generation** (Future) - LLVM IR generation

## Error Handling

The compiler provides comprehensive error reporting:

```
Semantic Error at 4:5 - Variable already declared in current scope: x
Semantic Error at 7:23 - Arithmetic operations require numeric types
Semantic Error at 8:5 - If condition must be boolean type
```

## Test Files

- `tests/test.em` - Successful compilation example with fibonacci function
- `tests/error_test.em` - Comprehensive error detection test

## Development Status

### Completed
- [x] Lexer with complete tokenization
- [x] Parser with recursive descent and operator precedence
- [x] AST with visitor pattern
- [x] Semantic analyzer with type checking
- [x] Symbol table and scope management
- [x] Error reporting system
- [x] Build system with CMake

### In Progress
- [ ] LLVM code generation
- [ ] Standard library implementation
- [ ] Optimization passes

### Future Plans
- [ ] Advanced type system (arrays, structs)
- [ ] Module system
- [ ] Package manager
- [ ] IDE language server

## Contributing

This is a learning project demonstrating compiler construction with modern C++ and LLVM. Feel free to explore the codebase and contribute improvements!

## License

MIT License - see LICENSE file for details.
