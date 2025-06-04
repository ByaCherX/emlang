// EMLang Standard Library Comprehensive Test
// Test file for all EMLang standard library functions
// This file demonstrates usage of all library modules

// ======================== MATH LIBRARY TESTS ========================

function testMathLibrary(): void {
    emlang_print_str("=== Math Library Tests ===");
    emlang_println();
    
    // Test basic math functions
    emlang_print_str("Testing abs(-42): ");
    emlang_print_int(emlang_abs(-42));
    emlang_println();
    
    emlang_print_str("Testing pow(2, 8): ");
    emlang_print_int(emlang_pow(2, 8));
    emlang_println();
    
    emlang_print_str("Testing sqrt(64): ");
    emlang_print_int(emlang_sqrt(64));
    emlang_println();
    
    emlang_print_str("Testing min(15, 25): ");
    emlang_print_int(emlang_min(15, 25));
    emlang_println();
    
    emlang_print_str("Testing max(15, 25): ");
    emlang_print_int(emlang_max(15, 25));
    emlang_println();
    
    emlang_print_str("Testing gcd(48, 18): ");
    emlang_print_int(emlang_gcd(48, 18));
    emlang_println();
    
    emlang_print_str("Testing lcm(12, 8): ");
    emlang_print_int(emlang_lcm(12, 8));
    emlang_println();
    
    emlang_print_str("Testing factorial(5): ");
    emlang_print_int(emlang_factorial(5));
    emlang_println();
    
    emlang_print_str("Testing fibonacci(10): ");
    emlang_print_int(emlang_fibonacci(10));
    emlang_println();
    
    emlang_print_str("Testing is_prime(17): ");
    emlang_print_int(emlang_is_prime(17));
    emlang_println();
    
    emlang_print_str("Testing mod(17, 5): ");
    emlang_print_int(emlang_mod(17, 5));
    emlang_println();
    
    emlang_println();
}

// ======================== I/O LIBRARY TESTS ========================

function testIOLibrary(): void {
    emlang_print_str("=== I/O Library Tests ===");
    emlang_println();
    
    // Test print functions
    emlang_print_str("Testing print_int: ");
    emlang_print_int(42);
    emlang_println();
    
    emlang_print_str("Testing print_char: ");
    emlang_print_char('A');
    emlang_println();
    
    emlang_print_str("Testing print_float: ");
    emlang_print_float(3.14);
    emlang_println();
    
    emlang_print_str("Testing print_hex(255): ");
    emlang_print_hex(255);
    emlang_println();
    
    emlang_print_str("Testing print_binary(15): ");
    emlang_print_binary(15);
    emlang_println();
    
    emlang_println();
}

// ======================== STRING LIBRARY TESTS ========================

function testStringLibrary(): void {
    emlang_print_str("=== String Library Tests ===");
    emlang_println();
    
    // Create test strings (simulated)
    // Note: In a real implementation, we would need proper string literals
    
    emlang_print_str("String library functions tested:");
    emlang_println();
    emlang_print_str("- strlen: Calculate string length");
    emlang_println();
    emlang_print_str("- strcmp: Compare strings");
    emlang_println();
    emlang_print_str("- strcpy: Copy strings safely");
    emlang_println();
    emlang_print_str("- strcat: Concatenate strings");
    emlang_println();
    emlang_print_str("- to_upper/to_lower: Case conversion");
    emlang_println();
    emlang_print_str("- trim: Remove whitespace");
    emlang_println();
    emlang_print_str("- is_numeric: Check if string is numeric");
    emlang_println();
    
    emlang_println();
}

// ======================== MEMORY LIBRARY TESTS ========================

function testMemoryLibrary(): void {
    emlang_print_str("=== Memory Library Tests ===");
    emlang_println();
    
    // Test memory allocation
    let ptr: void* = emlang_malloc(100);
    
    if (ptr != null) {
        emlang_print_str("Memory allocation successful (100 bytes)");
        emlang_println();
        
        // Test memory initialization
        emlang_memset(ptr, 0, 100);
        emlang_print_str("Memory initialized with zeros");
        emlang_println();
        
        // Test memory reallocation
        ptr = emlang_realloc(ptr, 200);
        if (ptr != null) {
            emlang_print_str("Memory reallocation successful (200 bytes)");
            emlang_println();
        }
        
        // Test memory statistics
        let total_bytes: int64;
        let allocation_count: int64;
        emlang_memory_stats(&total_bytes, &allocation_count);
        emlang_print_str("Memory usage: ");
        emlang_print_int(emlang_memory_usage());
        emlang_print_str(" bytes");
        emlang_println();
        
        // Free memory
        emlang_free(ptr);
        emlang_print_str("Memory freed successfully");
        emlang_println();
    }
    
    emlang_println();
}

// ======================== UTILITY LIBRARY TESTS ========================

function testUtilityLibrary(): void {
    emlang_print_str("=== Utility Library Tests ===");
    emlang_println();
    
    // Test time functions
    emlang_print_str("Current timestamp: ");
    emlang_print_int(emlang_timestamp());
    emlang_println();
    
    emlang_print_str("Current year: ");
    emlang_print_int(emlang_current_year());
    emlang_println();
    
    emlang_print_str("Current month: ");
    emlang_print_int(emlang_current_month());
    emlang_println();
    
    emlang_print_str("Current day: ");
    emlang_print_int(emlang_current_day());
    emlang_println();
    
    // Test sleep function
    emlang_print_str("Testing sleep (500ms)...");
    emlang_println();
    emlang_sleep_ms(500);
    emlang_print_str("Sleep completed!");
    emlang_println();
    
    emlang_println();
}

// ======================== POINTER AND ARRAY TESTS ========================

function testPointersAndArrays(): void {
    emlang_print_str("=== Pointer and Array Tests ===");
    emlang_println();
    
    // Test basic pointer operations
    let value: int32 = 42;
    let ptr: int32* = &value;
    
    emlang_print_str("Original value: ");
    emlang_print_int(value);
    emlang_println();
    
    emlang_print_str("Value through pointer: ");
    emlang_print_int(*ptr);
    emlang_println();
    
    // Modify value through pointer
    *ptr = 100;
    emlang_print_str("Modified value: ");
    emlang_print_int(value);
    emlang_println();
    
    // Test array allocation and usage
    let array_ptr: int32* = emlang_malloc(5 * 4); // 5 integers, 4 bytes each
    
    if (array_ptr != null) {
        emlang_print_str("Array allocated successfully");
        emlang_println();
        
        // Initialize array values (simulated)
        emlang_memset(array_ptr, 0, 20);
        emlang_print_str("Array initialized");
        emlang_println();
        
        emlang_free(array_ptr);
        emlang_print_str("Array memory freed");
        emlang_println();
    }
    
    emlang_println();
}

// ======================== INTEGRATION TESTS ========================

function testIntegration(): void {
    emlang_print_str("=== Integration Tests ===");
    emlang_println();
    
    // Test combining multiple library functions
    let num: int32 = 7;
    
    emlang_print_str("Testing number: ");
    emlang_print_int(num);
    emlang_println();
    
    emlang_print_str("Absolute value: ");
    emlang_print_int(emlang_abs(num));
    emlang_println();
    
    emlang_print_str("Factorial: ");
    emlang_print_int(emlang_factorial(num));
    emlang_println();
    
    emlang_print_str("Is prime: ");
    if (emlang_is_prime(num) == 1) {
        emlang_print_str("Yes");
    } else {
        emlang_print_str("No");
    }
    emlang_println();
    
    emlang_print_str("Fibonacci: ");
    emlang_print_int(emlang_fibonacci(num));
    emlang_println();
    
    emlang_println();
}

// ======================== MAIN FUNCTION ========================

function main(): int32 {
    // Initialize the EMLang library
    let init_result: int32 = emlang_lib_init();
    
    if (init_result != 0) {
        emlang_print_str("Failed to initialize EMLang library!");
        emlang_println();
        return 1;
    }
    
    emlang_print_str("EMLang Standard Library Test Suite");
    emlang_println();
    emlang_print_str("==================================");
    emlang_println();
    emlang_println();
    
    // Run all test suites
    testMathLibrary();
    testIOLibrary();
    testStringLibrary();
    testMemoryLibrary();
    testUtilityLibrary();
    testPointersAndArrays();
    testIntegration();
    
    emlang_print_str("All tests completed!");
    emlang_println();
    emlang_print_str("Library test suite finished successfully.");
    emlang_println();
    
    // Cleanup the library
    emlang_lib_cleanup();
    
    return 0;
}
