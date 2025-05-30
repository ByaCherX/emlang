#ifndef EMLANG_MEMORY_H
#define EMLANG_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

// Memory function declarations
void* emlang_malloc(int size);
void emlang_free(void* ptr);
void emlang_memset(void* ptr, int value, int size);

#ifdef __cplusplus
}
#endif

#endif // EMLANG_MEMORY_H
