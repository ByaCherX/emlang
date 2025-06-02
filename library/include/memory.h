#ifndef EMLANG_MEMORY_H
#define EMLANG_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

// Memory function declarations
void* emlang_malloc(int size);
void emlang_free(void* ptr);
void emlang_memset(void* ptr, int value, int size);

// Extended memory functions
void* emlang_calloc(int count, int size);                       // Allocate and zero-initialize
void* emlang_realloc(void* ptr, int new_size);                  // Resize memory block
int emlang_memcmp(const void* ptr1, const void* ptr2, int size); // Compare memory blocks
void* emlang_memcpy(void* dest, const void* src, int size);     // Copy memory blocks
void* emlang_memmove(void* dest, const void* src, int size);    // Safe copy (overlapping)
long emlang_memory_usage(void);                                 // Get current memory usage
void emlang_memory_stats(long* total_bytes, long* allocation_count); // Get memory statistics

#ifdef __cplusplus
}
#endif

#endif // EMLANG_MEMORY_H
