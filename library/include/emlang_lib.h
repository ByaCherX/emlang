#ifndef EMLANG_LIB_H
#define EMLANG_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

// ======================== LIBRARY INITIALIZATION ========================

/**
 * @brief Initialize EMLang standard library
 * @return 0 on success, non-zero on error
 */
int emlang_lib_init(void);

/**
 * @brief Cleanup EMLang standard library resources
 */
void emlang_lib_cleanup(void);

// ======================== MATH FUNCTIONS ========================

/**
 * @brief Calculate absolute value of an integer
 * @param x Input integer
 * @return Absolute value of x
 */
int emlang_abs(int x);

/**
 * @brief Calculate power of two integers
 * @param base Base number
 * @param exp Exponent
 * @return base^exp
 */
int emlang_pow(int base, int exp);

/**
 * @brief Calculate square root (integer approximation)
 * @param x Input value
 * @return Integer square root of x
 */
int emlang_sqrt(int x);

/**
 * @brief Generate random number between min and max
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random number in range [min, max]
 */
int emlang_random(int min, int max);

// ======================== I/O FUNCTIONS ========================

/**
 * @brief Print integer to stdout
 * @param value Integer value to print
 */
void emlang_print_int(int value);

/**
 * @brief Print string to stdout
 * @param str String to print (null-terminated)
 */
void emlang_print_str(const char* str);

/**
 * @brief Print newline to stdout
 */
void emlang_println(void);

/**
 * @brief Read integer from stdin
 * @return Integer value read from input
 */
int emlang_read_int(void);

// ======================== STRING FUNCTIONS ========================

/**
 * @brief Calculate length of a string
 * @param str Input string (null-terminated)
 * @return Length of the string
 */
int emlang_strlen(const char* str);

/**
 * @brief Compare two strings
 * @param str1 First string
 * @param str2 Second string
 * @return 0 if equal, negative if str1 < str2, positive if str1 > str2
 */
int emlang_strcmp(const char* str1, const char* str2);

/**
 * @brief Copy string from source to destination
 * @param dest Destination buffer
 * @param src Source string
 * @param max_len Maximum characters to copy
 * @return Pointer to destination string
 */
char* emlang_strcpy(char* dest, const char* src, int max_len);

// ======================== MEMORY FUNCTIONS ========================

/**
 * @brief Allocate memory block
 * @param size Size in bytes
 * @return Pointer to allocated memory, NULL on failure
 */
void* emlang_malloc(int size);

/**
 * @brief Free allocated memory
 * @param ptr Pointer to memory block to free
 */
void emlang_free(void* ptr);

/**
 * @brief Set memory block to specified value
 * @param ptr Pointer to memory block
 * @param value Value to set (0-255)
 * @param size Number of bytes to set
 */
void emlang_memset(void* ptr, int value, int size);

#ifdef __cplusplus
}
#endif

#endif // EMLANG_LIB_H
