#include "io.h"
#include <stdio.h>
#include <stdlib.h>

extern "C" {

void emlang_print_int(int value) {
    printf("%d", value);
    fflush(stdout);
}

void emlang_print_str(const char* str) {
    if (str) {
        printf("%s", str);
        fflush(stdout);
    }
}

void emlang_println(void) {
    printf("\n");
    fflush(stdout);
}

int emlang_read_int(void) {
    int value = 0;
    if (scanf("%d", &value) == 1) {
        return value;
    }
    return 0;  // Return 0 on read error
}

// ======================== EXTENDED IO FUNCTIONS ========================

void emlang_print_char(char c) {
    printf("%c", c);
    fflush(stdout);
}

char emlang_read_char(void) {
    char c = 0;
    if (scanf(" %c", &c) == 1) {  // Space before %c to skip whitespace
        return c;
    }
    return '\0';  // Return null character on read error
}

void emlang_print_float(float value) {
    printf("%.6f", value);
    fflush(stdout);
}

float emlang_read_float(void) {
    float value = 0.0f;
    if (scanf("%f", &value) == 1) {
        return value;
    }
    return 0.0f;  // Return 0.0 on read error
}

char* emlang_read_line(char* buffer, int max_len) {
    if (!buffer || max_len <= 1) return nullptr;
    
    // Clear input buffer first
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (fgets(buffer, max_len, stdin) != nullptr) {
        // Remove trailing newline if present
        int len = 0;
        while (buffer[len] != '\0') len++;  // Find length
        
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    
    return nullptr;
}

void emlang_print_hex(int value) {
    printf("0x%X", value);
    fflush(stdout);
}

void emlang_print_binary(int value) {
    printf("0b");
    
    // Print binary representation (32 bits)
    for (int i = 31; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    
    fflush(stdout);
}

void emlang_clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void emlang_set_cursor(int row, int col) {
    if (row < 0) row = 0;
    if (col < 0) col = 0;
    
#ifdef _WIN32
    printf("\033[%d;%dH", row + 1, col + 1);  // ANSI escape sequence
#else
    printf("\033[%d;%dH", row + 1, col + 1);  // ANSI escape sequence
#endif
    fflush(stdout);
}

} // extern "C"
