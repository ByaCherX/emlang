// Character ve String literal test dosyası

// Dummy print function for testing
function print_int32(value: int32): void {
    // Implementation would be provided by runtime
    return;
}

function main(): int32 {
    // Character literals
    let simple_char: char = 'A';
    let newline_char: char = '\n';
    let tab_char: char = '\t';
    let backslash_char: char = '\\';
    let quote_char: char = '\'';
    let unicode_char: char = '\u{1F600}'; // Emoji grinning face
    
    // String literals  
    let simple_string: str = "Hello, World!";
    let escaped_string: str = "Line 1\nLine 2\tTabbed";
    let quote_string: str = "He said \"Hello\"";
    
    // Output character values (ASCII/Unicode code points)
    print_int32(simple_char);      // Should print 65 (ASCII 'A')
    print_int32(newline_char);     // Should print 10 (ASCII '\n')
    print_int32(tab_char);         // Should print 9 (ASCII '\t')
    print_int32(unicode_char);     // Should print 128512 (Unicode grinning face)
    
    return 0;
}
