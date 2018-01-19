/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define ENC6_A_Pin GPIO_PIN_0
#define ENC6_A_GPIO_Port GPIOF
#define ENC6_B_Pin GPIO_PIN_1
#define ENC6_B_GPIO_Port GPIOF
#define ENC2_A_Pin GPIO_PIN_0
#define ENC2_A_GPIO_Port GPIOA
#define ENC2_B_Pin GPIO_PIN_1
#define ENC2_B_GPIO_Port GPIOA
#define ENC2_SW_Pin GPIO_PIN_2
#define ENC2_SW_GPIO_Port GPIOA
#define ENC2_SW_EXTI_IRQn EXTI2_3_IRQn
#define CHANGE_RDY_Pin GPIO_PIN_3
#define CHANGE_RDY_GPIO_Port GPIOA
#define SPI_NCS_Pin GPIO_PIN_4
#define SPI_NCS_GPIO_Port GPIOA
#define SPI_NCS_EXTI_IRQn EXTI4_15_IRQn
#define ENC4_B_Pin GPIO_PIN_7
#define ENC4_B_GPIO_Port GPIOA
#define ENC5_SW_Pin GPIO_PIN_0
#define ENC5_SW_GPIO_Port GPIOB
#define ENC5_SW_EXTI_IRQn EXTI0_1_IRQn
#define ENC1_A_Pin GPIO_PIN_8
#define ENC1_A_GPIO_Port GPIOA
#define ENC1_B_Pin GPIO_PIN_9
#define ENC1_B_GPIO_Port GPIOA
#define ENC1_SW_Pin GPIO_PIN_10
#define ENC1_SW_GPIO_Port GPIOA
#define ENC1_SW_EXTI_IRQn EXTI4_15_IRQn
#define ENC3_A_Pin GPIO_PIN_11
#define ENC3_A_GPIO_Port GPIOA
#define ENC3_B_Pin GPIO_PIN_12
#define ENC3_B_GPIO_Port GPIOA
#define ENC3_SW_Pin GPIO_PIN_15
#define ENC3_SW_GPIO_Port GPIOA
#define ENC3_SW_EXTI_IRQn EXTI4_15_IRQn
#define ENC4_SW_Pin GPIO_PIN_3
#define ENC4_SW_GPIO_Port GPIOB
#define ENC4_SW_EXTI_IRQn EXTI2_3_IRQn
#define ENC4_A_Pin GPIO_PIN_4
#define ENC4_A_GPIO_Port GPIOB
#define ENC5_A_Pin GPIO_PIN_5
#define ENC5_A_GPIO_Port GPIOB
#define ENC5_B_Pin GPIO_PIN_6
#define ENC5_B_GPIO_Port GPIOB
#define ENC6_SW_Pin GPIO_PIN_7
#define ENC6_SW_GPIO_Port GPIOB
#define ENC6_SW_EXTI_IRQn EXTI4_15_IRQn

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
