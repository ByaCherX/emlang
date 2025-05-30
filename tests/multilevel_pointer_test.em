// Multi-level pointer test
function main() : int32 {
    let x: int32 = 42;
    let ptr: int32* = &x;
    let ptr_ptr: int32** = &ptr;
    
    // Test single-level operations first
    let value1: int32 = *ptr;
    
    // Now test multi-level
    let value2: int32 = *(*ptr_ptr);
    
    return value2;
}
