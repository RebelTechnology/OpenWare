/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#define SAI_MCLK_Pin GPIO_PIN_2
#define SAI_MCLK_GPIO_Port GPIOE
#define SAI_SDO_Pin GPIO_PIN_3
#define SAI_SDO_GPIO_Port GPIOE
#define SAI_LRCK_Pin GPIO_PIN_4
#define SAI_LRCK_GPIO_Port GPIOE
#define SAI_BCK_Pin GPIO_PIN_5
#define SAI_BCK_GPIO_Port GPIOE
#define SAI_SDI_Pin GPIO_PIN_6
#define SAI_SDI_GPIO_Port GPIOE
#define EXTI5_Pin GPIO_PIN_13
#define EXTI5_GPIO_Port GPIOC
#define EXTI7_Pin GPIO_PIN_14
#define EXTI7_GPIO_Port GPIOC
#define ADC5_Pin GPIO_PIN_0
#define ADC5_GPIO_Port GPIOC
#define ADC1_Pin GPIO_PIN_1
#define ADC1_GPIO_Port GPIOC
#define ADC_SPI_MISO_Pin GPIO_PIN_2
#define ADC_SPI_MISO_GPIO_Port GPIOC
#define ADC_SPI_MOSI_Pin GPIO_PIN_3
#define ADC_SPI_MOSI_GPIO_Port GPIOC
#define ADC7_Pin GPIO_PIN_2
#define ADC7_GPIO_Port GPIOA
#define ADC3_Pin GPIO_PIN_3
#define ADC3_GPIO_Port GPIOA
#define DAC1_Pin GPIO_PIN_4
#define DAC1_GPIO_Port GPIOA
#define DAC2_Pin GPIO_PIN_5
#define DAC2_GPIO_Port GPIOA
#define ADC8_Pin GPIO_PIN_6
#define ADC8_GPIO_Port GPIOA
#define ADC4_Pin GPIO_PIN_7
#define ADC4_GPIO_Port GPIOA
#define ADC6_Pin GPIO_PIN_4
#define ADC6_GPIO_Port GPIOC
#define ADC2_Pin GPIO_PIN_5
#define ADC2_GPIO_Port GPIOC
#define ADC_RESET_Pin GPIO_PIN_8
#define ADC_RESET_GPIO_Port GPIOE
#define TIM3_CH1_Pin GPIO_PIN_9
#define TIM3_CH1_GPIO_Port GPIOE
#define TIM3_CH2_Pin GPIO_PIN_11
#define TIM3_CH2_GPIO_Port GPIOE
#define SPI1_SCK_Pin GPIO_PIN_12
#define SPI1_SCK_GPIO_Port GPIOE
#define SPI1_MISO_Pin GPIO_PIN_13
#define SPI1_MISO_GPIO_Port GPIOE
#define SPI1_MOSI_Pin GPIO_PIN_14
#define SPI1_MOSI_GPIO_Port GPIOE
#define I2C_SCL_Pin GPIO_PIN_10
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_11
#define I2C_SDA_GPIO_Port GPIOB
#define USBH_ID_Pin GPIO_PIN_12
#define USBH_ID_GPIO_Port GPIOB
#define USBH_VBUS_Pin GPIO_PIN_13
#define USBH_VBUS_GPIO_Port GPIOB
#define USBH_DM_Pin GPIO_PIN_14
#define USBH_DM_GPIO_Port GPIOB
#define USBH_DP_Pin GPIO_PIN_15
#define USBH_DP_GPIO_Port GPIOB
#define UART2_TX_Pin GPIO_PIN_8
#define UART2_TX_GPIO_Port GPIOD
#define UART2_RX_Pin GPIO_PIN_9
#define UART2_RX_GPIO_Port GPIOD
#define ADC_NCS_Pin GPIO_PIN_11
#define ADC_NCS_GPIO_Port GPIOD
#define TIM6_CH1_Pin GPIO_PIN_6
#define TIM6_CH1_GPIO_Port GPIOC
#define TIM6_CH2_Pin GPIO_PIN_7
#define TIM6_CH2_GPIO_Port GPIOC
#define USBD_VBUS_Pin GPIO_PIN_9
#define USBD_VBUS_GPIO_Port GPIOA
#define USBD_ID_Pin GPIO_PIN_10
#define USBD_ID_GPIO_Port GPIOA
#define USBD_DM_Pin GPIO_PIN_11
#define USBD_DM_GPIO_Port GPIOA
#define USBD_DP_Pin GPIO_PIN_12
#define USBD_DP_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SPI2_SCK_Pin GPIO_PIN_10
#define SPI2_SCK_GPIO_Port GPIOC
#define SPI2_MISO_Pin GPIO_PIN_11
#define SPI2_MISO_GPIO_Port GPIOC
#define SPI2_MOSI_Pin GPIO_PIN_12
#define SPI2_MOSI_GPIO_Port GPIOC
#define EXTI4_Pin GPIO_PIN_0
#define EXTI4_GPIO_Port GPIOD
#define EXTI8_Pin GPIO_PIN_1
#define EXTI8_GPIO_Port GPIOD
#define EXTI3_Pin GPIO_PIN_2
#define EXTI3_GPIO_Port GPIOD
#define ADC_SPI_CLK_Pin GPIO_PIN_3
#define ADC_SPI_CLK_GPIO_Port GPIOD
#define EXTI2_Pin GPIO_PIN_4
#define EXTI2_GPIO_Port GPIOD
#define UART1_TX_Pin GPIO_PIN_5
#define UART1_TX_GPIO_Port GPIOD
#define UART1_RX_Pin GPIO_PIN_6
#define UART1_RX_GPIO_Port GPIOD
#define EXTI6_Pin GPIO_PIN_7
#define EXTI6_GPIO_Port GPIOD
#define TIM1_CH1_Pin GPIO_PIN_4
#define TIM1_CH1_GPIO_Port GPIOB
#define TIM1_CH2_Pin GPIO_PIN_5
#define TIM1_CH2_GPIO_Port GPIOB
#define EXTI1_Pin GPIO_PIN_6
#define EXTI1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
