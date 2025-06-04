#include "emlang_math.h"
#include <stdlib.h>
#include <time.h>

// Static flag to track random initialization
static bool random_initialized = false;

/**
 * @brief Initialize random number generator if not already done
 */
static void ensure_random_init() {
    if (!random_initialized) {
        srand(static_cast<unsigned int>(time(nullptr)));
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
    
    return min + (rand() % (max - min + 1));
}

// ======================== EXTENDED MATH FUNCTIONS ========================

int emlang_min(int a, int b) {
    return (a < b) ? a : b;
}

int emlang_max(int a, int b) {
    return (a > b) ? a : b;
}

int emlang_gcd(int a, int b) {
    if (a < 0) a = -a;  // Handle negative numbers
    if (b < 0) b = -b;
    
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int emlang_lcm(int a, int b) {
    if (a == 0 || b == 0) return 0;
    
    int gcd = emlang_gcd(a, b);
    return (a / gcd) * b;  // Avoid overflow by dividing first
}

int emlang_factorial(int n) {
    if (n < 0) return -1;  // Error case for negative numbers
    if (n == 0 || n == 1) return 1;
    
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int emlang_fibonacci(int n) {
    if (n < 0) return -1;  // Error case for negative numbers
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    int a = 0, b = 1, result = 0;
    for (int i = 2; i <= n; ++i) {
        result = a + b;
        a = b;
        b = result;
    }
    return result;
}

int emlang_is_prime(int n) {
    if (n <= 1) return 0;  // Not prime
    if (n == 2) return 1;  // 2 is prime
    if (n % 2 == 0) return 0;  // Even numbers > 2 are not prime
    
    // Check odd divisors up to sqrt(n)
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int emlang_mod(int a, int b) {
    if (b == 0) return 0;  // Avoid division by zero
    
    int result = a % b;
    // Handle negative numbers to always return positive result
    if (result < 0) {
        result += (b > 0) ? b : -b;
    }
    return result;
}

} // extern "C"
