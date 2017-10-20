/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
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
#ifndef __MAIN_H
#define __MAIN_H
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
#define EXTSPI_nCS_Pin GPIO_PIN_15
#define EXTSPI_nCS_GPIO_Port GPIOC
#define EXTSPI_SCK_Pin GPIO_PIN_7
#define EXTSPI_SCK_GPIO_Port GPIOF
#define EXTSPI_MISO_Pin GPIO_PIN_8
#define EXTSPI_MISO_GPIO_Port GPIOF
#define EXTSPI_MOSI_Pin GPIO_PIN_9
#define EXTSPI_MOSI_GPIO_Port GPIOF
#define ADC4_Pin GPIO_PIN_10
#define ADC4_GPIO_Port GPIOF
#define ADC3_Pin GPIO_PIN_1
#define ADC3_GPIO_Port GPIOC
#define ADC2_Pin GPIO_PIN_2
#define ADC2_GPIO_Port GPIOC
#define ADC1_Pin GPIO_PIN_3
#define ADC1_GPIO_Port GPIOC
#define PWM1_Pin GPIO_PIN_0
#define PWM1_GPIO_Port GPIOA
#define PWM2_Pin GPIO_PIN_1
#define PWM2_GPIO_Port GPIOA
#define TOGGLE_A1_Pin GPIO_PIN_3
#define TOGGLE_A1_GPIO_Port GPIOA
#define TOGGLE_A1_EXTI_IRQn EXTI3_IRQn
#define TOGGLE_A2_Pin GPIO_PIN_4
#define TOGGLE_A2_GPIO_Port GPIOA
#define TOGGLE_A2_EXTI_IRQn EXTI4_IRQn
#define FLASH_SCK_Pin GPIO_PIN_5
#define FLASH_SCK_GPIO_Port GPIOA
#define FLASH_MISO_Pin GPIO_PIN_6
#define FLASH_MISO_GPIO_Port GPIOA
#define FLASH_MOSI_Pin GPIO_PIN_7
#define FLASH_MOSI_GPIO_Port GPIOA
#define FLASH_HOLD_Pin GPIO_PIN_4
#define FLASH_HOLD_GPIO_Port GPIOC
#define FLASH_nCS_Pin GPIO_PIN_5
#define FLASH_nCS_GPIO_Port GPIOC
#define FLASH_nWP_Pin GPIO_PIN_0
#define FLASH_nWP_GPIO_Port GPIOB
#define PWM3_Pin GPIO_PIN_1
#define PWM3_GPIO_Port GPIOB
#define PUSHBUTTON_Pin GPIO_PIN_2
#define PUSHBUTTON_GPIO_Port GPIOB
#define PUSHBUTTON_EXTI_IRQn EXTI2_IRQn
#define CS_nCS_Pin GPIO_PIN_10
#define CS_nCS_GPIO_Port GPIOB
#define CS_nRST_Pin GPIO_PIN_11
#define CS_nRST_GPIO_Port GPIOB
#ifdef OWL_TESSERACT_REV4
#define TOGGLE_B1_Pin GPIO_PIN_12
#define TOGGLE_B1_GPIO_Port GPIOC
#define TOGGLE_B1_EXTI_IRQn EXTI15_10_IRQn
#else
#define TOGGLE_B1_Pin GPIO_PIN_12
#define TOGGLE_B1_GPIO_Port GPIOC
#define TOGGLE_B1_EXTI_IRQn EXTI15_10_IRQn
#endif
#define CS_SDIN_Pin GPIO_PIN_6
#define CS_SDIN_GPIO_Port GPIOD
#define TOGGLE_B2_Pin GPIO_PIN_9
#define TOGGLE_B2_GPIO_Port GPIOG
#define TOGGLE_B2_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

#define CS_CS_Pin GPIO_PIN_10
#define CS_CS_GPIO_Port GPIOB
#define CS_RST_Pin GPIO_PIN_11
#define CS_RST_GPIO_Port GPIOB

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
