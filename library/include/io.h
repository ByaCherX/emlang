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

#ifdef __cplusplus
}
#endif

#endif // EMLANG_IO_H
