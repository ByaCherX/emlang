#ifndef EMLANG_LIB_H
#define EMLANG_LIB_H

// Include all EMLang library modules
#include "math.h"
#include "io.h" 
#include "string.h"
#include "memory.h"
#include "utility.h"

#ifdef __cplusplus
extern "C" {
#endif

// ======================== LIBRARY INITIALIZATION ========================

/**
 * @brief Initialize EMLang standard library
 * @return 0 on success, non-zero on error
 */
int emlang_lib_init(void);

/**
 * @brief Cleanup EMLang standard library resources
 */
void emlang_lib_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // EMLANG_LIB_H
