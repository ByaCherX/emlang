// Advanced Character and String Literal Test
// Tests edge cases, Unicode, and escape sequences

function print_char(value: char): void {
    // Dummy function for testing
    return;
}

function print_str(value: str): void {
    // Dummy function for testing
    return;
}

function main(): int32 {
    // Basic ASCII characters
    let ascii_a: char = 'a';
    let ascii_z: char = 'Z';
    let ascii_0: char = '0';
    let ascii_9: char = '9';
    
    // Escape sequences
    let newline: char = '\n';     // 10
    let tab: char = '\t';         // 9
    let carriage: char = '\r';    // 13
    let backslash: char = '\\';   // 92
    let single_quote: char = '\''; // 39
    let double_quote: char = '"'; // 34
    let null_char: char = '\0';   // 0
    
    // Unicode characters
    let euro: char = '\u{20AC}';     // € symbol (U+20AC)
    let smile: char = '\u{1F600}';   // 😀 grinning face (U+1F600)
    let heart: char = '\u{2764}';    // ❤ red heart (U+2764)
    
    // String literals
    let empty_str: str = "";
    let hello: str = "Hello, World!";
    let escaped_str: str = "Tab:\t Newline:\n Quote:\"";
    let unicode_str: str = "Unicode: €€€ 😀😀😀";
    
    // Print all character values
    print_char(ascii_a);
    print_char(ascii_z);
    print_char(newline);
    print_char(tab);
    print_char(euro);
    print_char(smile);
    print_char(heart);
    
    // Print all strings
    print_str(empty_str);
    print_str(hello);
    print_str(escaped_str);
    print_str(unicode_str);
    
    return 0;
}
