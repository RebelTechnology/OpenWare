/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
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
#ifndef __MXCONSTANT_H
#define __MXCONSTANT_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define MAX_nCS_Pin GPIO_PIN_4
#define MAX_nCS_GPIO_Port GPIOA
#define MAX_SCLK_Pin GPIO_PIN_5
#define MAX_SCLK_GPIO_Port GPIOA
#define MAX_DOUT_Pin GPIO_PIN_6
#define MAX_DOUT_GPIO_Port GPIOA
#define MAX_DIN_Pin GPIO_PIN_7
#define MAX_DIN_GPIO_Port GPIOA
#define TLC_BLANK_Pin GPIO_PIN_12
#define TLC_BLANK_GPIO_Port GPIOB
#define TLC_SCLK_Pin GPIO_PIN_13
#define TLC_SCLK_GPIO_Port GPIOB
#define TLC_SOUT_Pin GPIO_PIN_14
#define TLC_SOUT_GPIO_Port GPIOB
#define TLC_SIN_Pin GPIO_PIN_15
#define TLC_SIN_GPIO_Port GPIOB
#define TLC_MODE_Pin GPIO_PIN_8
#define TLC_MODE_GPIO_Port GPIOA
#define TLC_GSCLK_Pin GPIO_PIN_9
#define TLC_GSCLK_GPIO_Port GPIOA
#define TLC_XLAT_Pin GPIO_PIN_10
#define TLC_XLAT_GPIO_Port GPIOA
#define TLC_XERR_Pin GPIO_PIN_11
#define TLC_XERR_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

#define MAX_CS_GPIO_Port MAX_nCS_GPIO_Port
#define MAX_CS_Pin MAX_nCS_Pin

/* USER CODE END Private defines */

/**
  * @}
  */

/**
  * @}
*/

#endif /* __MXCONSTANT_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
