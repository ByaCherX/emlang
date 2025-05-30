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

#ifdef __cplusplus
}
#endif

#endif // EMLANG_MATH_H
