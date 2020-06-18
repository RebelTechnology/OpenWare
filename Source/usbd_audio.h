#ifndef __USBD_AUDIO_H
#define __USBD_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_ioreq.h"
#include "device.h"

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01

#define AUDIO_REQ_SET_CUR                             0x01
#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_GET_MIN                             0x82
#define AUDIO_REQ_GET_MAX                             0x83
#define AUDIO_REQ_GET_RES                             0x84

#define AUDIO_IN_STREAMING_CTRL                       0x03
#define AUDIO_OUT_STREAMING_CTRL                      0x04

#define AUDIO_RX_PACKET_SIZE                          (uint32_t)(((USBD_AUDIO_FREQ * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE) /1000)) 
#define AUDIO_TX_PACKET_SIZE                          (uint32_t)(((USBD_AUDIO_FREQ * USB_AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE) /1000))
    
/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define AUDIO_RX_PACKET_NUM                           1
#define AUDIO_TX_PACKET_NUM                           1
/* Total size of the audio transfer buffer */
#define AUDIO_RX_TOTAL_BUF_SIZE                       ((uint32_t)(AUDIO_RX_PACKET_SIZE * AUDIO_RX_PACKET_NUM))
/* Total size of the IN (i.e. microphopne) transfer buffer */
#define AUDIO_TX_TOTAL_BUF_SIZE                       ((uint32_t)(AUDIO_TX_PACKET_SIZE * AUDIO_TX_PACKET_NUM))

#define MIDI_DATA_IN_PACKET_SIZE       0x40
#define MIDI_DATA_OUT_PACKET_SIZE      0x40

#define USBD_EP_ATTR_ISOC_ASYNC                           0x04 /* Asynchronous isochronic transfer  */
#define USBD_EP_ATTR_ISOC_ADAPT                           0x08 /* Adaptative isochronic transfer  */
#define USBD_EP_ATTR_ISOC_SYNC                            0x0C /* Synchronous isochronic transfer  */

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
  uint8_t                   tx_alt_setting, rx_alt_setting, midi_alt_setting;
#ifdef USE_USBD_AUDIO_TX
  uint8_t                   audio_tx_buffer[AUDIO_TX_TOTAL_BUF_SIZE];
#endif
#ifdef USE_USBD_AUDIO_RX
  uint8_t                   audio_rx_buffer[AUDIO_RX_TOTAL_BUF_SIZE];
#endif
#ifdef USE_USBD_MIDI
  uint8_t                   midi_rx_buffer[MIDI_DATA_OUT_PACKET_SIZE];
#endif
  volatile uint8_t          midi_tx_lock;
  volatile uint8_t          audio_tx_active;
  USBD_AUDIO_ControlTypeDef control;   
}
USBD_AUDIO_HandleTypeDef; 

extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO

uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        void *fops);

void usbd_audio_tx_start_callback(uint16_t rate, uint8_t channels);
void usbd_audio_tx_stop_callback();
void usbd_audio_tx_callback(uint8_t* data, size_t len);
void usbd_audio_rx_start_callback(uint16_t rate, uint8_t channels);
void usbd_audio_rx_stop_callback();
size_t usbd_audio_rx_callback(uint8_t* data, size_t len);
void usbd_audio_gain_callback(uint8_t gain);
void usbd_audio_sync_callback(uint8_t gain);


void usbd_audio_write(uint8_t* buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_AUDIO_H */
