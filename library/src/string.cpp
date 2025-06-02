#include "string.h"
#include <string.h>

extern "C" {

int emlang_strlen(const char* str) {
    if (!str) return 0;
    
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int emlang_strcmp(const char* str1, const char* str2) {
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    
    while (*str1 && *str2 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    
    return static_cast<int>(*str1) - static_cast<int>(*str2);
}

char* emlang_strcpy(char* dest, const char* src, int max_len) {
    if (!dest || !src || max_len <= 0) return dest;
    
    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';  // Null terminate
    
    return dest;
}

// ======================== EXTENDED STRING FUNCTIONS ========================

char* emlang_strcat(char* dest, const char* src, int max_len) {
    if (!dest || !src || max_len <= 0) return dest;
    
    // Find end of dest string
    int dest_len = emlang_strlen(dest);
    int i = 0;
    
    // Append src to dest
    while (i < max_len - dest_len - 1 && src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';  // Null terminate
    
    return dest;
}

const char* emlang_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return nullptr;
    if (*needle == '\0') return haystack;  // Empty needle matches at start
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        // Check if needle matches at current position
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (*n == '\0') return haystack;  // Found complete match
        haystack++;
    }
    
    return nullptr;  // Not found
}

const char* emlang_strchr(const char* str, char c) {
    if (!str) return nullptr;
    
    while (*str) {
        if (*str == c) return str;
        str++;
    }
    
    // Check for null terminator
    if (c == '\0') return str;
    
    return nullptr;  // Character not found
}

int emlang_strncmp(const char* str1, const char* str2, int n) {
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    if (n <= 0) return 0;
    
    int i = 0;
    while (i < n && str1[i] && str2[i] && (str1[i] == str2[i])) {
        i++;
    }
    
    if (i == n) return 0;  // All n characters matched
    return static_cast<int>(str1[i]) - static_cast<int>(str2[i]);
}

char* emlang_strncpy(char* dest, const char* src, int n) {
    if (!dest || !src || n <= 0) return dest;
    
    int i = 0;
    // Copy up to n characters
    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    
    // Pad with null characters if src is shorter than n
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    
    return dest;
}

char* emlang_to_upper(char* str) {
    if (!str) return str;
    
    char* ptr = str;
    while (*ptr) {
        if (*ptr >= 'a' && *ptr <= 'z') {
            *ptr = *ptr - 'a' + 'A';
        }
        ptr++;
    }
    
    return str;
}

char* emlang_to_lower(char* str) {
    if (!str) return str;
    
    char* ptr = str;
    while (*ptr) {
        if (*ptr >= 'A' && *ptr <= 'Z') {
            *ptr = *ptr - 'A' + 'a';
        }
        ptr++;
    }
    
    return str;
}

int emlang_is_numeric(const char* str) {
    if (!str || *str == '\0') return 0;
    
    // Skip leading whitespace
    while (*str == ' ' || *str == '\t') str++;
    
    // Check for optional sign
    if (*str == '+' || *str == '-') str++;
    
    // Must have at least one digit
    if (!(*str >= '0' && *str <= '9')) return 0;
    
    // Check all remaining characters are digits
    while (*str) {
        if (!(*str >= '0' && *str <= '9')) return 0;
        str++;
    }
    
    return 1;
}

char* emlang_trim(char* str) {
    if (!str) return str;
    
    // Trim leading whitespace
    char* start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        str[0] = '\0';
        return str;
    }
    
    // Trim trailing whitespace
    char* end = start + emlang_strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    // Move trimmed string to beginning if needed
    if (start != str) {
        int len = end - start + 1;
        for (int i = 0; i < len; i++) {
            str[i] = start[i];
        }
        str[len] = '\0';
    } else {
        *(end + 1) = '\0';
    }
    
    return str;
}

} // extern "C"
