/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
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
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

/* USER CODE BEGIN 0 */
#include "MagusEncoder.h"
extern uint8_t bSwitch_ENC[7];
extern uint8_t rgENC_Data[13];
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;

/******************************************************************************/
/*            Cortex-M0 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles SPI1 global interrupt.
*/
void SPI1_IRQHandler(void)
{
  /* USER CODE BEGIN SPI1_IRQn 0 */
	
  /* USER CODE END SPI1_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi1);
  /* USER CODE BEGIN SPI1_IRQn 1 */

  /* USER CODE END SPI1_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void EXTI0_1_IRQHandler(void)
{
	// ENC5 Switch (INT0)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC5_SW_Pin))
	{
		bSwitch_ENC[5] = 1-HAL_GPIO_ReadPin(ENC5_SW_GPIO_Port, ENC5_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC5_SW_Pin);
	}
}

void EXTI2_3_IRQHandler(void)
{
	// ENC2 Switch (INT2)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC2_SW_Pin))
	{
		bSwitch_ENC[2] = 1-HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC2_SW_Pin);
	}
	
	// ENC4 Switch (INT3)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC4_SW_Pin))
	{
		bSwitch_ENC[4] = 1-HAL_GPIO_ReadPin(ENC4_SW_GPIO_Port, ENC4_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC4_SW_Pin);
	}
}

void EXTI4_15_IRQHandler(void)
{
	// SPI NSS Pin
	if (__HAL_GPIO_EXTI_GET_FLAG(SPI_NCS_Pin))
	{
		send_SPI();
		__HAL_GPIO_EXTI_CLEAR_IT(SPI_NCS_Pin);
	}
	
	// ENC1 Switch (INT10)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC1_SW_Pin))
	{
		bSwitch_ENC[1] = 1-HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC1_SW_Pin);
	}
	
	// ENC3 Switch (INT15)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC3_SW_Pin))
	{
		bSwitch_ENC[3] = 1-HAL_GPIO_ReadPin(ENC3_SW_GPIO_Port, ENC3_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC3_SW_Pin);
	}
	
	// ENC6 Switch (INT7)
	if (__HAL_GPIO_EXTI_GET_FLAG(ENC6_SW_Pin))
	{
		bSwitch_ENC[6] = 1-HAL_GPIO_ReadPin(ENC6_SW_GPIO_Port, ENC6_SW_Pin);
		rgENC_Data[0] = bSwitch_ENC[1]<<0 | bSwitch_ENC[2]<<1 | bSwitch_ENC[3]<<2 | bSwitch_ENC[4]<<3 | bSwitch_ENC[5]<<4 | bSwitch_ENC[6]<<5;
		rgENC_Data[1] = 0;
		HAL_GPIO_WritePin(CHANGE_RDY_GPIO_Port, CHANGE_RDY_Pin, GPIO_PIN_SET);
		__HAL_GPIO_EXTI_CLEAR_IT(ENC6_SW_Pin);
	}
}


/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
