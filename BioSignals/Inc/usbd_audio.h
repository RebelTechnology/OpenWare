/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   header file for the usbd_audio.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_AUDIO_H
#define __USB_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include "device.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_AUDIO
  * @brief This file is the Header file for usbd_audio.c
  * @{
  */ 

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_UNDEFINED                      0x00
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_SUBCLASS_MIDISTREAMING                  0x03
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_MIXER_UNIT                      0x04
#define AUDIO_CONTROL_SELECTOR_UNIT                   0x05
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06
#define AUDIO_CONTROL_PROCESSING_UNIT                 0x07
#define AUDIO_CONTROL_EXTENSION_UNIT                  0x08

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x0001
#define AUDIO_CONTROL_VOLUME                          0x0002
#define AUDIO_CONTROL_BASS                            0x0003
#define AUDIO_CONTROL_MID                             0x0004
#define AUDIO_CONTROL_TREBLE                          0x0005
#define AUDIO_CONTROL_GRAPHIC_EQ                      0x0006
#define AUDIO_CONTROL_AUTO_GAIN                       0x0007
#define AUDIO_CONTROL_DELAY                           0x0008
#define AUDIO_CONTROL_BASS_BOOST                      0x0009
#define AUDIO_CONTROL_LOUDNESS                        0x000A

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

#define AUDIO_IN_STREAMING_CTRL                       0x03
#define AUDIO_OUT_STREAMING_CTRL                      0x04

#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE) /1000)) 
#define AUDIO_IN_PACKET_SIZE                          (uint32_t)(((USBD_AUDIO_FREQ * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE) /1000))
#define AUDIO_DEFAULT_VOLUME                          70
    
/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define AUDIO_OUT_PACKET_NUM                          40
#define AUDIO_IN_PACKET_NUM                           1
/* Total size of the audio transfer buffer */
#define AUDIO_TOTAL_BUF_SIZE                          ((uint32_t)(AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM))
/* Total size of the IN (i.e. microphopne) transfer buffer */
#define AUDIO_IN_TOTAL_BUF_SIZE                       ((uint32_t)(AUDIO_IN_PACKET_SIZE * AUDIO_IN_PACKET_NUM))

#define MIDI_BUF_SIZE                                 64
    
    /* Audio Commands enumeration */
typedef enum
{
  AUDIO_CMD_START = 1,
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
}AUDIO_CMD_TypeDef;


typedef enum
{
  AUDIO_OFFSET_NONE = 0,
  AUDIO_OFFSET_HALF,
  AUDIO_OFFSET_FULL,  
  AUDIO_OFFSET_UNKNOWN,    
}
AUDIO_OffsetTypeDef;
/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
 typedef struct
{
   uint8_t cmd;   
   uint8_t data[USB_MAX_EP0_SIZE];  
   uint8_t len;  
   uint8_t unit;    
}
USBD_AUDIO_ControlTypeDef; 



typedef struct
{
  __IO uint32_t             alt_setting; 
#ifdef USE_USBD_AUDIO
  uint8_t                   audio_out_buffer[AUDIO_TOTAL_BUF_SIZE];
#endif
#ifdef USE_USBD_MIDI
  uint8_t                   midi_in_buffer[MIDI_BUF_SIZE];
#endif
  AUDIO_OffsetTypeDef       offset;
  uint8_t                    rd_enable;  
  uint16_t                   rd_ptr;  
  uint16_t                   wr_ptr;  
  USBD_AUDIO_ControlTypeDef control;   
}
USBD_AUDIO_HandleTypeDef; 


typedef struct
{
    int8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
    int8_t  (*DeInit)       (uint32_t options);
    int8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
    int8_t  (*VolumeCtl)    (uint8_t vol);
    int8_t  (*MuteCtl)      (uint8_t cmd);
    int8_t  (*PeriodicTC)   (uint8_t cmd);
    int8_t  (*GetState)     (void);
}USBD_AUDIO_ItfTypeDef;
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_AUDIO_ItfTypeDef *fops);

void  USBD_AUDIO_Sync (USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset);


   void usbd_audio_start_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio);
   void usbd_audio_data_in_callback(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef* haudio);
   void usbd_audio_gain_callback(uint8_t gain);
   void usbd_audio_write(USBD_HandleTypeDef* pdev, uint8_t* buf, uint32_t len);

   /**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_AUDIO_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
