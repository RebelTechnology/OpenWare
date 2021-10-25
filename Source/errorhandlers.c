#include "errorhandlers.h"
#include "device.h"
#include "Owl.h"

#ifdef DEBUG
#include <stdio.h>
#endif

volatile int8_t errorcode = 0;
const char* errormsg = 0;

void error(int8_t code, const char* reason){
  setErrorMessage(code, reason);
  /* assert_param(0); */
  if(code != NO_ERROR)
    onError(code, reason);
#ifdef DEBUG
  // assuming semihosting enabled
  if(reason != NULL)
    printf("ERROR: %s\n", reason);
#endif
}

int8_t getErrorStatus(){
  return errorcode;
}

const char* getErrorMessage(){
  return errormsg;
}

void setErrorStatus(int8_t err){
  if(errorcode == NO_ERROR || err == NO_ERROR)
    errorcode = err;
}

void setErrorMessage(int8_t err, const char* msg){
  if(errorcode == NO_ERROR || err == NO_ERROR){
    errorcode = err;
    errormsg = msg;
  }
}

/* exception handlers - so we know what's failing */
void NMI_Handler(void){
  errorcode = NMI_ERROR;
  assert_failed(0, 0);
}

void MemManage_Handler(void){ 
  errorcode = MEM_ERROR;
  assert_failed(0, 0);
}

void BusFault_Handler(void){ 
  errorcode = BUS_ERROR;
  assert_failed(0, 0);
}

void UsageFault_Handler(void){ 
  errorcode = USAGE_ERROR;
  assert_failed(0, 0);
}

void DebugMon_Handler(void){ 
  for(;;);
}

void HardFault_Handler(void){
  errorcode = HARDFAULT_ERROR;
  assert_failed(0, 0);
}

/* defined by FreeRTOS
void SVC_Handler(void){ 
  for(;;);
}

void PendSV_Handler(void){ 
  for(;;);
}
*/

void WWDG_IRQHandler(void) {
  assert_failed(0, 0);
}

void PVD_IRQHandler(void) {
  assert_failed(0, 0);
}

void FPU_IRQHandler(void){
  assert_failed(0, 0);
}
