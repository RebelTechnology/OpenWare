/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#include "hardware.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_BLUE_Pin GPIO_PIN_2
#define LED_BLUE_GPIO_Port GPIOE
#define BLE_RESET_Pin GPIO_PIN_0
#define BLE_RESET_GPIO_Port GPIOC
#define BLE_NCS_Pin GPIO_PIN_1
#define BLE_NCS_GPIO_Port GPIOC
#define BLE_SPI_MISO_Pin GPIO_PIN_2
#define BLE_SPI_MISO_GPIO_Port GPIOC
#define BLE_SPI_MOSI_Pin GPIO_PIN_3
#define BLE_SPI_MOSI_GPIO_Port GPIOC
#define ADC1_N_Pin GPIO_PIN_1
#define ADC1_N_GPIO_Port GPIOA
#define ADC1_P_Pin GPIO_PIN_2
#define ADC1_P_GPIO_Port GPIOA
#define ADC_DRDY_Pin GPIO_PIN_4
#define ADC_DRDY_GPIO_Port GPIOA
#define ADC_DRDY_EXTI_IRQn EXTI4_IRQn
#define ADC_SPI_CLK_Pin GPIO_PIN_5
#define ADC_SPI_CLK_GPIO_Port GPIOA
#define ADC_START_Pin GPIO_PIN_6
#define ADC_START_GPIO_Port GPIOA
#define ADC_SPI_MOSI_Pin GPIO_PIN_7
#define ADC_SPI_MOSI_GPIO_Port GPIOA
#define ADC_NCS_Pin GPIO_PIN_4
#define ADC_NCS_GPIO_Port GPIOC
#define ADC_RESET_Pin GPIO_PIN_5
#define ADC_RESET_GPIO_Port GPIOC
#define ADC2_P_Pin GPIO_PIN_0
#define ADC2_P_GPIO_Port GPIOB
#define ADC2_N_Pin GPIO_PIN_1
#define ADC2_N_GPIO_Port GPIOB
#define RGB_3_Pin GPIO_PIN_9
#define RGB_3_GPIO_Port GPIOE
#define RGB_2_Pin GPIO_PIN_11
#define RGB_2_GPIO_Port GPIOE
#define RGB_1_Pin GPIO_PIN_13
#define RGB_1_GPIO_Port GPIOE
#define BLE_SPI_CLK_Pin GPIO_PIN_10
#define BLE_SPI_CLK_GPIO_Port GPIOB
#define USB_DM_Pin GPIO_PIN_14
#define USB_DM_GPIO_Port GPIOB
#define USB_DP_Pin GPIO_PIN_15
#define USB_DP_GPIO_Port GPIOB
#define BLE_UART_TX_Pin GPIO_PIN_9
#define BLE_UART_TX_GPIO_Port GPIOA
#define BLE_UART_RX_Pin GPIO_PIN_10
#define BLE_UART_RX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWDCLK_Pin GPIO_PIN_14
#define SWDCLK_GPIO_Port GPIOA
#define ACC_SPI_SCK_Pin GPIO_PIN_10
#define ACC_SPI_SCK_GPIO_Port GPIOC
#define ACC_SPI_MISO_Pin GPIO_PIN_11
#define ACC_SPI_MISO_GPIO_Port GPIOC
#define ACC_SPI_MOSI_Pin GPIO_PIN_12
#define ACC_SPI_MOSI_GPIO_Port GPIOC
#define ACC_NCS_Pin GPIO_PIN_0
#define ACC_NCS_GPIO_Port GPIOD
#define ACC_INT1_Pin GPIO_PIN_1
#define ACC_INT1_GPIO_Port GPIOD
#define ACC_INT1_EXTI_IRQn EXTI1_IRQn
#define ACC_INT2_Pin GPIO_PIN_2
#define ACC_INT2_GPIO_Port GPIOD
#define ADC_SPI_MISO_Pin GPIO_PIN_4
#define ADC_SPI_MISO_GPIO_Port GPIOB
#define GROVE_I2C_SCL_Pin GPIO_PIN_6
#define GROVE_I2C_SCL_GPIO_Port GPIOB
#define GROVE_I2C_SDA_Pin GPIO_PIN_7
#define GROVE_I2C_SDA_GPIO_Port GPIOB
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
