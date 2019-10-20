/**
  ******************************************************************************
  * @file    BlueNRG1_it.c  
  * @author  Central Labs
  * @version V 1.0.0
  * @date    May-2017
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
/* #include "steval_bluemic1.h" */
/* #include "steval_bluemic1_audio_in.h" */
#include "peripheral_mngr_app.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint32_t lSystickCounter;
extern uint16_t usiTimerVal;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SVCall exception.
  */
void SVC_Handler(void)
{
}


/**
  * @brief  This function handles SysTick Handler.
  */
void SysTick_Handler(void)
{
  usiTimerVal++; // used by outgoing MIDI messages
  lSystickCounter++;
}


/******************************************************************************/
/*                 BlueNRG1LP Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_BlueNRG1lp.s).                                               */
/******************************************************************************/


/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  */
void DMA_Handler(void)
{
  /* Check DMA Half Transfer Complete interrupt */
  if(DMA_GetFlagStatus(DMA_FLAG_HT0)) {      
    DMA_ClearFlag(DMA_FLAG_HT0);
    /* HT_IT_Callback(); */
  }
    
  /* Check DMA Transfer Complete interrupt */
  if(DMA_GetFlagStatus(DMA_FLAG_TC0)) {      
    DMA_ClearFlag(DMA_FLAG_TC0);
    /* TC_IT_Callback(); */
  }
}

/**
  * @brief  This function handles GPIO interrupt request.
  * @param  None
  * @retval None
  */
void GPIO_Handler(void)
{
  /* If BUTTON_1 is pressed */
  /* if(BSP_PB_GetITPendingBit(BUTTON_USER) == SET) */
  /* { */
  /*   BSP_PB_ClearITPendingBit(BUTTON_USER); */
  /*   if(APP_PER_state==APP_STATUS_CONNECTED) */
  /*   {    */
  /*     BV_APP_StartStop_ctrl(); */
  /*   } */
  /* } */
}

/**
* @brief  This function handles MFT1B interrupt request.
* @param  None
* @retval None
*/
void MFT1B_Handler(void)
{
  if ( MFT_StatusIT(MFT1,MFT_IT_TND) != RESET )
  {    
    /* Set the counter at 30 ms */
    MFT_SetCounter2(MFT1, 6000);
    
    /** Clear MFT11 pending interrupt */
    MFT_ClearIT(MFT1, MFT_IT_TND);
  }
}

void Blue_Handler(void)
{
  // Call RAL_Isr
  RAL_Isr();
}
 

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
