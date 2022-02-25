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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define FLASH_DQ2_Pin GPIO_PIN_2
#define FLASH_DQ2_GPIO_Port GPIOE
#define BUT_A_LED_Pin GPIO_PIN_12
#define BUT_A_LED_GPIO_Port GPIOG
#define BUT_B_LED_Pin GPIO_PIN_11
#define BUT_B_LED_GPIO_Port GPIOG
#define BUT_B_Pin GPIO_PIN_10
#define BUT_B_GPIO_Port GPIOG
#define BUT_B_EXTI_IRQn EXTI15_10_IRQn
#define BUT_A_Pin GPIO_PIN_2
#define BUT_A_GPIO_Port GPIOD
#define BUT_A_EXTI_IRQn EXTI2_IRQn
#define FLASH_NCS_Pin GPIO_PIN_6
#define FLASH_NCS_GPIO_Port GPIOG
#define FLASH_DQ3_Pin GPIO_PIN_6
#define FLASH_DQ3_GPIO_Port GPIOF
#define ADC_RST_Pin GPIO_PIN_3
#define ADC_RST_GPIO_Port GPIOG
#define FLASH_CLK_Pin GPIO_PIN_10
#define FLASH_CLK_GPIO_Port GPIOF
#define FLASH_DQ1_Pin GPIO_PIN_9
#define FLASH_DQ1_GPIO_Port GPIOF
#define FLASH_DQ0_Pin GPIO_PIN_8
#define FLASH_DQ0_GPIO_Port GPIOF
#define ADC1_Pin GPIO_PIN_0
#define ADC1_GPIO_Port GPIOC
#define ADC5_Pin GPIO_PIN_1
#define ADC5_GPIO_Port GPIOC
#define ADC_SPI_MISO_Pin GPIO_PIN_2
#define ADC_SPI_MISO_GPIO_Port GPIOC
#define ADC_SPI_MOSI_Pin GPIO_PIN_3
#define ADC_SPI_MOSI_GPIO_Port GPIOC
#define ADC8_Pin GPIO_PIN_4
#define ADC8_GPIO_Port GPIOC
#define ADC2_Pin GPIO_PIN_2
#define ADC2_GPIO_Port GPIOA
#define ADC6_Pin GPIO_PIN_6
#define ADC6_GPIO_Port GPIOA
#define ADC4_Pin GPIO_PIN_5
#define ADC4_GPIO_Port GPIOC
#define ADC3_Pin GPIO_PIN_1
#define ADC3_GPIO_Port GPIOB
#define ADC7_Pin GPIO_PIN_0
#define ADC7_GPIO_Port GPIOB
#define ADC_SPI_CLK_Pin GPIO_PIN_10
#define ADC_SPI_CLK_GPIO_Port GPIOB
#define ADC_SPI_NCS_Pin GPIO_PIN_11
#define ADC_SPI_NCS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define OLED_DC_Pin GPIO_PIN_11
#define OLED_DC_GPIO_Port GPIOC

#define ADC_NCS_Pin ADC_SPI_NCS_Pin
#define ADC_NCS_GPIO_Port ADC_SPI_NCS_GPIO_Port
#define ADC_RESET_Pin ADC_RST_Pin
#define ADC_RESET_GPIO_Port ADC_RST_GPIO_Port

#define FLASH_nCS_Pin FLASH_NCS_Pin
#define FLASH_nCS_GPIO_Port FLASH_NCS_GPIO_Port
#define FLASH_WP_Pin FLASH_DQ2_Pin
#define FLASH_WP_GPIO_Port FLASH_DQ2_GPIO_Port
#define FLASH_HOLD_Pin FLASH_DQ3_Pin
#define FLASH_HOLD_GPIO_Port FLASH_DQ3_GPIO_Port

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
