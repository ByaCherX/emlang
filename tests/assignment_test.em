// Assignment Expression Test File
// This file tests the assignment functionality in EMLang

// External function declarations for testing
extern function emlang_print_int(value: int32): void;
extern function emlang_print_str(str_param: char*): void;
extern function emlang_println(): void;

// Test basic assignment operations
function testBasicAssignment(): void {
    let x: int32 = 10;
    
    emlang_print_str("Initial value of x: ");
    emlang_print_int(x);
    emlang_println();
    
    // Basic assignment
    x = 20;
    emlang_print_str("After x = 20: ");
    emlang_print_int(x);
    emlang_println();
    
    // Assignment with expression
    x = x + 5;
    emlang_print_str("After x = x + 5: ");
    emlang_print_int(x);
    emlang_println();
    
    // Assignment with complex expression
    x = x * 2 - 10;
    emlang_print_str("After x = x * 2 - 10: ");
    emlang_print_int(x);
    emlang_println();
}

// Test pointer assignment operations
function testPointerAssignment(): void {
    emlang_print_str("\nTesting pointer assignments:");
    emlang_println();
    
    let value: int32 = 42;
    let ptr: int32* = &value;
    
    emlang_print_str("Initial value: ");
    emlang_print_int(value);
    emlang_println();
    
    // Dereferenced assignment
    *ptr = 100;
    emlang_print_str("After *ptr = 100: ");
    emlang_print_int(value);
    emlang_println();
    
    // Change where the pointer points to
    let another_value: int32 = 200;
    ptr = &another_value;
    emlang_print_str("After ptr points to another variable: ");
    emlang_print_int(*ptr);
    emlang_println();
    
    // Modify the new target
    *ptr = 300;
    emlang_print_str("After *ptr = 300: ");
    emlang_print_int(another_value);
    emlang_println();
}

// Test assignments in expressions
function testAssignmentInExpressions(): void {
    emlang_print_str("\nTesting assignments in expressions:");
    emlang_println();
    
    let a: int32 = 5;
    let b: int32 = 10;
    
    emlang_print_str("Initial values - a: ");
    emlang_print_int(a);
    emlang_print_str(", b: ");
    emlang_print_int(b);
    emlang_println();
    
    // Use assignment within conditional
    if ((a = 20) > 15) {
        emlang_print_str("Assignment within if worked, a is now: ");
        emlang_print_int(a);
        emlang_println();
    }
    
    // Assignment within an expression
    b = (a = 30) + 5;
    emlang_print_str("After b = (a = 30) + 5, a: ");
    emlang_print_int(a);
    emlang_print_str(", b: ");
    emlang_print_int(b);
    emlang_println();
}

function main(): int32 {
    emlang_print_str("===== Assignment Expression Test =====");
    emlang_println();
    
    testBasicAssignment();
    testPointerAssignment();
    testAssignmentInExpressions();
    
    emlang_print_str("\nAll assignment tests completed!");
    emlang_println();
    
    return 0;
}
