#ifndef __QINT_H__
#define __QINT_H__

#include <stdint.h>

#ifdef ARM_CORTEX
#include "arm_math.h"
#define Q15_MUL_Q15(a,b) (__SSAT(((q31_t)(a)*(b))>>15, 16))
#define Q31_MUL_Q31(a,b) (__SSAT(((q63_t)(a)*(b))>>31, 32))
#define FLOAT_TO_Q7(a) (__SSAT((q15_t)((a)*128.0f), 8))
#define FLOAT_TO_Q15(a) (__SSAT((q31_t)((a)*32768.0f), 16))
#define FLOAT_TO_Q31(a) clip_q63_to_q31((q63_t)((a)*2147483648.0f))
#else
#define Q7_CLIP(a) ((a) > Q7_MAX ? Q7_MAX : (a) < Q7_MIN ? Q7_MIN : (a))
#define Q15_CLIP(a) ((a) > Q15_MAX ? Q15_MAX : (a) < Q15_MIN ? Q15_MIN : (a))
#define Q31_CLIP(a) ((a) > Q31_MAX ? Q31_MAX : (a) < Q31_MIN ? Q31_MIN : (a))
#define Q15_MUL_Q15(a,b) Q15_CLIP(((q31_t)(a)*(b))>>15)
#define Q31_MUL_Q31(a,b) Q31_CLIP(((q63_t)(a)*(b))>>31)
#define FLOAT_TO_Q7(a) Q7_CLIP((q15_t)((a)*128.0f))
#define FLOAT_TO_Q15(a) Q15_CLIP((q31_t)((a)*32768.0f))
#define FLOAT_TO_Q31(a) Q31_CLIP((q63_t)((a)*2147483648.0f))
#endif

typedef int8_t q7_t;
typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

#ifndef Q7_MIN
#define Q7_MIN INT8_MIN
#endif
#ifndef Q7_MAX
#define Q7_MAX INT8_MAX
#endif
#ifndef Q15_MIN
#define Q15_MIN INT16_MIN
#endif
#ifndef Q15_MAX
#define Q15_MAX INT16_MAX
#endif
#ifndef Q31_MIN
#define Q31_MIN INT32_MIN
#endif
#ifndef Q31_MAX
#define Q31_MAX INT32_MAX
#endif

#define Q15_DIV_Q15(a,b) (((q31_t)(a)<<15)/(b)) /* non-rounding */
#define Q15_RECIPROCAL(a) (((q31_t)Q15_MAX<<15)/(a))

#define Q7_TO_FLOAT(a) ((float)(a)/128.0f)
#define Q15_TO_FLOAT(a) ((float)(a)/32768.0f)
#define Q31_TO_FLOAT(a) ((float)(a)/2147483648.0f)
#define Q63_TO_FLOAT(a) ((float)(a)/9223372036854775808.0f)

#endif /* defined(__QINT_H__) */
