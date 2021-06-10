/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#define Flash_CS_Pin GPIO_PIN_6
#define Flash_CS_GPIO_Port GPIOF
#define Flash_CLK_Pin GPIO_PIN_7
#define Flash_CLK_GPIO_Port GPIOF
#define Flash_MISO_Pin GPIO_PIN_8
#define Flash_MISO_GPIO_Port GPIOF
#define Flash_MOSI_Pin GPIO_PIN_9
#define Flash_MOSI_GPIO_Port GPIOF
#define OLED_MOSI_Pin GPIO_PIN_1
#define OLED_MOSI_GPIO_Port GPIOC
#define OLED_RST_Pin GPIO_PIN_2
#define OLED_RST_GPIO_Port GPIOC
#define OLED_DC_Pin GPIO_PIN_3
#define OLED_DC_GPIO_Port GPIOC
#define ADC0_Pin GPIO_PIN_0
#define ADC0_GPIO_Port GPIOA
#define ADC1_Pin GPIO_PIN_1
#define ADC1_GPIO_Port GPIOA
#define ENC2_CH1_Pin GPIO_PIN_6
#define ENC2_CH1_GPIO_Port GPIOA
#define ENC2_CH2_Pin GPIO_PIN_7
#define ENC2_CH2_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_10
#define OLED_SCK_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_11
#define OLED_CS_GPIO_Port GPIOB
#define USB_HOST_PWR_EN_Pin GPIO_PIN_9
#define USB_HOST_PWR_EN_GPIO_Port GPIOC
#define USB_HOST_PWR_FAULT_Pin GPIO_PIN_10
#define USB_HOST_PWR_FAULT_GPIO_Port GPIOA
#define ENC1_CH1_Pin GPIO_PIN_15
#define ENC1_CH1_GPIO_Port GPIOA
#define SW_A_Pin GPIO_PIN_10
#define SW_A_GPIO_Port GPIOC
#define SW_A_EXTI_IRQn EXTI15_10_IRQn
#define TR_IN_A_Pin GPIO_PIN_11
#define TR_IN_A_GPIO_Port GPIOC
#define TR_IN_A_EXTI_IRQn EXTI15_10_IRQn
#define SW_B_Pin GPIO_PIN_12
#define SW_B_GPIO_Port GPIOC
#define SW_B_EXTI_IRQn EXTI15_10_IRQn
#define TR_IN_B_Pin GPIO_PIN_2
#define TR_IN_B_GPIO_Port GPIOD
#define TR_OUT_B_Pin GPIO_PIN_3
#define TR_OUT_B_GPIO_Port GPIOD
#define TR_OUT_A_Pin GPIO_PIN_4
#define TR_OUT_A_GPIO_Port GPIOD
#define CS_SDIN_Pin GPIO_PIN_6
#define CS_SDIN_GPIO_Port GPIOD
#define CS_CS_Pin GPIO_PIN_9
#define CS_CS_GPIO_Port GPIOG
#define CS_RST_Pin GPIO_PIN_10
#define CS_RST_GPIO_Port GPIOG
#define ENC1_SW_Pin GPIO_PIN_14
#define ENC1_SW_GPIO_Port GPIOG
#define ENC1_SW_EXTI_IRQn EXTI15_10_IRQn
#define ENC1_CH2_Pin GPIO_PIN_3
#define ENC1_CH2_GPIO_Port GPIOB
#define ENC2_SW_Pin GPIO_PIN_4
#define ENC2_SW_GPIO_Port GPIOB
#define ENC2_SW_EXTI_IRQn EXTI4_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
