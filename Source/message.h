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

/* #define CONFIGURATION_ERROR_STATUS -30 */
/* #define OUT_OF_MEMORY_ERROR_STATUS -20 */
/* #define CHECKSUM_ERROR_STATUS      -10 */

#ifdef __cplusplus
 extern "C" {
#endif

   void debugMessage(const char* msg);
   const char* getDebugMessage();
   void error(int8_t code, const char* reason);
   void assert_failed(uint8_t* location, uint32_t line);

   char* msg_itoa(int val, int base);
   char* msg_ftoa(float val, int base);
#ifdef __cplusplus
}

   char* msg_itoa(int val, int base, int pad);

class Debug {
private:
  uint8_t pos = 0;
public:
  void print(char arg);
  void print(float arg);
  void print(int arg);
  void print(const char* arg);
};
extern Debug debug;

template<class T>
inline Debug &operator <<(Debug &obj, T arg)
#ifdef OWL_PRISM
{ obj.print(arg); return obj; }
#else
{ return obj; }
#endif

void debugMessage(const char* msg, int);
void debugMessage(const char* msg, int, int, int);
void debugMessage(const char* msg, float);
void debugMessage(const char* msg, float, float);
void debugMessage(const char* msg, float, float, float);
void assert_failed(const char* msg, const char* location, int line);

#endif

#endif /* __MESSAGE_H */
