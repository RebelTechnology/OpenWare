#ifndef __USBD_AUDIO_H
#define __USBD_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_ioreq.h"
#include "midi.h"
#include "device.h"

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01

/* USB AUDIO class bRequest types */
#define AUDIO_REQ_SET_CUR                             0x01
#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_SET_MIN                             0x02
#define AUDIO_REQ_GET_MIN                             0x82
#define AUDIO_REQ_SET_MAX                             0x03
#define AUDIO_REQ_GET_MAX                             0x83
#define AUDIO_REQ_SET_RES                             0x04
#define AUDIO_REQ_GET_RES                             0x84
#define AUDIO_REQ_SET_MEM                             0x05
#define AUDIO_REQ_GET_MEM                             0x85
#define AUDIO_REQ_GET_STAT                            0xff   

#define AUDIO_IN_STREAMING_CTRL                       0x03
#define AUDIO_OUT_STREAMING_CTRL                      0x04

#define AUDIO_RX_SAMPLES_PER_MS                       (USBD_AUDIO_RX_FREQ / 1000)
#define AUDIO_RX_PACKET_SIZE                          (AUDIO_RX_SAMPLES_PER_MS * USBD_AUDIO_RX_CHANNELS * AUDIO_BYTES_PER_SAMPLE)
#define AUDIO_RX_MAX_PACKET_SIZE                      (AUDIO_RX_PACKET_SIZE + USBD_AUDIO_RX_CHANNELS * AUDIO_BYTES_PER_SAMPLE)

#define AUDIO_FB_PACKET_SIZE                          3U

#define AUDIO_TX_SAMPLES_PER_MS                       (USBD_AUDIO_TX_FREQ / 1000)
#define AUDIO_TX_PACKET_SIZE                          (AUDIO_TX_SAMPLES_PER_MS * USBD_AUDIO_TX_CHANNELS * AUDIO_BYTES_PER_SAMPLE)
#define AUDIO_TX_MAX_PACKET_SIZE                      (AUDIO_TX_PACKET_SIZE + USBD_AUDIO_TX_CHANNELS * AUDIO_BYTES_PER_SAMPLE)

/* Number of sub-packets in the audio transfer buffer. */
#define AUDIO_RX_PACKET_NUM                           5
#define AUDIO_TX_PACKET_NUM                           5

/* Total size of the OUT audio transfer buffer */
#define AUDIO_RX_TOTAL_BUF_SIZE                       (AUDIO_RX_PACKET_SIZE * AUDIO_RX_PACKET_NUM)
/* Total size of the IN transfer buffer */
#define AUDIO_TX_TOTAL_BUF_SIZE                       (AUDIO_TX_PACKET_SIZE * AUDIO_TX_PACKET_NUM)

#define MIDI_TX_PACKET_SIZE                           0x40
#define MIDI_RX_PACKET_SIZE                           0x40

#ifdef STM32H7xx
#define USBD_TOTAL_FIFO_SIZE                          0x1000 /* 4k FIFO buffers on H7 */
#else
#define USBD_TOTAL_FIFO_SIZE                          0x500  /* 1.25k FIFO buffers on F4 */
#endif
#define USBD_MIN_FIFO_SIZE                            0x40
 
/* Isochronous Synchronisation Type */
#define USBD_EP_ATTR_ISOC_NOSYNC                      0x00 /* no synchro */
#define USBD_EP_ATTR_ISOC_ASYNC                       0x04 /* synchronisation by feedback  */
#define USBD_EP_ATTR_ISOC_ADAPT                       0x08 /* adaptative synchronisation   */
#define USBD_EP_ATTR_ISOC_SYNC                        0x0C /* synchronous mode  */

/* Isochronous Usage Type */
#define USBD_EP_ATTR_ISOC_DATA                        0x00 /* Data Endpoint  */
#define USBD_EP_ATTR_ISOC_FB                          0x10 /* Feedback Endpoint  */
#define USBD_EP_ATTR_ISOC_IMPL_FB                     0x20 /* Implicit Feedback Data Endpoint  */

   
/* Class-Specific AS Isochronous Audio Data Endpoint Descriptor bmAttributes */
#define USBD_AUDIO_AS_CONTROL_SAMPLING_FREQUENCY             0x0001 /* D0 = 1*/
#define USBD_AUDIO_AS_CONTROL_PITCH                          0x0002 /* D1 = 1*/
#define USBD_AUDIO_AS_CONTROL_MAX_PACKET_ONLY                0x0080 /* D7 = 1*/
   
#if 1
/* Audio Control Requests */
#define AUDIO_CONTROL_REQ                             0x01U
/* Feature Unit, UAC Spec 1.0 p.102 */
#define AUDIO_CONTROL_REQ_FU_MUTE                     0x01U
#define AUDIO_CONTROL_REQ_FU_VOL                      0x02U

/* Audio Streaming Requests */
#define AUDIO_STREAMING_REQ                           0x02U
#define AUDIO_STREAMING_REQ_FREQ_CTRL                 0x01U
#define AUDIO_STREAMING_REQ_PITCH_CTRL                0x02U

/* Volume. See UAC Spec 1.0 p.77 */
#ifndef USBD_AUDIO_VOL_DEFAULT
#define USBD_AUDIO_VOL_DEFAULT                        0x8d00U
#endif
#ifndef USBD_AUDIO_VOL_MAX
#define USBD_AUDIO_VOL_MAX                            0x0000U
#endif
#ifndef USBD_AUDIO_VOL_MIN
#define USBD_AUDIO_VOL_MIN                            0x8100U
#endif
#ifndef USBD_AUDIO_VOL_STEP
#define USBD_AUDIO_VOL_STEP                           0x0100U
#endif /* Total number of steps can't be too many, host will complain. */

#endif

#if AUDIO_BITS_PER_SAMPLE == 8
typedef int8_t audio_t;
#elif AUDIO_BITS_PER_SAMPLE == 16
typedef int16_t audio_t;
#elif AUDIO_BITS_PER_SAMPLE == 32
typedef int32_t audio_t;
#else
#error "Unsupported AUDIO_BITS_PER_SAMPLE"
#endif

/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
 typedef struct
{
   uint8_t cmd;                    /* bRequest */
   uint8_t req_type;               /* bmRequest */
   uint8_t cs;                     /* wValue (high byte): Control Selector */
   uint8_t cn;                     /* wValue (low byte): Control Number */
   uint8_t unit;                   /* wIndex: Feature Unit ID, Extension Unit ID, or Interface, Endpoint */
   uint8_t len;                    /* wLength */
   uint8_t data[USB_MAX_EP0_SIZE]; /* Data */
}
USBD_AUDIO_ControlTypeDef;

typedef struct
{
  uint8_t                   ac_alt_setting, tx_alt_setting, rx_alt_setting, midi_alt_setting;
#ifdef USE_USBD_AUDIO_TX
  uint8_t                   audio_tx_buffer[AUDIO_TX_TOTAL_BUF_SIZE];
  uint8_t                   audio_tx_transmit[AUDIO_TX_MAX_PACKET_SIZE];
  volatile uint8_t          audio_tx_active;
  volatile uint16_t         tx_soffn;
#endif
#ifdef USE_USBD_AUDIO_RX
  uint8_t                   audio_rx_buffer[AUDIO_RX_TOTAL_BUF_SIZE];
  uint8_t                   audio_rx_transmit[AUDIO_RX_MAX_PACKET_SIZE];
  volatile uint8_t          audio_rx_active;
#ifdef USE_USBD_RX_FB
  volatile uint16_t         fb_soffn;
  union {
    uint8_t buf[3];
    uint32_t val;
  } fb_data;
#endif
#endif
#ifdef USE_USBD_MIDI
  uint8_t                   midi_rx_buffer[MIDI_RX_PACKET_SIZE];
  volatile uint8_t          midi_tx_lock;
#endif
  int16_t                   volume;
  uint32_t                  frequency;
  uint32_t                  bit_depth;
  USBD_AUDIO_ControlTypeDef control;   
}
USBD_AUDIO_HandleTypeDef; 

extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO

uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, void *fops);
uint8_t  USBD_AUDIO_SetFiFos(PCD_HandleTypeDef *hpcd);

void usbd_audio_tx_start_callback(size_t rate, uint8_t channels, void* cb);
void usbd_audio_tx_stop_callback();
void usbd_audio_tx_callback(uint8_t* data, size_t len);
void usbd_audio_rx_start_callback(size_t rate, uint8_t channels, void* cb);
void usbd_audio_rx_stop_callback();
size_t usbd_audio_rx_callback(uint8_t* data, size_t len);
void usbd_audio_mute_callback(int16_t gain);
void usbd_audio_gain_callback(int16_t gain);
void usbd_audio_write(uint8_t* buffer, size_t len);
uint32_t usbd_audio_get_rx_count();

void usbd_midi_rx(uint8_t *buffer, uint32_t length);
void usbd_midi_tx(uint8_t* buffer, uint32_t length);
uint8_t usbd_midi_connected(void);
uint8_t usbd_midi_ready(void);

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_AUDIO_H */
