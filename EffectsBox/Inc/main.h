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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

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
#define TSW2_A_Pin GPIO_PIN_13
#define TSW2_A_GPIO_Port GPIOC
#define TSW2_B_Pin GPIO_PIN_14
#define TSW2_B_GPIO_Port GPIOC
#define OLED_SCK_Pin GPIO_PIN_7
#define OLED_SCK_GPIO_Port GPIOF
#define OLED_DC_Pin GPIO_PIN_8
#define OLED_DC_GPIO_Port GPIOF
#define OLED_MOSI_Pin GPIO_PIN_9
#define OLED_MOSI_GPIO_Port GPIOF
#define ADC4_Pin GPIO_PIN_10
#define ADC4_GPIO_Port GPIOF
#define OSC1_Pin GPIO_PIN_0
#define OSC1_GPIO_Port GPIOH
#define OSC2_Pin GPIO_PIN_1
#define OSC2_GPIO_Port GPIOH
#define ADC3_Pin GPIO_PIN_1
#define ADC3_GPIO_Port GPIOC
#define ADC2_Pin GPIO_PIN_2
#define ADC2_GPIO_Port GPIOC
#define ADC1_Pin GPIO_PIN_3
#define ADC1_GPIO_Port GPIOC
#define ENC2_A_Pin GPIO_PIN_0
#define ENC2_A_GPIO_Port GPIOA
#define ENC2_B_Pin GPIO_PIN_1
#define ENC2_B_GPIO_Port GPIOA
#define SW2_LED_Pin GPIO_PIN_2
#define SW2_LED_GPIO_Port GPIOA
#define SW3_LED_Pin GPIO_PIN_3
#define SW3_LED_GPIO_Port GPIOA
#define SW1_BTN_Pin GPIO_PIN_4
#define SW1_BTN_GPIO_Port GPIOA
#define SW1_BTN_EXTI_IRQn EXTI4_IRQn
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
#define SW2_BTN_Pin GPIO_PIN_1
#define SW2_BTN_GPIO_Port GPIOB
#define SW2_BTN_EXTI_IRQn EXTI1_IRQn
#define SW4_BTN_Pin GPIO_PIN_2
#define SW4_BTN_GPIO_Port GPIOB
#define SW4_BTN_EXTI_IRQn EXTI2_IRQn
#define CS_CS_Pin GPIO_PIN_10
#define CS_CS_GPIO_Port GPIOB
#define CS_RST_Pin GPIO_PIN_11
#define CS_RST_GPIO_Port GPIOB
#define SW4_LED_Pin GPIO_PIN_11
#define SW4_LED_GPIO_Port GPIOD
#define ENC1_B_Pin GPIO_PIN_12
#define ENC1_B_GPIO_Port GPIOD
#define ENC1_A_Pin GPIO_PIN_13
#define ENC1_A_GPIO_Port GPIOD
#define SW7_LED_Pin GPIO_PIN_9
#define SW7_LED_GPIO_Port GPIOC
#define SW7_PWM_Pin GPIO_PIN_8
#define SW7_PWM_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_10
#define OLED_CS_GPIO_Port GPIOC
#define OLED_RST_Pin GPIO_PIN_11
#define OLED_RST_GPIO_Port GPIOC
#define CS_SDIN_Pin GPIO_PIN_6
#define CS_SDIN_GPIO_Port GPIOD
#define SW6_BTN_Pin GPIO_PIN_7
#define SW6_BTN_GPIO_Port GPIOD
#define SW6_BTN_EXTI_IRQn EXTI9_5_IRQn
#define SW7_BTN_Pin GPIO_PIN_9
#define SW7_BTN_GPIO_Port GPIOG
#define SW7_BTN_EXTI_IRQn EXTI9_5_IRQn
#define SW6_LED_Pin GPIO_PIN_10
#define SW6_LED_GPIO_Port GPIOG
#define SW5_BTN_Pin GPIO_PIN_12
#define SW5_BTN_GPIO_Port GPIOG
#define SW5_BTN_EXTI_IRQn EXTI15_10_IRQn
#define SW3_BTN_Pin GPIO_PIN_13
#define SW3_BTN_GPIO_Port GPIOG
#define SW3_BTN_EXTI_IRQn EXTI15_10_IRQn
#define SW5_LED_Pin GPIO_PIN_14
#define SW5_LED_GPIO_Port GPIOG
#define FLASH_SCK_Pin GPIO_PIN_3
#define FLASH_SCK_GPIO_Port GPIOB
#define TSW1_A_Pin GPIO_PIN_4
#define TSW1_A_GPIO_Port GPIOB
#define TSW1_B_Pin GPIO_PIN_7
#define TSW1_B_GPIO_Port GPIOB
#define SW1_LED_Pin GPIO_PIN_8
#define SW1_LED_GPIO_Port GPIOB
#define SW1_6_PWM_Pin GPIO_PIN_9
#define SW1_6_PWM_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void Error_Handler(void);
#define _Error_Handler(a, b) Error_Handler()
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
