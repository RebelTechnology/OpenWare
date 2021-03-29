/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define ENC_SW_Pin GPIO_PIN_13
#define ENC_SW_GPIO_Port GPIOC
#define MAX_CS_Pin GPIO_PIN_0
#define MAX_CS_GPIO_Port GPIOC
#define OLED_CS_Pin GPIO_PIN_6
#define OLED_CS_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_0
#define OLED_DC_GPIO_Port GPIOB
#define OLED_RST_Pin GPIO_PIN_9
#define OLED_RST_GPIO_Port GPIOC
#define USBH_PWR_FAULT_Pin GPIO_PIN_9
#define USBH_PWR_FAULT_GPIO_Port GPIOA
#define USBH_PWR_EN_Pin GPIO_PIN_10
#define USBH_PWR_EN_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

#define PIXI_nCS_GPIO_Port MAX_CS_GPIO_Port
#define PIXI_nCS_Pin MAX_CS_Pin
  
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
