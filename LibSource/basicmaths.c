#include "basicmaths.h"
#include <stdint.h>

// todo: see
// http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
// http://www.finesse.demon.co.uk/steven/sqrt.html
// http://www.keil.com/forum/7934/
// http://processors.wiki.ti.com/index.php/ARM_compiler_optimizations

static uint32_t r32seed = 33641;

void arm_srand32(uint32_t s){
  r32seed = s;
}

/**
 * generate an unsigned 32bit pseudo-random number using xorshifter algorithm.
 * "Anyone who considers arithmetical methods of producing random digits is, of course, in a state of sin." 
 * -- John von Neumann.
*/
uint32_t arm_rand32(){
  r32seed ^= r32seed << 13;
  r32seed ^= r32seed >> 17;
  r32seed ^= r32seed << 5;
  return r32seed;
}

float arm_sqrtf(float in){
  float out;
#ifdef ARM_CORTEX
  arm_sqrt_f32(in, &out);
#else
  out=sqrtf(in);
#endif
  return out;
}

/* http://stackoverflow.com/questions/6475373/optimizations-for-pow-with-const-non-integer-exponent */
/* http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html */
/* https://hackage.haskell.org/package/approximate-0.2.2.3/src/cbits/fast.c */
float fastpowf(float a, float b) {
  union { float d; int x; } u = { a };
  u.x = (int)(b * (u.x - 1064866805) + 1064866805);
  return u.d;
}

/* ----------------------------------------------------------------------
** Fast approximation to the log2() function.  It uses a two step
** process.  First, it decomposes the floating-point number into
** a fractional component F and an exponent E.  The fraction component
** is used in a polynomial approximation and then the exponent added
** to the result.  A 3rd order polynomial is used and the result
** when computing db20() is accurate to 7.984884e-003 dB.
** http://community.arm.com/thread/6741
** ------------------------------------------------------------------- */
const float log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};
float fastlog2f(float X){
  const float *C = &log2f_approx_coeff[0];
  float Y;
  float F;
  int E;
  // This is the approximation to log2()
  F = frexpf(fabsf(X), &E);
  //  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
  Y = *C++;
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y += E;
  return(Y);
}

#if 1
/* Andrew Simper's pow(2, x) aproximation from the music-dsp list */
float fastpow2f(float x)
{
  int32_t *px = (int32_t*)(&x); // store address of float as long pointer
  const float tx = (x-0.5f) + (3<<22); // temporary value for truncation
  const long  lx = *((int32_t*)&tx) - 0x4b400000; // integer power of 2
  const float dx = x-(float)(lx); // float remainder of power of 2
  x = 1.0f + dx*(0.6960656421638072f + // cubic apporoximation of 2^x
		 dx*(0.224494337302845f +  // for x in the range [0, 1]
		     dx*(0.07944023841053369f)));
  *px += (lx<<23); // add integer power of 2 to exponent
  return x;
}
#else
/* union version by Steve Harris steve@plugin.org.uk */
/* 32 bit "pointer cast" union */
typedef union {
        float f;
        int32_t i;
} ls_pcast32;
float fastpow2f(float x){
  ls_pcast32 *px, tx, lx;
  float dx;
  px = (ls_pcast32 *)&x; // store address of float as long pointer
  tx.f = (x-0.5f) + (3<<22); // temporary value for truncation
  lx.i = tx.i - 0x4b400000; // integer power of 2
  dx = x - (float)lx.i; // float remainder of power of 2
  x = 1.0f + dx * (0.6960656421638072f + // cubic apporoximation of 2^x
		   dx * (0.224494337302845f +  // for x in the range [0, 1]
			 dx * (0.07944023841053369f)));
  (*px).i += (lx.i << 23); // add integer power of 2 to exponent
  return (*px).f;
}
#endif

/* Fast arctan2
 * from http://dspguru.com/dsp/tricks/fixed-point-atan2-with-self-normalization
 */
float fastatan2f(float y, float x){
  const float coeff_1 = M_PI/4;
  const float coeff_2 = 3*M_PI/4;
  float abs_y = fabs(y)+1e-10; // kludge to prevent 0/0 condition
  float r, angle;
  if (x>=0){
    r = (x - abs_y) / (x + abs_y);
    angle = coeff_1 - coeff_1 * r;
  }else{
    r = (x + abs_y) / (abs_y - x);
    angle = coeff_2 - coeff_1 * r;
  }
  if(y < 0)
    return(-angle); // negate if in quad III or IV
  else
    return(angle);
}


#define FASTPOW_PRECISION 11
typedef struct {
  uint32_t  precision_m;
  uint32_t pTable_m[(1<<FASTPOW_PRECISION)*sizeof(unsigned int)];
} PowFast;

#if 0
#include "fastpow.h"
static PowFast pPowFast;
void initFastPow(){
  pPowFast->precision_m = FASTPOW_PRECISION;
  powFastSetTable( pPowFast->pTable_m, pPowFast->precision_m );
}

float fast_pow2f(float f){
   return powFastLookup( f, 1.0f, ppf->pTable_m, ppf->precision_m );
}

float fast_powef(float f){
   return powFastLookup( f, 1.44269504088896f, ppf->pTable_m, ppf->precision_m);
}


float powFast10
(
   const PowFast* powFast,
   float       f
)
{
   const PowFast* ppf = (const PowFast*)powFast;

   return powFastLookup( f, 3.32192809488736f, ppf->pTable_m, ppf->precision_m);
}

#endif
