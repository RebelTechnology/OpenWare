#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdint.h>
#include "errorhandlers.h"

/* #ifdef DDEBUG */
/* #define DEBUG_ASSERT(cond, msg) if(!(cond)){debugAssertFailed(msg, __FILE__, __LINE__);} */
/* #else */
/* #define DEBUG_ASSERT(x, y) */
/* #endif */
/* #define ASSERT_TRUE(cond) if(!(cond)){debugAssertFailed("Assertion failed", __PRETTY_FUNCTION__, __LINE__);} */
/* #define ASSERT_FALSE(cond) if(cond){debugAssertFailed("Assertion failed", __PRETTY_FUNCTION__, __LINE__);} */
/* #define ASSERT(cond, msg) if(!(cond)){debugAssertFailed(msg, __PRETTY_FUNCTION__, __LINE__);} */

/* #define ASSERT(cond, msg) if(!(cond)){assert_failed(msg, __PRETTY_FUNCTION__, __LINE__);} */

/* #define NO_ERROR         0x00 */
/* #define HARDFAULT_ERROR  0x10 */
/* #define BUS_ERROR        0x20 */
/* #define MEM_ERROR        0x30 */
/* #define NMI_ERROR        0x40 */
/* #define USAGE_ERROR      0x50 */
/* #define PROGRAM_ERROR    0x60 */
/* #define FLASH_ERROR      0x70 */
/* #define USB_ERROR        0x80 */

/* #define CONFIGURATION_ERROR_STATUS -30 */
/* #define OUT_OF_MEMORY_ERROR_STATUS -20 */
/* #define CHECKSUM_ERROR_STATUS      -10 */

#ifdef __cplusplus
 extern "C" {
#endif

   void debugMessage(const char* msg);
   void error(int8_t code, const char* reason);
   void assert_failed(uint8_t* location, uint32_t line);

   char* msg_itoa(int val, int base);
   char* msg_ftoa(float val, int base);
#ifdef __cplusplus
}

void debugMessage(const char* msg, int);
void debugMessage(const char* msg, int, int, int);
void debugMessage(const char* msg, float);
void debugMessage(const char* msg, float, float);
void debugMessage(const char* msg, float, float, float);
void assert_failed(const char* msg, const char* location, int line);

#endif

#endif /* __MESSAG£_H */
