// Simple Assignment Test

extern function emlang_print_int(value: int32): void;
extern function emlang_print_str(str_param: char*): void;
extern function emlang_println(): void;

function main(): int32 {
    // Basic variable declaration and assignment
    let x: int32 = 10;
    emlang_print_str("Initial x value: ");
    emlang_print_int(x);
    emlang_println();
    
    // Simple assignment
    x = 20;
    emlang_print_str("After assignment x = 20: ");
    emlang_print_int(x);
    emlang_println();
    
    return 0;
}
