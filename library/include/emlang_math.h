#ifndef EMLANG_MATH_H
#define EMLANG_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

// Math function declarations
int emlang_abs(int x);
int emlang_pow(int base, int exp);
int emlang_sqrt(int x);
int emlang_random(int min, int max);

// Extended math functions
int emlang_min(int a, int b);
int emlang_max(int a, int b);
int emlang_gcd(int a, int b);  // Greatest Common Divisor
int emlang_lcm(int a, int b);  // Least Common Multiple
int emlang_factorial(int n);
int emlang_fibonacci(int n);
int emlang_is_prime(int n);    // Returns 1 if prime, 0 otherwise
int emlang_mod(int a, int b);  // Modulo operation

#ifdef __cplusplus
}
#endif

#endif // EMLANG_MATH_H
