#include "string.h"
#include <cstring>

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

} // extern "C"
