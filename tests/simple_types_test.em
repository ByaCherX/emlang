// EMLang Test - Simple Rust-like Primitive Types
// Test file with smaller numbers to avoid stoi overflow

function testBasicTypes(): void {
    // Signed integer types with small values
    let small: int8 = 100;
    let medium: int16 = 1000;
    let normal: int32 = 100000;
    let large: int64 = 100000;
    
    // Unsigned integer types with small values
    let ubyte: uint8 = 200;
    let ushort: uint16 = 2000;
    let uint: uint32 = 200000;
    let ulong: uint64 = 200000;
      // Floating point types
    let single: float = 3.14;
    let doubleVal: double = 2.718;
    
    // Boolean and legacy compatibility
    let flag: bool = true;
    let old_bool: boolean = false;
    
    return;
}

function main(): int32 {
    testBasicTypes();
    return 0;
}
