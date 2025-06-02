#ifndef EMLANG_STRING_H
#define EMLANG_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

// String function declarations
int emlang_strlen(const char* str);
int emlang_strcmp(const char* str1, const char* str2);
char* emlang_strcpy(char* dest, const char* src, int max_len);

// Extended string functions
char* emlang_strcat(char* dest, const char* src, int max_len);  // String concatenation
const char* emlang_strstr(const char* haystack, const char* needle);  // Find substring
const char* emlang_strchr(const char* str, char c);             // Find character
int emlang_strncmp(const char* str1, const char* str2, int n);  // Compare first n characters
char* emlang_strncpy(char* dest, const char* src, int n);       // Copy first n characters
char* emlang_to_upper(char* str);                               // Convert to uppercase
char* emlang_to_lower(char* str);                               // Convert to lowercase
int emlang_is_numeric(const char* str);                         // Check if string is numeric
char* emlang_trim(char* str);                                   // Trim whitespace
char* emlang_str_trim(char* str);                               // Remove leading/trailing whitespace

#ifdef __cplusplus
}
#endif

#endif // EMLANG_STRING_H
