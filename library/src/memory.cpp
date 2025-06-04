#include "emlang_memory.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <malloc.h>
#endif

extern "C" {

void* emlang_malloc(int size) {
    if (size <= 0) return nullptr;
    return malloc(static_cast<size_t>(size));
}

void emlang_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

void emlang_memset(void* ptr, int value, int size) {
    if (ptr && size > 0) {
        // Manual memset implementation for better compatibility
        unsigned char* p = static_cast<unsigned char*>(ptr);
        unsigned char val = static_cast<unsigned char>(value);
        for (int i = 0; i < size; i++) {
            p[i] = val;
        }
    }
}

// ======================== EXTENDED MEMORY FUNCTIONS ========================

void* emlang_calloc(int count, int size) {
    if (count <= 0 || size <= 0) return nullptr;
    
    size_t total_size = static_cast<size_t>(count) * static_cast<size_t>(size);
    void* ptr = malloc(total_size);
    
    if (ptr) {
        emlang_memset(ptr, 0, static_cast<int>(total_size));
    }
    
    return ptr;
}

void* emlang_realloc(void* ptr, int new_size) {
    if (new_size <= 0) {
        if (ptr) emlang_free(ptr);
        return nullptr;
    }
    
    return realloc(ptr, static_cast<size_t>(new_size));
}

int emlang_memcmp(const void* ptr1, const void* ptr2, int size) {
    if (!ptr1 && !ptr2) return 0;
    if (!ptr1) return -1;
    if (!ptr2) return 1;
    if (size <= 0) return 0;
    
    const unsigned char* p1 = static_cast<const unsigned char*>(ptr1);
    const unsigned char* p2 = static_cast<const unsigned char*>(ptr2);
    
    for (int i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return static_cast<int>(p1[i]) - static_cast<int>(p2[i]);
        }
    }
    
    return 0;
}

void* emlang_memcpy(void* dest, const void* src, int size) {
    if (!dest || !src || size <= 0) return dest;
    
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    
    for (int i = 0; i < size; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void* emlang_memmove(void* dest, const void* src, int size) {
    if (!dest || !src || size <= 0) return dest;
    
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    
    // Handle overlapping memory regions
    if (d < s) {
        // Copy forward
        for (int i = 0; i < size; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy backward
        for (int i = size - 1; i >= 0; i--) {
            d[i] = s[i];
        }
    }
    // If d == s, no operation needed
    
    return dest;
}

// Simple memory tracking variables
static long total_allocated = 0;
static long allocation_count = 0;

long emlang_memory_usage(void) {
    return total_allocated;
}

void emlang_memory_stats(long* total_bytes, long* allocation_count_out) {
    if (total_bytes) *total_bytes = total_allocated;
    if (allocation_count_out) *allocation_count_out = allocation_count;
}

// Override malloc/free to track memory usage
void* emlang_tracked_malloc(int size) {
    void* ptr = emlang_malloc(size);
    if (ptr) {
        total_allocated += size;
        allocation_count++;
    }
    return ptr;
}

void emlang_tracked_free(void* ptr) {
    if (ptr) {
        // In a real implementation, we'd need to track allocation sizes
        // For simplicity, we'll just decrement the count
        allocation_count--;
        emlang_free(ptr);
    }
}

} // extern "C"
