/**
  ******************************************************************************
  * @file    system_bluenrg1.h
  * @author  VMA Application Team
  * @version V1.0.0
  * @date    31-June-2015
  * @brief   This file contains all the functions prototypes for the CRMU firmware 
  *          library.
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
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SYSTEM_BLUENRG1_H
#define SYSTEM_BLUENRG1_H

#ifdef __cplusplus
 extern "C" {
#endif 
	 
/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup system_bluenrg1 system bluenrg1
  * @{
  */  

#include "compiler.h"
#include "hal_types.h"
#include <core_cm0.h>


/** @addtogroup system_bluenrg1_device_configuration Device Configuration Constants
  * @{
  */

/**
  * @brief High Speed crystal 32 MHz
  */ 
#define HS_SPEED_XTAL_32MHZ       1
   
/**
  * @brief High Speed crystal 16 MHz
  */ 
#define HS_SPEED_XTAL_16MHZ       2

/**
  * @brief High Speed Internal RO
  * Not useful when radio operation are needed
  * or in any case when accurate ref clock is needed.
  */ 
#define HS_SPEED_XTAL_INTERNAL_RO 3
   

/**
  * @brief Low Speed Internal RO
  */
#define LS_SOURCE_INTERNAL_RO     1

/**
 * @brief Low Speed External 32 KHz
 */
#define LS_SOURCE_EXTERNAL_32KHZ  2

/**
 * @brief SMPS Inductor 10 uH
 */
#define SMPS_INDUCTOR_10uH        1

/**
 * @brief SMPS Inductor 4.7 uH
 */
#define SMPS_INDUCTOR_4_7uH       2

/**
 * @brief SMPS Inductor None
 */
#define SMPS_INDUCTOR_NONE        3

 /**
  * @brief High Speed Crystal default configuration 
  */
#ifndef HS_SPEED_XTAL
#define HS_SPEED_XTAL HS_SPEED_XTAL_16MHZ
#endif

/**
 * @brief  Low Speed Crystal default source 
 */
#ifndef LS_SOURCE
#define LS_SOURCE LS_SOURCE_EXTERNAL_32KHZ
#endif

/** 
 * @brief SMPS default configuration 
 */
#ifndef SMPS_INDUCTOR
#define SMPS_INDUCTOR SMPS_INDUCTOR_10uH
#endif

   
/**
  * @}
  */

/** @addtogroup system_bluenrg1_Exported_Constants Exported Constants
  * @{
  */


/**
 * @brief RAM base address
 */   
#define _MEMORY_RAM_BEGIN_    0x20000000
#define _MEMORY_RAM_SIZE_     0x6000           /* 24KB  */
#define _MEMORY_RAM_END_      0x20005FFF
   
/**
 * @brief User FLASH base address
 */
#define _MEMORY_FLASH_BEGIN_  0x10040000
#define _MEMORY_FLASH_SIZE_   0x28000          /* 160KB */
#define _MEMORY_FLASH_END_    0x10067FFF

/**
 * @brief ROM base address
 */   
#define _MEMORY_ROM_BEGIN_    0x10000000
#define _MEMORY_ROM_SIZE_     0x800             /* 2KB */
#define _MEMORY_ROM_END_      0x100007FF


   
/**
  * @}
  */

/** @addtogroup system_bluenrg1_Exported_Macros Exported Macros
  * @{
  */



#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))


/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */
  
  
/**
  * @}
  */
  
/** @addtogroup system_bluenrg1_Exported_Typedefs Exported Typedefs
  * @{
  */
  
typedef void( *intfunc )( void );
typedef union { intfunc __fun; void * __ptr; } intvec_elem;

/**
  * @}
  */

/** @addtogroup system_bluenrg1_Exported_Functions Exported Functions
  * @{
  */
  
/* Important note: The __low_level_init() function is critical for waking up from 
   deep sleep and it should not use more that 10 stack positions
   otherwise a stack corruption will occur when waking up from deep sleep.
   For this reason we are saving and restoring the first 10 words of the stack that 
   will be corrupted during the wake-up procedure from deep sleep.
   If the __low_level_init() will be modified, this define shall be modifed according
   the new function implementation
*/
#define CSTACK_PREAMBLE_NUMBER 10


void SystemInit(void);
void DeviceConfiguration(BOOL coldStart, BOOL waitLS_Ready);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_BLUENRG1_H */

/**
  * @}
  */
  
/**
  * @}
  */
