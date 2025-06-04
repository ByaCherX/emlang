#ifndef EMLANG_IO_H
#define EMLANG_IO_H

#ifdef __cplusplus
extern "C" {
#endif

// I/O function declarations
void emlang_print_int(int value);
void emlang_print_str(const char* str);
void emlang_println(void);
int emlang_read_int(void);

// Extended I/O functions
void emlang_print_char(char c);                                 // Print single character
char emlang_read_char(void);                                    // Read single character
void emlang_print_float(float value);                           // Print floating point
float emlang_read_float(void);                                  // Read floating point
char* emlang_read_line(char* buffer, int max_size);             // Read entire line
void emlang_print_hex(int value);                               // Print in hexadecimal
void emlang_print_binary(int value);                            // Print in binary
void emlang_clear_screen(void);                                 // Clear console screen
void emlang_set_cursor(int row, int col);                       // Set cursor position

#ifdef __cplusplus
}
#endif

#endif // EMLANG_IO_H
