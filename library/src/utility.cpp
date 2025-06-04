#include "emlang_utility.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
// Forward declare only the necessary Windows functions we need
typedef unsigned long DWORD;
extern "C" {
    void __stdcall Sleep(DWORD dwMilliseconds);
}
#else
#include <unistd.h>
#include <sys/time.h>
typedef unsigned int useconds_t;
#endif

extern "C" {

// ======================== TIME & DATE UTILITIES ========================

long emlang_timestamp(void) {
    return static_cast<long>(time(nullptr));
}

int emlang_current_year(void) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return timeinfo ? (timeinfo->tm_year + 1900) : 0;
}

int emlang_current_month(void) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return timeinfo ? (timeinfo->tm_mon + 1) : 0;  // tm_mon is 0-11, we want 1-12
}

int emlang_current_day(void) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return timeinfo ? timeinfo->tm_mday : 0;
}

// ======================== SYSTEM UTILITIES ========================

void emlang_sleep_ms(int ms) {
    if (ms <= 0) return;
    
#ifdef _WIN32
    Sleep(static_cast<DWORD>(ms));
#else
    usleep(static_cast<useconds_t>(ms * 1000));
#endif
}

int emlang_system_exec(const char* command) {
    if (!command) return -1;
    return system(command);
}

const char* emlang_get_env(const char* name) {
    if (!name) return nullptr;
    return getenv(name);
}

// ======================== ARRAY UTILITIES ========================

int emlang_array_min(const int* arr, int size) {
    if (!arr || size <= 0) return 0;
    
    int min_val = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
    }
    return min_val;
}

int emlang_array_max(const int* arr, int size) {
    if (!arr || size <= 0) return 0;
    
    int max_val = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

int emlang_array_sum(const int* arr, int size) {
    if (!arr || size <= 0) return 0;
    
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

void emlang_array_sort(int* arr, int size) {
    if (!arr || size <= 1) return;
    
    // Simple bubble sort implementation
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // Swap elements
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void emlang_array_reverse(int* arr, int size) {
    if (!arr || size <= 1) return;
    
    int start = 0;
    int end = size - 1;
    
    while (start < end) {
        // Swap elements
        int temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        
        start++;
        end--;
    }
}

// ======================== BIT MANIPULATION ========================

int emlang_set_bit(int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value | (1 << position);
}

int emlang_clear_bit(int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value & ~(1 << position);
}

int emlang_toggle_bit(int value, int position) {
    if (position < 0 || position >= 32) return value;
    return value ^ (1 << position);
}

int emlang_is_bit_set(int value, int position) {
    if (position < 0 || position >= 32) return 0;
    return (value & (1 << position)) != 0 ? 1 : 0;
}

int emlang_count_bits(int value) {
    int count = 0;
    
    // Count set bits using Brian Kernighan's algorithm
    while (value) {
        count++;
        value &= (value - 1);  // Clear the lowest set bit
    }
    
    return count;
}

// ======================== HASH UTILITIES ========================

unsigned int emlang_hash_string(const char* str) {
    if (!str) return 0;
    
    unsigned int hash = 5381;  // djb2 hash algorithm
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    
    return hash;
}

unsigned int emlang_hash_int(int value) {
    // Simple integer hash function
    unsigned int hash = static_cast<unsigned int>(value);
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}

} // extern "C"
