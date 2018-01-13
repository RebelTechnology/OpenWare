/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @brief          : Generic media access Layer.
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_AUDIO 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_TYPES */
/* USER CODE END PRIVATE_TYPES */ 
/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */
  
/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */ 
/* USER CODE BEGIN PRIVATE_MACRO */
/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Private_Variables
  * @{
  */
/* USER CODE BEGIN PRIVATE_VARIABLES */
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */ 
  
/** @defgroup USBD_AUDIO_IF_Exported_Variables
  * @{
  */ 
  extern USBD_HandleTypeDef hUsbDeviceHS;  
/* USER CODE BEGIN EXPORTED_VARIABLES */
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */ 
  
/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */

static int8_t  AUDIO_Init_HS         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
static int8_t  AUDIO_DeInit_HS       (uint32_t options);
static int8_t  AUDIO_AudioCmd_HS     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t  AUDIO_VolumeCtl_HS    (uint8_t vol);
static int8_t  AUDIO_MuteCtl_HS      (uint8_t cmd);
static int8_t  AUDIO_PeriodicTC_HS   (uint8_t cmd);
static int8_t  AUDIO_GetState_HS     (void);
/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */ 
  

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_HS = 
{
  AUDIO_Init_HS,
  AUDIO_DeInit_HS,
  AUDIO_AudioCmd_HS,
  AUDIO_VolumeCtl_HS,
  AUDIO_MuteCtl_HS,
  AUDIO_PeriodicTC_HS,
  AUDIO_GetState_HS,
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  AUDIO_Init_HS
  *         Initializes the AUDIO media low layer over ths USB HS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use 
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_Init_HS(uint32_t  AudioFreq, uint32_t Volume, uint32_t options)
{  
  /* USER CODE BEGIN 9 */ 
  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  AUDIO_DeInit_HS
  *         DeInitializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_DeInit_HS(uint32_t options)
{
  /* USER CODE BEGIN 10 */ 
  return (USBD_OK);
  /* USER CODE END 10 */
}

/**
  * @brief  AUDIO_AudioCmd_HS
  *         Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_AudioCmd_HS (uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 11 */
  switch(cmd)
  {
    case AUDIO_CMD_START:    
    break;
  
    case AUDIO_CMD_PLAY:   
    break;	
  }
  return (USBD_OK);
  /* USER CODE END 11 */
  
}

/**
  * @brief  AUDIO_VolumeCtl_HS
  *         Controls AUDIO Volume.  
  * @param  vol: volume level (0..100)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeCtl_HS (uint8_t vol)
{
  /* USER CODE BEGIN 12 */ 
  return (USBD_OK);
  /* USER CODE END 12 */
}

/**
  * @brief  AUDIO_MuteCtl_HS
  *         Controls AUDIO Mute.     
  * @param  cmd: command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteCtl_HS (uint8_t cmd)
{
  /* USER CODE BEGIN 13 */ 
  return (USBD_OK);
  /* USER CODE END 13 */
}

/**
  * @brief  AUDIO_PeriodicTC_HS              
  * @param  cmd: command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_PeriodicTC_HS (uint8_t cmd)
{
  /* USER CODE BEGIN 14 */ 
  return (USBD_OK);
  /* USER CODE END 14 */
}

/**
  * @brief  AUDIO_GetState_HS
  *         Gets AUDIO State.  
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_GetState_HS (void)
{
  /* USER CODE BEGIN 15 */ 
  return (USBD_OK);
  /* USER CODE END 15 */
}

/**
  * @brief  Manages the DMA full Transfer complete event.
  * @param  None
  * @retval None
  */
void TransferComplete_CallBack_HS(void)
{
  /* USER CODE BEGIN 16 */ 
  USBD_AUDIO_Sync(&hUsbDeviceHS, AUDIO_OFFSET_FULL);
  /* USER CODE END 16 */
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
void HalfTransfer_CallBack_HS(void)
{ 
  /* USER CODE BEGIN 17 */ 
  USBD_AUDIO_Sync(&hUsbDeviceHS, AUDIO_OFFSET_HALF);
  /* USER CODE END 17 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */ 

/**
  * @}
  */  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

