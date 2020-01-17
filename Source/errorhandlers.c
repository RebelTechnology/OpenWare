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
#if defined OWL_PEDAL || defined OWL_MODULAR || defined OWL_BIOSIGNALS
  if(code != NO_ERROR)
    setLed(0, RED_COLOUR);
#endif
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
#ifdef DEBUG
  volatile unsigned int hfsr = SCB->MMFAR;
  volatile unsigned int cfsr = SCB->CFSR;
  volatile unsigned int msp = __get_MSP();
  volatile unsigned int psp = __get_PSP();
  (void)hfsr;
  (void)cfsr;
  (void)msp;
  (void)psp;
#endif
  assert_failed(0, 0);
}

void BusFault_Handler(void){ 
  errorcode = BUS_ERROR;
#ifdef DEBUG
  volatile unsigned int hfsr = SCB->BFAR;
  volatile unsigned int cfsr = SCB->CFSR;
  volatile unsigned int msp = __get_MSP();
  volatile unsigned int psp = __get_PSP();
  (void)hfsr;
  (void)cfsr;
  (void)msp;
  (void)psp;
#endif
  assert_failed(0, 0);
}

void UsageFault_Handler(void){ 
  errorcode = USAGE_ERROR;
#ifdef DEBUG
  volatile unsigned int cfsr = SCB->CFSR;
  volatile unsigned int msp = __get_MSP();
  volatile unsigned int psp = __get_PSP();
  (void)cfsr;
  (void)msp;
  (void)psp;
#endif
  assert_failed(0, 0);
}

void DebugMon_Handler(void){ 
  for(;;);
}

/**
 * @see http://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html
 */
#if 0
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress) {
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used. If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
  volatile uint32_t r0;
  volatile uint32_t r1;
  volatile uint32_t r2;
  volatile uint32_t r3;
  volatile uint32_t r12;
  volatile uint32_t lr; /* Link register. */
  volatile uint32_t pc; /* Program counter. */
  volatile uint32_t psr;/* Program status register. */

  r0 = pulFaultStackAddress[0];
  r1 = pulFaultStackAddress[1];
  r2 = pulFaultStackAddress[2];
  r3 = pulFaultStackAddress[3];

  r12 = pulFaultStackAddress[4];
  lr = pulFaultStackAddress[5];
  pc = pulFaultStackAddress[6];
  psr = pulFaultStackAddress[7];

  /* When the following line is hit, the variables contain the register values. */
  for (;;);
}

The fault handler implementation calls a function called
   prvGetRegistersFromStack().
static void HardFault_Handler(void) {
  __asm volatile (
     " tst lr, #4                                                \n"
     " ite eq                                                    \n"
     " mrseq r0, msp                                             \n"
     " mrsne r0, psp                                             \n"
     " ldr r1, [r0, #24]                                         \n"
     " ldr r2, handler2_address_const                            \n"
     " bx r2                                                     \n"
     " handler2_address_const: .word prvGetRegistersFromStack    \n"
     );
}
#endif

/*
  HardFault_Handler from http://blog.frankvh.com/2011/12/07/cortex-m3-m4-hard-fault-handler/
*/
#if 0
void HardFault_Handler(void) __attribute__((naked));
 
void HardFault_Handler(void) {
/* HardFault_Handler: */
  __asm("TST LR, #4");
  __asm("ITE EQ");
  __asm("MRSEQ R0, MSP");
  __asm("MRSNE R0, PSP");
  __asm("B hard_fault_handler_c");
}

void hard_fault_handler_c (unsigned int * hardfault_args){
  volatile unsigned int stacked_r0;
  volatile unsigned int stacked_r1;
  volatile unsigned int stacked_r2;
  volatile unsigned int stacked_r3;
  volatile unsigned int stacked_r12;
  volatile unsigned int stacked_lr;
  volatile unsigned int stacked_pc;
  volatile unsigned int stacked_psr;
 
  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);
 
  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);
}
#endif

/*
see 
https://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/
*/
void HardFault_Handler(void){
  errorcode = HARDFAULT_ERROR;
#ifdef DEBUG
  volatile unsigned int hfsr = SCB->HFSR;
  volatile unsigned int cfsr = SCB->CFSR;
  volatile unsigned int msp = __get_MSP();
  volatile unsigned int psp = __get_PSP();
  (void)hfsr;
  (void)cfsr;
  (void)msp;
  (void)psp;
#endif
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

/*

void UART4_IRQHandler(void){
  assert_failed(0, 0);
}

void UART5_IRQHandler(void){
  assert_failed(0, 0);
}

void USART6_IRQHandler(void){
  assert_failed(0, 0);
}

void USART3_IRQHandler(void){
  assert_failed(0, 0);
} 
void USART2_IRQHandler(void){
  assert_failed(0, 0);
} 
void USART1_IRQHandler(void){
  assert_failed(0, 0);
} 

void FLASH_IRQHandler(void){
  assert_failed(0, 0);
}

void TIM8_UP_TIM13_IRQHandler(void){
  assert_failed(0, 0);
}

void TIM8_TRG_COM_TIM14_IRQHandler(void){
  assert_failed(0, 0);
}

void TIM8_CC_IRQHandler(void){
  assert_failed(0, 0);
}
*/
