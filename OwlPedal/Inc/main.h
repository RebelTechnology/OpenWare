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
  * Copyright (c) 2018 STMicroelectronics International N.V. 
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* missing include */
#include "stm32f4xx_ll_fsmc.h"

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
#define SW1_Pin GPIO_PIN_2
#define SW1_GPIO_Port GPIOE
#define SW1_EXTI_IRQn EXTI2_IRQn
#define LED_RED_Pin GPIO_PIN_3
#define LED_RED_GPIO_Port GPIOE
#define FOOTSWITCH_Pin GPIO_PIN_4
#define FOOTSWITCH_GPIO_Port GPIOE
#define FOOTSWITCH_EXTI_IRQn EXTI4_IRQn
#define LED_GREEN_Pin GPIO_PIN_5
#define LED_GREEN_GPIO_Port GPIOE
#define ADC1_Pin GPIO_PIN_0
#define ADC1_GPIO_Port GPIOC
#define ADC2_Pin GPIO_PIN_1
#define ADC2_GPIO_Port GPIOC
#define ADC3_Pin GPIO_PIN_2
#define ADC3_GPIO_Port GPIOC
#define ADC4_Pin GPIO_PIN_3
#define ADC4_GPIO_Port GPIOC
#define EXP1_RING_Pin GPIO_PIN_2
#define EXP1_RING_GPIO_Port GPIOA
#define EXP1_TIP_Pin GPIO_PIN_3
#define EXP1_TIP_GPIO_Port GPIOA
#define FLASH_nCS_Pin GPIO_PIN_5
#define FLASH_nCS_GPIO_Port GPIOC
#define I2C_SCK_Pin GPIO_PIN_10
#define I2C_SCK_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define I2S_LRCK_Pin GPIO_PIN_12
#define I2S_LRCK_GPIO_Port GPIOB
#define I2S_SCLK_Pin GPIO_PIN_13
#define I2S_SCLK_GPIO_Port GPIOB
#define I2S_SDI_Pin GPIO_PIN_14
#define I2S_SDI_GPIO_Port GPIOB
#define I2S_SDO_Pin GPIO_PIN_15
#define I2S_SDO_GPIO_Port GPIOB
#define I2S_MCLK_Pin GPIO_PIN_6
#define I2S_MCLK_GPIO_Port GPIOC
#define PUSH_GATE_IN_Pin GPIO_PIN_6
#define PUSH_GATE_IN_GPIO_Port GPIOB
#define PUSH_GATE_IN_EXTI_IRQn EXTI9_5_IRQn
#define PUSH_GATE_OUT_Pin GPIO_PIN_7
#define PUSH_GATE_OUT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define CS_CS_Pin GPIO_PIN_10
#define CS_CS_GPIO_Port GPIOB
#define CS_RST_Pin GPIO_PIN_11
#define CS_RST_GPIO_Port GPIOB

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
