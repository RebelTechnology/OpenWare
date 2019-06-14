#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_NOCTUA
#define HARDWARE_ID                  NOCTUA_HARDWARE
#define HARDWARE_VERSION             "Noctua"

#define USBD_AUDIO_FREQ 48000
#define USB_AUDIO_CHANNELS 4

/* #define USE_MODE_BUTTON */
/* #define MODE_BUTTON_PIN SW3_Pin */
/* #define MODE_BUTTON_PORT SW3_GPIO_Port */
/* #define MODE_BUTTON_GAIN ADC_C */
/* #define MODE_BUTTON_PATCH ADC_D */

/* #define USE_BKPSRAM enable to prevent reset loops */ 

/* #define USE_RGB_LED */
#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define USE_CODEC
#define USE_PCM3168A
#define CODEC_HP_FILTER
#define CODEC_SPI hspi2
#define USE_USBD_FS

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
