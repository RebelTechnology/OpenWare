/**
  ******************************************************************************
  * @file    crash_handler.h
  * @author  AMS - VMA RF Application team
  * @version V1.1.0
  * @date    25-November-2016
  * @brief   This header file defines the crash handler framework useful for 
  *          application issues debugging
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  ******************************************************************************
  */
#ifndef __CRASH_HANDLER_H_
#define __CRASH_HANDLER_H_

#include <stdint.h>
#include <string.h>

/**
 * @brief Number of word for the Exception RAM Locations 
 */
#define NMB_OF_EXCEP_RAM_WORD 10

/**
 * @brief Base address to store the Crash Information
 */
#define CRASH_INFO_BASE_ADDRESS 0x20000008
#define CRASH_RAM_SIZE          40

/**
 * @brief Signature Information: ASSERT_SIGNATURE
 */
#define ASSERT_SIGNATURE     0xBCEC0000
/**
 * @brief Signature Information: NMI_SIGNATURE
 */
#define NMI_SIGNATURE        0xBCEC0001
/**
 * @brief Signature Information: HARD_FAULT_SIGNATURE
 */
#define HARD_FAULT_SIGNATURE 0xBCEC0002
/**
 * @brief Signature Information: WATCHDOG_SIGNATURE
 */
#define WATCHDOG_SIGNATURE   0xBCEC0003


/**
 * @brief Get Crash Information utility
 */
#define GET_CRASH_INFO(PTR)     { PTR = *(crash_info_t *)CRASH_INFO_BASE_ADDRESS; \
                                  memset((void *)CRASH_INFO_BASE_ADDRESS, 0, CRASH_RAM_SIZE);}

/**
 * @brief Typedef for the overall crash information (stack pointer, programm counter, registers, ...)
 */
typedef PACKED(struct) crash_infoS {
  uint32_t signature;
  uint32_t SP;
  uint32_t R0;
  uint32_t R1;
  uint32_t R2;
  uint32_t R3;
  uint32_t R12;
  uint32_t LR;
  uint32_t PC;
  uint32_t xPSR;
} crash_info_t;

/**
 * @brief Macro to store in RAM the crash information. 
 *
 * All the information stored are words (32 bit):
 * - Crash Signature
 * - SP
 * - R0
 * - R1
 * - R2
 * - R3
 * - R12
 * - LR
 * - PC
 * - xPSR
 */
  //__ASM volatile ("MOV %0, SP" : "=r" (reg_content) ); 
/*  reg_content = __get_MSP() ;         to be compliant with Keil */
#define CRASH_HANDLER(signature) {                                               \
  volatile uint32_t * crash_info = (volatile uint32_t *)CRASH_INFO_BASE_ADDRESS; \
  register uint32_t reg_content;                                                 \
  /* Init to zero the crash_info RAM locations */                                \
  for (reg_content=0; reg_content<NMB_OF_EXCEP_RAM_WORD; reg_content++) {        \
    *crash_info = 0;                                                             \
    crash_info++;                                                                \
  }                                                                              \
  crash_info = (volatile uint32_t *)CRASH_INFO_BASE_ADDRESS;                     \
  /* Store Crash Signature */                                                    \
  *crash_info = signature;                                                       \
  crash_info++;                                                                  \
  /* Store SP register */                                                        \
   reg_content = __get_MSP() ;                                                   \
  *crash_info = reg_content;                                                     \
  crash_info++;                                                                  \
  if ((reg_content >= 0x20000000) && (reg_content <= 0x20005FFF)) {              \
    /* Store exception R0 */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x00);                      \
    crash_info++;                                                                \
    /* Store exception R1 */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x04);                      \
    crash_info++;                                                                \
    /* Store exception R2 */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x08);                      \
    crash_info++;                                                                \
    /* Store exception R3 */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x0C);                      \
    crash_info++;                                                                \
    /* Store exception R12 */                                                    \
    *crash_info = *(volatile uint32_t *)(reg_content+0x10);                      \
    crash_info++;                                                                \
    /* Store exception LR */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x14);                      \
    crash_info++;                                                                \
    /* Store exception PC */                                                     \
    *crash_info = *(volatile uint32_t *)(reg_content+0x18);                      \
    crash_info++;                                                                \
    /* Store exception xPSR */                                                   \
    *crash_info = *(volatile uint32_t *)(reg_content+0x1C);                      \
  }                                                                              \
}                                                                                \

/**
 * @brief Macro to store in RAM the register information where an assert is verified.
 *
 * All the information stored are words (32 bit):
 * - Assert Signature
 * - SP
 * - R0
 * - R1
 * - R2
 * - R3
 * - R12
 * - LR
 * - PC
 * - xPSR
 */
#define ASSERT_HANDLER(expression) {                                              \
  volatile uint32_t * crash_info = (volatile uint32_t *)CRASH_INFO_BASE_ADDRESS;  \
  register uint32_t reg_content;                                                  \
  if (!(expression)) {                                                            \
   /* Init to zero the crash_info RAM locations */                                \
   for (reg_content=0; reg_content<NMB_OF_EXCEP_RAM_WORD; reg_content++) {        \
     *crash_info = 0;                                                             \
     crash_info++;                                                                \
   }                                                                              \
   crash_info = (volatile uint32_t *)CRASH_INFO_BASE_ADDRESS;                     \
   /* Store Crash Signature */                                                    \
   *crash_info = ASSERT_SIGNATURE;                                                \
   crash_info+=2;                                                                 \
   /* Store R0 register */                                                        \
   __ASM volatile ("MOV %0, R0" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info--;                                                                  \
   /* Store SP register */                                                        \
   __ASM volatile ("MOV %0, SP" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info+=2;                                                                 \
   /* Store R1 register */                                                        \
   __ASM volatile ("MOV %0, R1" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store R2 register */                                                        \
   __ASM volatile ("MOV %0, R2" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store R3 register */                                                        \
   __ASM volatile ("MOV %0, R3" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store R12 register */                                                       \
   __ASM volatile ("MOV %0, R12" : "=r" (reg_content) );                          \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store LR register */                                                        \
   __ASM volatile ("MOV %0, LR" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store PC register */                                                        \
   __ASM volatile ("MOV %0, PC" : "=r" (reg_content) );                           \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
   /* Store xPSR register */                                                      \
   __ASM volatile ("MRS %0, PSR" : "=r" (reg_content) );                          \
   *crash_info = reg_content;                                                     \
   crash_info++;                                                                  \
  }                                                                               \
}                                                                                 \

#endif // __CRASH_HANDLER_H_
