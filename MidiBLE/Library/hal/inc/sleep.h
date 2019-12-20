/**
  ******************************************************************************
  * @file    sleep.h
  * @author  AMS - VMA RF Application team
  * @version V1.0.0
  * @date    21-Sept-2015
  * @brief   Header file for BlueNRG Sleep management
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
#ifndef __SLEEP_H__
#define __SLEEP_H__

#include "BlueNRG1.h"

/** @brief Enumerations for the possible microcontroller sleep modes.
 * - SLEEPMODE_RUNNING
 *     Everything is active and running.  In practice this mode is not
 *     used, but it is defined for completeness of information.
 * - SLEEPMODE_CPU_HALT
 *     Only the CPU is halted.  The rest of the chip continues running
 *     normally.  The chip will wake from any interrupt.
 * - SLEEPMODE_WAKETIMER
 *     The device is in deep sleep and the timer clock sources remain running. 
 *     Wakeup is possible from both GPIO and the sleep timer.
 * - SLEEPMODE_NOTIMER
 *     The sleep timer clock sources are turned off.
 *     Wakeup is possible from only GPIO.
 */
typedef enum {
  SLEEPMODE_RUNNING       = 0,
  SLEEPMODE_CPU_HALT      = 1,
  SLEEPMODE_WAKETIMER     = 2,
  SLEEPMODE_NOTIMER       = 3,
} SleepModes; 

/** @brief Wakeup source IO9 Mask */
#define WAKEUP_IO9   0x01

/** @brief Wakeup source IO10 Mask */
#define WAKEUP_IO10  0x02

/** @brief Wakeup source IO11 Mask */
#define WAKEUP_IO11  0x04

/** @brief Wakeup source IO12 Mask */
#define WAKEUP_IO12  0x08

/** @brief Wakeup source IO13 Mask */
#define WAKEUP_IO13  0x10

/** @brief Wakeup source Blue block Sleep Timer 1 */
#define WAKEUP_SLEEP_TIMER1  0x20

/** @brief Wakeup source Blue block Sleep Timer 2 */
#define WAKEUP_SLEEP_TIMER2  0x40

/** @brief The Reset reason is not caused from a wakeup source but from a BOR reset */
#define WAKEUP_BOR 0xF0

/** @brief The Reset reason is not caused from a wakeup source but from a POR reset */
#define WAKEUP_POR 0xF1

/** @brief The Reset reason is not caused from a wakeup source but from a System reset request */
#define WAKEUP_SYS_RESET_REQ 0xF2

/** @brief The Reset reason is not caused from a wakeup source but from a watchdog timer reset */
#define WAKEUP_RESET_WDG 0xF3

/** @brief The Reset reason is not caused from a known wakeup source */
#define NO_WAKEUP_RESET 0xFF;

/** @brief The system wakes up when IO is low */
#define WAKEUP_IO_LOW 0x00

/** @brief The system wakes up when IO is high */
#define WAKEUP_IO_HIGH 0x01

/** @brief Sleep Configuration Error */
#define BLUENRG_SLEEP_CONFIGURATION_ERROR 0xFE

/** @brief Number of virtual sleep timer used */
#define SLEEP_TIMER 0

/** 
 * @brief Puts the BlueNRG in power save. The device can be configured with different wake sources
 * according the sleepMode paramter. The most power save is obtained configuring the device in 
 * SLEEPMODE_NOTIMER because all the sources is turned off. When the wakeup occours, the CPU returns
 * executing the next instruction after the call BlueNRG_Sleep() resoting the context.
 * 
 * @param sleepMode Sleep mode (see SleepModes enum)
 * 
 * @param gpioWakeBitMask  A bit mask of the GPIO that are allowed to wake
 * the chip from deep sleep.  A high bit in the mask will enable waking
 * the chip if the corresponding GPIO changes state.  bit0 is IO9, bit1 is
 * IO10, bit2 is IO11, bit3 is IO12, bit4 is IO13, bits[5:7] are ignored.
 * If this param is 0 the wakeup GPIO are ignored
 * @param gpioWakeLevelMask  A mask used to setup the active wakeup level:
 * - 0: the system wakes up when IO is low
 * - 1: the system wakes up when IO is high
 * The bit mask follow the bit mask of the gpioWakeBitMask parameter.
 * 
 * @param sleep_time Deep sleep timeout. This timeout is expressed in ms.
*  The device is put in deep sleep with this timeout.
 * If this timeout is equal to zero, the power save procedure follows the timeout
 * imposed from the BlueNRG-1 stack.
 * The wakeup sources can be also the GPIOs.
 *
 * @retval Status of the call
 */
uint8_t BlueNRG_Sleep(SleepModes sleepMode, 
                      uint8_t gpioWakeBitMask, 
                      uint8_t gpioWakeLevelMask, 
                      uint32_t sleep_time);

/** 
 * @brief This function shall check if the BlueNRG Sleep mode selected is safe 
 * in according with the application point of view. 
 * This is a weak function, can be modified from the application.
 * 
 * @param sleepMode Sleep mode (see SleepModes enum)
 *
 * @retval Return the sleep mode possible and safe from the application point of view
 */
SleepModes App_SleepMode_Check(SleepModes sleepMode);

/**
 * @brief This function executes a wait for interrupt instruction and
 * suspends the core execution until one of a number of events occurs
 */ 
void BlueNRG_IdleSleep(void);

/**
 * @brief Return the wakeup source from deep sleep. Possible value are:
 * - WAKEUP_IO9
 * - WAKEUP_IO10
 * - WAKEUP_IO11
 * - WAKEUP_IO12
 * - WAKEUP_IO13
 * - WAKEUP_SLEEP_TIMER1
 * - WAKEUP_SLEEP_TIMER2
 * - WAKEUP_BOR
 * - WAKEUP_POR
 * - WAKEUP_SYS_RESET_REQ
 * - WAKEUP_RESET_WDG
 * - NO_WAKEUP_RESET
 */ 
uint8_t BlueNRG_WakeupSource(void);

#endif
