#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_NOCTUA
#define HARDWARE_ID                  NOCTUA_HARDWARE
#define HARDWARE_VERSION             "Noctua"

#define AUDIO_CHANNELS               8

/* #define USE_USB_AUDIO */
/* #define USE_USBD_MIDI */
/* #define USE_USBD_AUDIO_IN // microphone */

/* #define USE_MODE_BUTTON */
/* #define MODE_BUTTON_PIN SW3_Pin */
/* #define MODE_BUTTON_PORT SW3_GPIO_Port */
/* #define MODE_BUTTON_GAIN ADC_C */
/* #define MODE_BUTTON_PATCH ADC_D */

/* #define USE_BKPSRAM enable to prevent reset loops */ 

/* #define USE_RGB_LED */
/* #define USE_ADC */
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define ADC_E 4
#define ADC_F 5
#define ADC_G 6
#define ADC_H 7
#define USE_CODEC
#define USE_IIS3DWB

/* #define USE_USB_AUDIO */
/* #define USE_USBD_AUDIO_IN // microphone */
#define USE_USBD_MIDI
#define USE_USBD_FS

#define AUDIO_SAMPLINGRATE          16000
#define TIM8_PERIOD                 (871*48000/AUDIO_SAMPLINGRATE) /* experimentally determined */

/* #define USE_PCM3168A */
#define CODEC_HP_FILTER
#define CODEC_SPI hspi2
/* #define USE_USB_HOST */
#define USBH_HANDLE hUsbHostHS
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */

#define USB_HOST_PWR_EN_GPIO_Port GPIOB
#define USB_HOST_PWR_EN_Pin GPIO_PIN_0 // PB0 is unused
#define USB_HOST_PWR_FAULT_GPIO_Port GPIOB
#define USB_HOST_PWR_FAULT_Pin GPIO_PIN_1 // PB1 is unused
  
#define NOF_ADC_VALUES               8
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
