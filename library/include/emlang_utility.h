#ifndef EMLANG_UTILITY_H
#define EMLANG_UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif

// ======================== TIME & DATE UTILITIES ========================
/**
 * @brief Get current timestamp in seconds since epoch
 * @return Current timestamp
 */
long emlang_timestamp(void);

/**
 * @brief Get current year
 * @return Current year (e.g., 2024)
 */
int emlang_current_year(void);

/**
 * @brief Get current month (1-12)
 * @return Current month
 */
int emlang_current_month(void);

/**
 * @brief Get current day of month (1-31)
 * @return Current day
 */
int emlang_current_day(void);

// ======================== SYSTEM UTILITIES ========================
/**
 * @brief Sleep for specified milliseconds
 * @param ms Milliseconds to sleep
 */
void emlang_sleep_ms(int ms);

/**
 * @brief Execute system command
 * @param command Command to execute
 * @return Exit code of the command
 */
int emlang_system_exec(const char* command);

/**
 * @brief Get environment variable value
 * @param name Environment variable name
 * @return Environment variable value or NULL if not found
 */
const char* emlang_get_env(const char* name);

// ======================== ARRAY UTILITIES ========================
/**
 * @brief Find minimum value in integer array
 * @param arr Array of integers
 * @param size Array size
 * @return Minimum value
 */
int emlang_array_min(const int* arr, int size);

/**
 * @brief Find maximum value in integer array
 * @param arr Array of integers
 * @param size Array size
 * @return Maximum value
 */
int emlang_array_max(const int* arr, int size);

/**
 * @brief Calculate sum of integer array
 * @param arr Array of integers
 * @param size Array size
 * @return Sum of array elements
 */
int emlang_array_sum(const int* arr, int size);

/**
 * @brief Sort integer array in ascending order
 * @param arr Array of integers
 * @param size Array size
 */
void emlang_array_sort(int* arr, int size);

/**
 * @brief Reverse integer array
 * @param arr Array of integers
 * @param size Array size
 */
void emlang_array_reverse(int* arr, int size);

// ======================== BIT MANIPULATION ========================
/**
 * @brief Set bit at specified position
 * @param value Original value
 * @param position Bit position (0-based)
 * @return Value with bit set
 */
int emlang_set_bit(int value, int position);

/**
 * @brief Clear bit at specified position
 * @param value Original value
 * @param position Bit position (0-based)
 * @return Value with bit cleared
 */
int emlang_clear_bit(int value, int position);

/**
 * @brief Toggle bit at specified position
 * @param value Original value
 * @param position Bit position (0-based)
 * @return Value with bit toggled
 */
int emlang_toggle_bit(int value, int position);

/**
 * @brief Check if bit is set at specified position
 * @param value Value to check
 * @param position Bit position (0-based)
 * @return 1 if bit is set, 0 otherwise
 */
int emlang_is_bit_set(int value, int position);

/**
 * @brief Count number of set bits
 * @param value Value to check
 * @return Number of set bits
 */
int emlang_count_bits(int value);

// ======================== HASH UTILITIES ========================
/**
 * @brief Simple hash function for strings
 * @param str String to hash
 * @return Hash value
 */
unsigned int emlang_hash_string(const char* str);

/**
 * @brief Simple hash function for integers
 * @param value Integer to hash
 * @return Hash value
 */
unsigned int emlang_hash_int(int value);

#ifdef __cplusplus
}
#endif

#endif // EMLANG_UTILITY_H
