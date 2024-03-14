/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CS_SCK_Pin GPIO_PIN_2
#define CS_SCK_GPIO_Port GPIOE
#define CS_SDOUT_Pin GPIO_PIN_3
#define CS_SDOUT_GPIO_Port GPIOE
#define CS_LRCK_Pin GPIO_PIN_4
#define CS_LRCK_GPIO_Port GPIOE
#define CS_SCKE5_Pin GPIO_PIN_5
#define CS_SCKE5_GPIO_Port GPIOE
#define CS_SDA_Pin GPIO_PIN_6
#define CS_SDA_GPIO_Port GPIOE
#define RANDOM_AMOUNT_SWITCH1_Pin GPIO_PIN_13
#define RANDOM_AMOUNT_SWITCH1_GPIO_Port GPIOC
#define RANDOM_AMOUNT_SWITCH2_Pin GPIO_PIN_14
#define RANDOM_AMOUNT_SWITCH2_GPIO_Port GPIOC
#define EXTSPI_nCS_Pin GPIO_PIN_15
#define EXTSPI_nCS_GPIO_Port GPIOC
#define LOOPER_START_CV_Pin GPIO_PIN_7
#define LOOPER_START_CV_GPIO_Port GPIOF
#define LOOPER_LENGTH_CV_Pin GPIO_PIN_8
#define LOOPER_LENGTH_CV_GPIO_Port GPIOF
#define LOOPER_SPEED_CV_Pin GPIO_PIN_9
#define LOOPER_SPEED_CV_GPIO_Port GPIOF
#define DELAY_TIME_CV_Pin GPIO_PIN_10
#define DELAY_TIME_CV_GPIO_Port GPIOF
#define OSC1_Pin GPIO_PIN_0
#define OSC1_GPIO_Port GPIOH
#define OSC2_Pin GPIO_PIN_1
#define OSC2_GPIO_Port GPIOH
#define RESONATOR_HARMONY_CV_Pin GPIO_PIN_1
#define RESONATOR_HARMONY_CV_GPIO_Port GPIOC
#define FILTER_CUTOFF_CV_Pin GPIO_PIN_2
#define FILTER_CUTOFF_CV_GPIO_Port GPIOC
#define SSWT_VOL_CV_Pin GPIO_PIN_3
#define SSWT_VOL_CV_GPIO_Port GPIOC
#define MUX1_Pin GPIO_PIN_0
#define MUX1_GPIO_Port GPIOA
#define MUX2_Pin GPIO_PIN_1
#define MUX2_GPIO_Port GPIOA
#define REVERB_TONESIZE_CV_Pin GPIO_PIN_2
#define REVERB_TONESIZE_CV_GPIO_Port GPIOA
#define OSC_VOCT_CV_Pin GPIO_PIN_3
#define OSC_VOCT_CV_GPIO_Port GPIOA
#define INLEVELGREEN_LED_Pin GPIO_PIN_4
#define INLEVELGREEN_LED_GPIO_Port GPIOA
#define MOD_LED_Pin GPIO_PIN_5
#define MOD_LED_GPIO_Port GPIOA
#define FLASH_MISO_Pin GPIO_PIN_6
#define FLASH_MISO_GPIO_Port GPIOA
#define FLASH_MOSI_Pin GPIO_PIN_7
#define FLASH_MOSI_GPIO_Port GPIOA
#define FLASH_HOLD_Pin GPIO_PIN_4
#define FLASH_HOLD_GPIO_Port GPIOC
#define FLASH_nCS_Pin GPIO_PIN_5
#define FLASH_nCS_GPIO_Port GPIOC
#define FLASH_WP_Pin GPIO_PIN_0
#define FLASH_WP_GPIO_Port GPIOB
#define MUX3_Pin GPIO_PIN_1
#define MUX3_GPIO_Port GPIOB
#define CU_DOWN_LED_Pin GPIO_PIN_2
#define CU_DOWN_LED_GPIO_Port GPIOB
#define CS_CS_Pin GPIO_PIN_10
#define CS_CS_GPIO_Port GPIOB
#define CS_RST_Pin GPIO_PIN_11
#define CS_RST_GPIO_Port GPIOB
#define USB_HOST_PWR_EN_Pin GPIO_PIN_12
#define USB_HOST_PWR_EN_GPIO_Port GPIOB
#define USB_HOST_PWR_FAULT_Pin GPIO_PIN_13
#define USB_HOST_PWR_FAULT_GPIO_Port GPIOB
#define RECORD_GATE_Pin GPIO_PIN_11
#define RECORD_GATE_GPIO_Port GPIOD
#define RECORD_GATE_EXTI_IRQn EXTI15_10_IRQn
#define RECORD_BUTTON_Pin GPIO_PIN_12
#define RECORD_BUTTON_GPIO_Port GPIOD
#define RECORD_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define SYNC_GATE_Pin GPIO_PIN_13
#define SYNC_GATE_GPIO_Port GPIOD
#define SYNC_GATE_EXTI_IRQn EXTI15_10_IRQn
#define PREPOST_SWITCH_Pin GPIO_PIN_8
#define PREPOST_SWITCH_GPIO_Port GPIOC
#define PREPOST_SWITCH_EXTI_IRQn EXTI9_5_IRQn
#define RECORD_BUTTON_LED_Pin GPIO_PIN_9
#define RECORD_BUTTON_LED_GPIO_Port GPIOC
#define RANDOM_BUTTON_LED_Pin GPIO_PIN_8
#define RANDOM_BUTTON_LED_GPIO_Port GPIOA
#define INLEVELRED_LED_Pin GPIO_PIN_15
#define INLEVELRED_LED_GPIO_Port GPIOA
#define MUX_A_Pin GPIO_PIN_10
#define MUX_A_GPIO_Port GPIOC
#define MUX_B_Pin GPIO_PIN_11
#define MUX_B_GPIO_Port GPIOC
#define CU_UP_LED_Pin GPIO_PIN_12
#define CU_UP_LED_GPIO_Port GPIOC
#define SHIFT_BUTTON_Pin GPIO_PIN_2
#define SHIFT_BUTTON_GPIO_Port GPIOD
#define CS_SDIN_Pin GPIO_PIN_6
#define CS_SDIN_GPIO_Port GPIOD
#define MOD_CV_RED_LED_Pin GPIO_PIN_7
#define MOD_CV_RED_LED_GPIO_Port GPIOD
#define MUX_C_Pin GPIO_PIN_11
#define MUX_C_GPIO_Port GPIOG
#define SSWT_SWITCH_Pin GPIO_PIN_12
#define SSWT_SWITCH_GPIO_Port GPIOG
#define RANDOM_BUTTON_Pin GPIO_PIN_13
#define RANDOM_BUTTON_GPIO_Port GPIOG
#define RANDOM_GATE_Pin GPIO_PIN_14
#define RANDOM_GATE_GPIO_Port GPIOG
#define RANDOM_GATE_EXTI_IRQn EXTI15_10_IRQn
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define SYNC_LED_Pin GPIO_PIN_4
#define SYNC_LED_GPIO_Port GPIOB
#define MOD_CV_BUTTON_Pin GPIO_PIN_7
#define MOD_CV_BUTTON_GPIO_Port GPIOB
#define SHIFT_LED_Pin GPIO_PIN_8
#define SHIFT_LED_GPIO_Port GPIOB
#define MOD_CV_GREEN_LED_Pin GPIO_PIN_9
#define MOD_CV_GREEN_LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
