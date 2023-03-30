/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#define RANDOMAMOUNTSWITCH_Pin GPIO_PIN_13
#define RANDOMAMOUNTSWITCH_GPIO_Port GPIOC
#define EXTSPI_nCS_Pin GPIO_PIN_15
#define EXTSPI_nCS_GPIO_Port GPIOC
#define OSC1_Pin GPIO_PIN_0
#define OSC1_GPIO_Port GPIOH
#define OSC2_Pin GPIO_PIN_1
#define OSC2_GPIO_Port GPIOH
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
#define MUX_C_Pin GPIO_PIN_11
#define MUX_C_GPIO_Port GPIOG
#define WTSWITCH_Pin GPIO_PIN_12
#define WTSWITCH_GPIO_Port GPIOG
#define RANDOMGATEIN_Pin GPIO_PIN_13
#define RANDOMGATEIN_GPIO_Port GPIOG
#define RANDOMBUTTON_Pin GPIO_PIN_14
#define RANDOMBUTTON_GPIO_Port GPIOG
#define RANDOMBUTTON_EXTI_IRQn EXTI15_10_IRQn
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define SYNCLED_Pin GPIO_PIN_4
#define SYNCLED_GPIO_Port GPIOB
#define FILTERMODESWITCH_Pin GPIO_PIN_7
#define FILTERMODESWITCH_GPIO_Port GPIOB
#define FILTERMODESWITCH_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
