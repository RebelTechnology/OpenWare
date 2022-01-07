#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_WIZARD
#define HARDWARE_ID                  WIZARD_HARDWARE
#define HARDWARE_VERSION             "Wizard"
/* #define FASCINATION_MACHINE */

#define USE_SPI_FLASH

#ifndef OWL_EUROWIZARD
#define USE_USBD_HS
#define USBD_HANDLE hUsbDeviceHS
#define USBH_HANDLE hUsbHostFS
#define CODEC_HP_FILTER
#else
#define USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#define USBH_HANDLE hUsbHostHS
#define CODEC_HP_FILTER
#define CODEC_LR_SWAP
#endif

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

#define USE_MODE_BUTTON
#define MODE_BUTTON_PIN SW5_Pin
#define MODE_BUTTON_PORT SW5_GPIO_Port
#define MODE_BUTTON_GAIN ADC_D
#define MODE_BUTTON_PATCH ADC_E

#define USE_BKPSRAM

#ifdef OWL_EUROWIZARD
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define ADC_E 4
#define AUDIO_OUTPUT_GAIN            127
#else
#define ADC_A 2
#define ADC_B 1
#define ADC_C 0
#define ADC_D 4
#define ADC_E 3
#endif

#define USE_RGB_LED
#define USE_DAC
#define TRIG1_Pin GP7_Pin
#define TRIG1_GPIO_Port GP7_GPIO_Port
#define TRIG2_Pin GP8_Pin
#define TRIG2_GPIO_Port GP8_GPIO_Port
#define USE_ADC
#define ADC_PERIPH hadc3
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4
#define USE_USB_HOST
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */

#define NOF_ADC_VALUES               5
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4+4)
