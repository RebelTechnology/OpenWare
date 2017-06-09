#ifndef __basicmaths_h__
#define __basicmaths_h__

#include <stdlib.h>
#include <stdint.h>
/* #include "heap.h" */
/* #include "FreeRTOS.h" */

#define _USE_MATH_DEFINES
/* Definitions of useful mathematical constants
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */
#ifdef ARM_CORTEX
#include "arm_math.h" 
#endif //ARM_CORTEX

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif
//#endif /* __cplusplus */

#ifdef __cplusplus
 extern "C" {
#endif

   float arm_sqrtf(float in);
   uint32_t arm_rand32();

   // fast approximations
   float fastexpf(float x);
   float fastlog2f(float x);
   float fastpow2f(float x);
   float fastpowf(float a, float b);
   float fastatan2f(float a, float b);

#ifdef __cplusplus
}
#endif

#define malloc(x) pvPortMalloc(x)
#define free(x) vPortFree(x)

#ifdef ARM_CORTEX
#define sin(x) arm_sin_f32(x)
#define sinf(x) arm_sin_f32(x)
#define cos(x) arm_cos_f32(x)
#define cosf(x) arm_cos_f32(x)
#define sqrt(x) arm_sqrtf(x)
#define sqrtf(x) arm_sqrtf(x)
#define rand() arm_rand32()

#ifdef __FAST_MATH__ /* set by gcc option -ffast-math */
// fast approximate math functions
#define atan2(x, y) fastatan2f(x, y)
#define atan2f(x, y) fastatan2f(x, y)
/* #define pow(x, y) fastpowf(x, y) */
/* #define powf(x, y) fastpowf(x, y) */
/* Fast exponentiation function, y = e^x */
/* 1.0 / ln(2) = 1.442695041f */
/* #define exp(x) fastpow2f(x * 1.442695041f) */
/* #define expf(x) fastpow2f(x * 1.442695041f) */
#endif

#undef RAND_MAX
#define RAND_MAX UINT32_MAX
#endif //ARM_CORTEX

#endif // __basicmaths_h__
