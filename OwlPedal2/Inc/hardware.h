#include "main.h"
#include "stm32f4xx_hal.h"

#define DEBUG_STACK

#define OWL_PEDAL_MKII
#define HARDWARE_ID                  OWL_PEDAL_HARDWARE
#define HARDWARE_VERSION             "OWL Pedal mkII"

#define USE_USBD_AUDIO
#define USE_USBD_RX_FB
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_TX  // microphone
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_MIDI
#define USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define USB_AUDIO_CHANNELS          2
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_BKPSRAM

/* #define USE_MODE_BUTTON */
/* #define MODE_BUTTON_PIN SW3_Pin */
/* #define MODE_BUTTON_PORT SW3_GPIO_Port */
/* #define MODE_BUTTON_GAIN ADC_C */
/* #define MODE_BUTTON_PATCH ADC_D */

#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define ADC_E 4
#define USE_CODEC
#define USE_CS4271
/* #define CODEC_HP_FILTER */
#define CODEC_SPI hspi4

#define NOF_ADC_VALUES               5
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
