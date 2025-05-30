#include "math.h"
#include <cstdlib>
#include <ctime>

// Static flag to track random initialization
static bool random_initialized = false;

/**
 * @brief Initialize random number generator if not already done
 */
static void ensure_random_init() {
    if (!random_initialized) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        random_initialized = true;
    }
}

extern "C" {

int emlang_abs(int x) {
    return (x < 0) ? -x : x;
}

int emlang_pow(int base, int exp) {
    if (exp < 0) return 0;  // Handle negative exponents as 0 for integer math
    if (exp == 0) return 1;
    
    int result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

int emlang_sqrt(int x) {
    if (x < 0) return -1;  // Error case for negative numbers
    if (x == 0 || x == 1) return x;
    
    // Binary search for integer square root
    int start = 1, end = x, ans = 0;
    while (start <= end) {
        int mid = (start + end) / 2;
        
        // To avoid overflow, check if mid*mid <= x
        if (mid <= x / mid) {
            start = mid + 1;
            ans = mid;
        } else {
            end = mid - 1;
        }
    }
    return ans;
}

int emlang_random(int min, int max) {
    ensure_random_init();
    
    if (min > max) {
        // Swap if min > max
        int temp = min;
        min = max;
        max = temp;
    }
    
    if (min == max) return min;
    
    return min + (std::rand() % (max - min + 1));
}

} // extern "C"
