// Phase 3: C-style pointer system test
// Testing pointer type declarations, operations, and basic memory management

function main() : int32 {
    // Basic pointer declarations
    let x: int32 = 42;
    let ptr: int32* = &x;
    let value: int32 = *ptr;
    
    // Multi-level pointers
    let ptr_ptr: int32** = &ptr;
    let indirect_value: int32 = **ptr_ptr;
    
    // Character pointers
    let ch: char = 'A';
    let ch_ptr: char* = &ch;
    let ch_value: char = *ch_ptr;
    
    // Different integer type pointers
    let byte_val: int8 = 100;
    let byte_ptr: int8* = &byte_val;
    
    let short_val: int16 = 1000;
    let short_ptr: int16* = &short_val;
    
    let long_val: int64 = 1000000;
    let long_ptr: int64* = &long_val;
    
    // Unsigned type pointers
    let ubyte_val: uint8 = 255;
    let ubyte_ptr: uint8* = &ubyte_val;
    
    let ushort_val: uint16 = 65535;
    let ushort_ptr: uint16* = &ushort_val;
    
    let uint_val: uint32 = 4294967295;
    let uint_ptr: uint32* = &uint_val;
    
    // Float pointers
    let float_val: float = 3.14;
    let float_ptr: float* = &float_val;
    
    let double_val: double = 2.71828;
    let double_ptr: double* = &double_val;
    
    // Boolean pointers
    let bool_val: bool = true;
    let bool_ptr: bool* = &bool_val;
    
    // Verify dereferencing works
    if (*ptr == 42) {
        if (*ch_ptr == 'A') {
            if (*bool_ptr == true) {
                return 0; // Success
            }
        }
    }
    
    return 1; // Failure
}
