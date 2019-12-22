/**
  ******************************************************************************
  * @file    gp_timer.h
  * @author  AMS - VMA RF Application team
  * @version V1.0.0
  * @date    21-Sept-2015
  * @brief   Header file for general purpose timer library.
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
#ifndef __GP_TIMER_H__
#define __GP_TIMER_H__

#include "clock.h"

/**
 * @brief A structure that represents a timer. Use Timer_Set() to set the timer.
 *
 */
struct timer {
    
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    
  tClockTime start;
  tClockTime interval;
  
#endif
};

/** 
 * @brief This function sets a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 * 
 * @param[in] t         Pointer to a timer structure
 * @param[in] interval  The interval before the timer expires.
 *
 * @retval None
 */
void Timer_Set(struct timer *t, tClockTime interval);

/** 
 * @brief This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_restart() function.
 * 
 * @param[in] t Pointer to a timer structure
 *
 * @retval None
 */
void Timer_Reset(struct timer *t);

/** 
 * @brief This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time. A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 * 
 * @param[in] t Pointer to a timer structure
 *
 * @retval None
 */
void Timer_Restart(struct timer *t);

/** 
 * @brief This function tests if a timer has expired and returns true or
 * false depending on its status.
 * 
 * @param[in] t Pointer to a timer structure
 *
 * @retval Non-zero if the timer has expired, zero otherwise.
 */
int Timer_Expired(struct timer *t);

/** 
 * @brief This function returns the time until the timer expires.
 * 
 * @param[in] t Pointer to a timer structure
 *
 * @retval The time until the timer expires
 */
tClockTime Timer_Remaining(struct timer *t);

#endif /* __GP_TIMER_H__ */
