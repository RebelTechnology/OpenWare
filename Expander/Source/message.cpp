#include "message.h"
#include <string.h>

#define MESSAGE_BUFFER_SIZE 128
static char buffer[MESSAGE_BUFFER_SIZE];
static const char hexnumerals[] = "0123456789abcdef";

#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif /* abs */

char* msg_itoa(int val, int base){
  return msg_itoa(val, base, 0);
}

char* msg_itoa(int val, int base, int pad){
  static char buf[13] = {0};
  int i = 11;
  unsigned int part = abs(val);
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(i && (--pad > 0 || part));
  if(val < 0)
    buf[i--] = '-';
  return &buf[i+1];
}

char* msg_ftoa(float val, int base){
  static char buf[16] = {0};
  int i = 14;
  // print 4 decimal points
  unsigned int part = abs((int)((val-int(val))*10000));
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(i>10);
  buf[i--] = '.';
  part = abs(int(val));
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(part && i);
  if(val < 0.0f)
    buf[i--] = '-';
  return &buf[i+1];
}

static const char* debugmessage;
const char* getDebugMessage(){
  return debugmessage;
}

// semihosting
// static void put_char(char c){
//     asm (
//     "mov r0, #0x03\n"   /* SYS_WRITEC */
//     "mov r1, %[msg]\n"
//     "bkpt #0xAB\n"
//     :
//     : [msg] "r" (&c)
//     : "r0", "r1"
//     );
// }

void setDebugMessage(const char* msg){
  // size_t len = strnlen(msg, sizeof(buffer));
  // for(int i=0; i<len; ++i)
  //   put_char(msg[i]);
  debugmessage = msg;
}

void debugMessage(const char* msg){
  strlcpy(buffer, msg, MESSAGE_BUFFER_SIZE);
}

void debugMessage(const char* msg, int a){
  char* p = buffer;
  p = stpncpy(p, msg, MESSAGE_BUFFER_SIZE-16);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(a, 10));
  setDebugMessage(buffer);
}

void debugMessage(const char* msg, int a, int b){
  char* p = buffer;
  p = stpncpy(p, msg, MESSAGE_BUFFER_SIZE-32);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(a, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(b, 10));
  setDebugMessage(buffer);
}

void debugMessage(const char* msg, int a, int b, int c){
  char* p = buffer;
  p = stpncpy(p, msg, 32);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(a, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(b, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_itoa(c, 10));
  setDebugMessage(buffer);
}

void debugMessage(const char* msg, float a){
  char* p = buffer;
  p = stpncpy(p, msg, MESSAGE_BUFFER_SIZE-16);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(a, 10));
  setDebugMessage(buffer);
}

void debugMessage(const char* msg, float a, float b){
  char* p = buffer;
  p = stpncpy(p, msg, MESSAGE_BUFFER_SIZE-32);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(a, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(b, 10));
  setDebugMessage(buffer);
}

void debugMessage(const char* msg, float a, float b, float c){
  char* p = buffer;
  p = stpncpy(p, msg, MESSAGE_BUFFER_SIZE-32);
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(a, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(b, 10));
  p = stpcpy(p, (const char*)" ");
  p = stpcpy(p, msg_ftoa(c, 10));
  setDebugMessage(buffer);
}

Debug debug;

void Debug::print(char arg){
  if(getDebugMessage() != buffer)
    pos = 0;
  if(pos < sizeof(buffer)-1)
    buffer[pos++] = arg;
  setDebugMessage(buffer);
}

void Debug::print(const char* arg){
  if(getDebugMessage() != buffer)
    pos = 0;
  char* p = buffer+pos;
  p = stpncpy(p, arg, sizeof(buffer)-pos);
  pos = p-buffer;
  setDebugMessage(buffer);
}

void Debug::print(float arg){
  if(getDebugMessage() != buffer)
    pos = 0;
  char* p = buffer+pos;
  p = stpncpy(p, msg_ftoa(arg, 10), sizeof(buffer)-pos);
  pos = p-buffer;
  setDebugMessage(buffer);
}

void Debug::print(int arg){ 
  if(getDebugMessage() != buffer)
    pos = 0;
  char* p = buffer+pos;
  p = stpncpy(p, msg_itoa(arg, 10), sizeof(buffer)-pos);
  pos = p-buffer;
  setDebugMessage(buffer);
}
