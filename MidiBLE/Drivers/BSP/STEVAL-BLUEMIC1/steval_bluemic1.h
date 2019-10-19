/**
  ******************************************************************************
  * @file    steval_bluemic1.h
  * @author  Central Lab
  * @version V1.0.0
  * @date    16-May-2017
  * @brief   This file contains definitions for steval_bluemic1.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#ifndef __STEVAL_BLUEMIC1_H
#define __STEVAL_BLUEMIC1_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "BlueNRG1.h"
#include "BlueNRG1_conf.h"
#include "accelerometer.h"
#include "gyroscope.h"

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup BLUEMIC1_LOW_LEVEL BLUEMIC1_LOW_LEVEL
 * @{
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Exported_Defines BLUEMIC1_LOW_LEVEL_Exported_Defines
* @{
*/  
  
/**
 * @brief BlueMic-1 Button, LEDs and interrupt
 */

#define MODE_PIN        GPIO_Pin_4
#define SPI_NCS_PIN			GPIO_Pin_1
#define SPI_CLK_PIN			GPIO_Pin_0
#define SPI_MOSI_PIN		GPIO_Pin_3
#define SPI_MISO_PIN		GPIO_Pin_2

/**
 * @brief  I2C buffer max size
 */
#define I2C_BUFF_SIZE                   (10)

/**
 * @brief  I2C_TX DMA channel
 */
#define DMA_CH_I2C_TX                   BLUE_MIC1_I2C_DMA_TX

/**
 * @brief  I2C_TX DMA Transfer Complete Interrupt
 */
#define DMA_CH_I2C_TX_IT_TC             BLUE_MIC1_I2C_DMA_TX_ITTC

/**
 * @brief  I2C_RX DMA channel
 */
#define DMA_CH_I2C_RX                   BLUE_MIC1_I2C_DMA_RX

/**
 * @brief  I2C_RX DMA Transfer Complete Interrupt
 */
#define DMA_CH_I2C_RX_IT_TC             BLUE_MIC1_I2C_DMA_RX_ITTC

/**
 * @}
 */ 
   
/** @defgroup BLUEMIC1_LOW_LEVEL_Exported_Typedef BLUEMIC1_LOW_LEVEL_Exported_Typedef
* @{
*/ 
  
typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;   
   
typedef enum 
{  
  BUTTON_USER = 0,
} Button_TypeDef;
 
typedef enum 
{  
  LED1 = 0,
  LED2 = 1
} Led_TypeDef;

/**
 * @}
 */ 
   
/** @defgroup BLUEMIC1_LOW_LEVEL_Exported_Functions_Prototype BLUEMIC1_LOW_LEVEL_Exported_Functions_Prototype
* @{
*/ 
   
/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured. This parameter should be: BUTTON_KEY
  * @param  ButtonMode: Specifies Button mode.
  *         This parameter can be one of following parameters:   
  *         @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
  *         @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt generation capability  
  */
void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *                 This parameter should be: BUTTON_USER 
  * @retval The Button GPIO pin value.
  */
uint32_t BSP_PB_GetState(Button_TypeDef Button);

/**
 * @brief  Clear the pending bit state.
 * @param  Button: Specifies the Button ID.
 *                 This parameter should be: BUTTON_USER 
 * @retval None
 */
void BSP_PB_ClearITPendingBit(Button_TypeDef Button);

/**
 * @brief  Get the pending bit state.
 * @param  Button: Specifies the Button to be checked.
 *                 This parameter should be: BUTTON_USER 
 * @retval This parameter can be: 1 or 0.
 */
uint32_t BSP_PB_GetITPendingBit(Button_TypeDef Button);

/**
 * @brief  Configures LEDs.
 * @param  Led Specifies the Led to be configured.
 *         This parameter can be one of following parameters:
 *         @arg LED1
 *         @arg LED2
 * @retval None.
 */
void BSP_LED_Init(Led_TypeDef Led);

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set On. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_On(Led_TypeDef Led);

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set Off. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Off(Led_TypeDef Led);

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Toggle(Led_TypeDef Led);

/**
 * @brief  Configures sensor I2C interface.
 * @param  None
 * @retval None
 */
void Sensor_IO_Init( void );

/**
 * @brief  Configures sensor interrupts interface for LSM6DSL sensor.
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef LSM6DSL_Sensor_IO_ITConfig( void );

/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Write( void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite );

/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead );

/**
 * @}
 */ 
 
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STEVAL_BLUEMIC1_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
