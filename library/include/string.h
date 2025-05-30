#ifndef EMLANG_STRING_H
#define EMLANG_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

// String function declarations
int emlang_strlen(const char* str);
int emlang_strcmp(const char* str1, const char* str2);
char* emlang_strcpy(char* dest, const char* src, int max_len);

#ifdef __cplusplus
}
#endif

#endif // EMLANG_STRING_H
