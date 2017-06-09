
#ifndef __USB_MIDI_CORE_H
#define __USB_MIDI_CORE_H

#include  <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_Midi
  * @brief This file is the header file for usbd_Midi_core.c
  * @{
  */ 


/** @defgroup USBD_Midi_Exported_Defines
  * @{
  */ 
#define MIDI_EPIN_ADDR                 0x81
#define MIDI_EPIN_SIZE                 0x10

#define USB_MIDI_CONFIG_DESC_SIZ       0x65

#define MIDI_OUT_EP                    0x01
#define MIDI_IN_EP                     0x81
#define MIDI_DATA_IN_PACKET_SIZE       0x40
#define MIDI_DATA_OUT_PACKET_SIZE      0x40

#define MIDI_BUF_SIZE                  64


#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02
#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_CUR                             0x01

extern USBD_ClassTypeDef  USBD_Midi_ClassDriver;

typedef struct _USBD_Midi_Itf
{
  void(*Receive)(uint8_t *, uint32_t);  

}USBD_Midi_ItfTypeDef;


typedef struct
{
  __IO uint32_t alt_setting; 
  uint8_t rxBuffer[MIDI_BUF_SIZE];
  uint32_t rxLen;
}
USBD_Midi_HandleTypeDef; 


uint8_t  USBD_Midi_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_Midi_ItfTypeDef *fops);

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MIDI_CORE_H */
