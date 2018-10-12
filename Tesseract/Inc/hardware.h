#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"

/* #define OWL_TESSERACT_REV4 // SWO mapped to toggle */

#define OWL_TESSERACT
#define HARDWARE_VERSION             "Tesseract"

#define USE_RGB_LED
#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 1
#define ADC_B 3
#define ADC_C 2
#define ADC_D 0
#define USE_CS4271
#define USE_USBD_HS
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  4
