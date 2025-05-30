// Simple pointer test
function main() : int32 {
    let x: int32 = 42;
    let ptr: int32* = &x;
    return *ptr;
}
