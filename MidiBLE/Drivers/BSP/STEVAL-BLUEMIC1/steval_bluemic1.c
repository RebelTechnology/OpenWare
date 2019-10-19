/**
 ******************************************************************************
 * @file    steval_bluemic1.c
 * @author  Central Lab
 * @version V1.0.0
 * @date    16-May-2017
 * @brief   This file provides low level functionalities for STEVAL-BLUEMIC-1
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

/* Includes ------------------------------------------------------------------*/

#include "steval_bluemic1.h"

/** @defgroup BSP BSP
 * @{
 */

/** @defgroup BLUEMIC1_LOW_LEVEL BLUEMIC1_LOW_LEVEL
 * @{
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Defines BLUEMIC1_LOW_LEVEL_Private_Defines
* @{
*/  

/**
 * @brief  I2C DR address
 */
#define I2C_TX_DR_BASE_ADDR                (I2C2_BASE + 0x10)

#define I2C_RX_DR_BASE_ADDR                (I2C2_BASE + 0x18)
   
/**
 * @}
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Variables BLUEMIC1_LOW_LEVEL_Private_Variables
 * @{
 */

/**
 * @brief I2C buffers used for DMA application
 */
uint8_t i2c_buffer_tx[I2C_BUFF_SIZE];
uint8_t i2c_buffer_rx[I2C_BUFF_SIZE];

/**
 * @brief  I2C flag set at end of transaction
 */
volatile FlagStatus i2c_eot = SET;

/**
 * @}
 */

void StevalBlueMic1_I2CDmaInit(uint32_t baudrate);
void StevalBlueMic1_I2CDmaRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
void StevalBlueMic1_I2CDmaWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);
void StevalBlueMic1_I2CIrqInit(uint32_t baudrate);
void StevalBlueMic1_I2CIrqRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
void StevalBlueMic1_I2CIrqWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);
void StevalBlueMic1_I2CInit(uint32_t baudrate);
DrvStatusTypeDef StevalBlueMic1_I2CRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead);
DrvStatusTypeDef StevalBlueMic1_I2CWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite);


/** @defgroup BLUEMIC1_LOW_LEVEL_Exported_Functions BLUEMIC1_LOW_LEVEL_Exported_Functions
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
void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{  
  
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *                 This parameter should be: BUTTON_USER 
  * @retval The Button GPIO pin value.
  */
uint32_t BSP_PB_GetState(Button_TypeDef Button)
{
	return 0;
}

/**
 * @brief  Get the pending bit state.
 * @param  Button: Specifies the Button to be checked.
 *                 This parameter should be: BUTTON_USER 
 * @retval This parameter can be: 1 or 0.
 */
uint32_t BSP_PB_GetITPendingBit(Button_TypeDef Button)
{
  return 0;
}

/**
 * @brief  Clear the pending bit state.
 * @param  Button: Specifies the Button ID.
 *                 This parameter should be: BUTTON_USER 
 * @retval None
 */
void BSP_PB_ClearITPendingBit(Button_TypeDef Button)
{

}

/**
 * @brief  Configures LEDs.
 * @param  Led Specifies the Led to be configured.
 *         This parameter can be one of following parameters:
 *         @arg LED1
 *         @arg LED2
 * @retval None.
 */
void BSP_LED_Init(Led_TypeDef Led)
{

}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set On. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_On(Led_TypeDef Led)
{
  //GPIO_WriteBit(LED_PIN[Led], Bit_SET);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set Off. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Off(Led_TypeDef Led)
{
  //GPIO_WriteBit(LED_PIN[Led], Bit_RESET);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *         This parameter can be one of following parameters:
  *         @arg LED1
  *         @arg LED2
  */
void BSP_LED_Toggle(Led_TypeDef Led)
{
  //GPIO_ToggleBits(LED_PIN[Led]);
}


/**
 * @brief  Configures sensor I2C interface.
 * @param  None
 * @retval None
 */
void Sensor_IO_Init(void)
{

}


/**
 * @brief  Writes a buffer to the sensor
 * @param  handle instance handle
 * @param  WriteAddr specifies the internal sensor address register to be written to
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToWrite number of bytes to be written
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Write(void *handle, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t nBytesToWrite)
{
  
return 0;
}



/**
 * @brief  Reads a from the sensor to buffer
 * @param  handle instance handle
 * @param  ReadAddr specifies the internal sensor address register to be read from
 * @param  pBuffer pointer to data buffer
 * @param  nBytesToRead number of bytes to be read
 * @retval 0 in case of success
 * @retval 1 in case of failure
 */
uint8_t Sensor_IO_Read( void *handle, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t nBytesToRead )
{
	return COMPONENT_OK;
}


/**
 * @brief  Configures sensor interrupts interface for LSM6DSL sensor.
 * @param  None
 * @retval COMPONENT_OK in case of success
 * @retval COMPONENT_ERROR in case of failure
 */
DrvStatusTypeDef LSM6DSL_Sensor_IO_ITConfig( void )
{

  
  return COMPONENT_OK;
}

/**
 * @}
 */

/** @defgroup BLUEMIC1_LOW_LEVEL_Private_Functions BLUEMIC1_LOW_LEVEL_Private_Functions
* @{
*/ 

/**
 * @brief  Configures I2C interface
 * @param  baudrate I2C clock frequency
 * @retval None
 */
void StevalBlueMic1_I2CInit(uint32_t baudrate)
{   

}

/**
 * @brief  I2C function to write registers of a slave device
 * @param  pBuffer buffer contains data to write
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to write
 * @retval SUCCESS in case of success, an error code otherwise
 */
DrvStatusTypeDef StevalBlueMic1_I2CWrite(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToWrite)
{

  return COMPONENT_OK;
}

/**
 * @brief  I2C function to read registers from a slave device
 * @param  pBuffer buffer to retrieve data from a slave
 * @param  DeviceAddr the I2C slave address
 * @param  RegisterAddr register address
 * @param  NumByteToRead number of byte to read
 * @retval SUCCESS in case of success, an error code otherwise
 */
DrvStatusTypeDef StevalBlueMic1_I2CRead(uint8_t* pBuffer, uint8_t DeviceAddr, uint8_t RegisterAddr, uint8_t NumByteToRead)
{
  
  return COMPONENT_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
