/**
******************************************************************************
* @file    clock.c 
* @author  AMS - VMA RF Application Team
* @version V1.0.0
* @date    14-September-2015
* @brief   clock APIs based on SySTick
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
/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1.h"
#include "BlueNRG1_conf.h"
#include "clock.h"

/** @addtogroup BlueNRG1_StdPeriph_Examples
* @{
*/

/** @addtogroup UART Interrupt Example
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSCLK_FREQ 	16000000            /*< System clock frequency */ 
#define RELOAD_VALUE   (SYSCLK_FREQ/1000)-1 // One clock each 1 ms

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile tClockTime sys_tick_count;
const tClockTime CLOCK_SECOND = 1000;

/*---------------------------------------------------------------------------*/
void SysCount_Handler(void) 
{
  sys_tick_count++;
}

/*---------------------------------------------------------------------------*/

void Clock_Init(void)
{
  /** Configure SysTick to generate Interrupt with 1ms period */
  SysTick_Config(RELOAD_VALUE);
}

/*---------------------------------------------------------------------------*/

tClockTime Clock_Time(void)
{
  return sys_tick_count;
}

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of 1 ms.
 *
 */
void Clock_Wait(uint32_t i)
{
  tClockTime start;

  start = Clock_Time();
  while(Clock_Time() - start < (tClockTime)i);
}
/*---------------------------------------------------------------------------*/

