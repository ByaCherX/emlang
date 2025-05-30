#include "memory.h"
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <malloc.h>
#endif

extern "C" {

void* emlang_malloc(int size) {
    if (size <= 0) return nullptr;
    return std::malloc(static_cast<std::size_t>(size));
}

void emlang_free(void* ptr) {
    if (ptr) {
        std::free(ptr);
    }
}

void emlang_memset(void* ptr, int value, int size) {
    if (ptr && size > 0) {
        std::memset(ptr, value, static_cast<std::size_t>(size));
    }
}

} // extern "C"
