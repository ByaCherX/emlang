// Test file for extern function declarations
// This file tests the semantic analysis of extern function declarations

// Basic extern function declarations with primitive types
extern function printf(format: str): int32;
extern function strlen(s: str): uint64;
extern function malloc(size: uint64): void*;
extern function free(ptr: void*): void;

// Math functions
extern function sin(x: double): double;
extern function cos(x: double): double;
extern function sqrt(x: double): double;

// String functions  
extern function strcpy(dest: char*, src: char*): char*;
extern function strcmp(s1: char*, s2: char*): int32;

// Test various integer types
extern function test_int8(x: int8): int8;
extern function test_int16(x: int16): int16;
extern function test_int32(x: int32): int32;
extern function test_int64(x: int64): int64;
extern function test_uint8(x: uint8): uint8;
extern function test_uint16(x: uint16): uint16;
extern function test_uint32(x: uint32): uint32;
extern function test_uint64(x: uint64): uint64;

// Test floating point types
extern function test_float(x: float): float;
extern function test_double(x: double): double;

// Test char and bool types
extern function test_char(c: char): bool;
extern function test_bool(b: bool): bool;

// Test pointer types
extern function test_int_ptr(p: int32*): int32*;
extern function test_char_ptr(p: char*): char*;
extern function test_void_ptr(p: void*): void*;

// Multiple parameters
extern function test_multiple(a: int32, b: float, c: char*, d: bool): double;

// Test functions that are used in main
function main(): int32 {
    let x: int32 = 42;
    let y: float = 3.14;
    let name: str = "Hello";
    
    // Call some extern functions
    let len: uint64 = strlen(name);
    printf("Length: %d\n");
    
    let result: double = sin(y);
    printf("sin(3.14) = %f\n");
    
    return 0;
}
