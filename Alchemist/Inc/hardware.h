#include "main.h"
#include "stm32f4xx_hal.h"

/* #define OWL_ALCHEMIST_LED */
#define OWL_ALCHEMIST
#define HARDWARE_ID                  ALCHEMIST_HARDWARE
#define HARDWARE_VERSION             "Alchemist"

#define USE_MODE_BUTTON
#define MODE_BUTTON_PIN SW3_Pin
#define MODE_BUTTON_PORT SW3_GPIO_Port
#define MODE_BUTTON_GAIN ADC_C
#define MODE_BUTTON_PATCH ADC_D

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
