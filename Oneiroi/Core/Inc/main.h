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
#define RANDOMAMOUNTSWITCH1_Pin GPIO_PIN_13
#define RANDOMAMOUNTSWITCH1_GPIO_Port GPIOC
#define RANDOMAMOUNTSWITCH2_Pin GPIO_PIN_14
#define RANDOMAMOUNTSWITCH2_GPIO_Port GPIOC
#define EXTSPI_nCS_Pin GPIO_PIN_15
#define EXTSPI_nCS_GPIO_Port GPIOC
#define STARTCV_Pin GPIO_PIN_7
#define STARTCV_GPIO_Port GPIOF
#define LENGTHCV_Pin GPIO_PIN_8
#define LENGTHCV_GPIO_Port GPIOF
#define SPEEDCV_Pin GPIO_PIN_9
#define SPEEDCV_GPIO_Port GPIOF
#define DELAYCV_Pin GPIO_PIN_10
#define DELAYCV_GPIO_Port GPIOF
#define OSC1_Pin GPIO_PIN_0
#define OSC1_GPIO_Port GPIOH
#define OSC2_Pin GPIO_PIN_1
#define OSC2_GPIO_Port GPIOH
#define RESONATORCV_Pin GPIO_PIN_1
#define RESONATORCV_GPIO_Port GPIOC
#define FILTERCV_Pin GPIO_PIN_2
#define FILTERCV_GPIO_Port GPIOC
#define OSC2CV_Pin GPIO_PIN_3
#define OSC2CV_GPIO_Port GPIOC
#define MUX1_Pin GPIO_PIN_0
#define MUX1_GPIO_Port GPIOA
#define MUX2_Pin GPIO_PIN_1
#define MUX2_GPIO_Port GPIOA
#define REVERBCV_Pin GPIO_PIN_2
#define REVERBCV_GPIO_Port GPIOA
#define VOCTCV_Pin GPIO_PIN_3
#define VOCTCV_GPIO_Port GPIOA
#define INLEVELLEDGREEN_Pin GPIO_PIN_4
#define INLEVELLEDGREEN_GPIO_Port GPIOA
#define MODLED_Pin GPIO_PIN_5
#define MODLED_GPIO_Port GPIOA
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
#define CS_CS_Pin GPIO_PIN_10
#define CS_CS_GPIO_Port GPIOB
#define CS_RST_Pin GPIO_PIN_11
#define CS_RST_GPIO_Port GPIOB
#define USB_HOST_PWR_EN_Pin GPIO_PIN_12
#define USB_HOST_PWR_EN_GPIO_Port GPIOB
#define USB_HOST_PWR_FAULT_Pin GPIO_PIN_13
#define USB_HOST_PWR_FAULT_GPIO_Port GPIOB
#define RECORDGATEIN_Pin GPIO_PIN_11
#define RECORDGATEIN_GPIO_Port GPIOD
#define RECORDGATEIN_EXTI_IRQn EXTI15_10_IRQn
#define RECORDBUTTON_Pin GPIO_PIN_12
#define RECORDBUTTON_GPIO_Port GPIOD
#define RECORDBUTTON_EXTI_IRQn EXTI15_10_IRQn
#define SYNCIN_Pin GPIO_PIN_13
#define SYNCIN_GPIO_Port GPIOD
#define SYNCIN_EXTI_IRQn EXTI15_10_IRQn
#define PREPOSTSWITCH_Pin GPIO_PIN_8
#define PREPOSTSWITCH_GPIO_Port GPIOC
#define PREPOSTSWITCH_EXTI_IRQn EXTI9_5_IRQn
#define RECORDBUTTONLED_Pin GPIO_PIN_9
#define RECORDBUTTONLED_GPIO_Port GPIOC
#define RANDOMBUTTONLED_Pin GPIO_PIN_8
#define RANDOMBUTTONLED_GPIO_Port GPIOA
#define INLEVELREDLED_Pin GPIO_PIN_15
#define INLEVELREDLED_GPIO_Port GPIOA
#define MUX_A_Pin GPIO_PIN_10
#define MUX_A_GPIO_Port GPIOC
#define MUX_B_Pin GPIO_PIN_11
#define MUX_B_GPIO_Port GPIOC
#define CS_SDIN_Pin GPIO_PIN_6
#define CS_SDIN_GPIO_Port GPIOD
#define FILTERMODESWITCH2_Pin GPIO_PIN_7
#define FILTERMODESWITCH2_GPIO_Port GPIOD
#define MUX_C_Pin GPIO_PIN_11
#define MUX_C_GPIO_Port GPIOG
#define WTSWITCH_Pin GPIO_PIN_12
#define WTSWITCH_GPIO_Port GPIOG
#define RANDOMBUTTON_Pin GPIO_PIN_13
#define RANDOMBUTTON_GPIO_Port GPIOG
#define RANDOMGATEIN_Pin GPIO_PIN_14
#define RANDOMGATEIN_GPIO_Port GPIOG
#define RANDOMGATEIN_EXTI_IRQn EXTI15_10_IRQn
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define SYNCLED_Pin GPIO_PIN_4
#define SYNCLED_GPIO_Port GPIOB
#define FILTERMODESWITCH1_Pin GPIO_PIN_7
#define FILTERMODESWITCH1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
