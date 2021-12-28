#include "main.h"
#include "stm32f4xx_hal.h"

/* #define OWL_ALCHEMIST_LED */
#define OWL_ALCHEMIST
#define HARDWARE_ID                  ALCHEMIST_HARDWARE
#define HARDWARE_VERSION             "Alchemist"

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
#define USBD_HANDLE hUsbDeviceHS

#define USE_MODE_BUTTON
#define MODE_BUTTON_PIN SW3_Pin
#define MODE_BUTTON_PORT SW3_GPIO_Port
#define MODE_BUTTON_GAIN ADC_C
#define MODE_BUTTON_PATCH ADC_D

#define USE_BKPSRAM

#define USE_RGB_LED
#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 2
#define ADC_B 3
#define ADC_C 0
#define ADC_D 1
#define USE_CODEC
#define USE_CS4271
#define CODEC_HP_FILTER
#define CODEC_SPI hspi4
#define USE_USBD_HS

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
