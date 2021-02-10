#ifndef __CLOCK_H
#define __CLOCK_H

#include <inttypes.h>

#ifdef __cplusplus
 extern "C" {
#endif

   /* void clock_setup(); */
   /* void delay(uint32_t ms); */

   /* volatile extern uint32_t systicks; */
   #define getSysTicks() HAL_GetTick()
   /* uint32_t getSysTicks(){ */
   /*   return osKernelSysTick(); */
   /* } */
/* #define getSysTicks() (systicks) */

/* uint32_t osKernelSysTick(void) */

/* uint32_t getSysTicks(){ */
/*   return systicks; */
/* } */
/* #define getSysTicks() (systicks) */

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */
