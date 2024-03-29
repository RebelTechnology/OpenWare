#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_WITCH
#define HARDWARE_ID                  WITCH_HARDWARE
#define HARDWARE_VERSION             "Witch"
#define USBD_PRODUCT_STRING_FSHS     "OWL-WITCH"

#define USE_SPI_FLASH

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define USBD_AUDIO_RX_CHANNELS      AUDIO_CHANNELS
#define USBD_AUDIO_TX_CHANNELS      AUDIO_CHANNELS
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_USBD_AUDIO
#define USE_USBD_RX_FB
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_AUDIO_TX  // microphone
#define USE_USBD_FS
#define USE_USBH_HS
#define USE_USB_HOST
#define USE_USBH_MIDI

#define AUDIO_BLOCK_SIZE             128

#define USE_BKPSRAM

#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define ADC_E 4
#define ADC_F 5
#define ADC_G 6
#define ADC_H 7
#define ADC_I 8
#define AUDIO_OUTPUT_GAIN            103

#define TRIG1_Pin GP7_Pin 	/* A17 */
#define TRIG1_GPIO_Port GP7_GPIO_Port
#define TRIG2_Pin GP8_Pin	/* A14 */
#define TRIG2_GPIO_Port GP8_GPIO_Port

#define USE_DAC
#define USE_ADC
#define ADC_PERIPH hadc3
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4
#define CODEC_HP_FILTER

#define NOF_ADC_VALUES               9
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4+4)
