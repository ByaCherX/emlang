#include "io.h"
#include <cstdio>

extern "C" {

void emlang_print_int(int value) {
    std::printf("%d", value);
    std::fflush(stdout);
}

void emlang_print_str(const char* str) {
    if (str) {
        std::printf("%s", str);
        std::fflush(stdout);
    }
}

void emlang_println(void) {
    std::printf("\n");
    std::fflush(stdout);
}

int emlang_read_int(void) {
    int value = 0;
    if (std::scanf("%d", &value) == 1) {
        return value;
    }
    return 0;  // Return 0 on read error
}

} // extern "C"
