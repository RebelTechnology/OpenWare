#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_NOCTUA
#define HARDWARE_ID                  NOCTUA_HARDWARE
#define HARDWARE_VERSION             "Noctua"
#undef USE_EXTERNAL_RAM

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
/* #define USE_IIS3DWB */
#define USE_PCM3168A

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              8
#define USB_AUDIO_CHANNELS          4
/* #define AUDIO_INT32_TO_SAMPLE(x)    (__REV16((x)>>8)) */
/* #define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(__REV16(x))<<8) */
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_USBD_AUDIO
#define USE_USBD_AUDIO_TX  // microphone
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#define USBH_HANDLE hUsbHostHS
#define USBD_MAX_POWER              100 // 100mA for iPad compatibility
#define USE_USB_HOST
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */
#define USE_USBH_MIDI

#define USE_UART_MIDI
#define UART_MIDI_HANDLE huart2
#define UART_MIDI_RX_BUFFER_SIZE 256

#define AUDIO_SAMPLINGRATE          48000
#define TIM8_PERIOD                 (871*48000/AUDIO_SAMPLINGRATE) /* experimentally determined */

/* #define USE_PCM3168A */
/* #define CODEC_HP_FILTER */
#define CODEC_SPI hspi2

#define USB_HOST_PWR_EN_GPIO_Port GPIOB
#define USB_HOST_PWR_EN_Pin GPIO_PIN_0 // PB0 is unused
#define USB_HOST_PWR_FAULT_GPIO_Port GPIOB
#define USB_HOST_PWR_FAULT_Pin GPIO_PIN_1 // PB1 is unused
  
#define NOF_ADC_VALUES               8
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
