#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"

/* #define OWL_TESSERACT_REV4 // SWO mapped to toggle */

#define OWL_WAVETABLE
#define HARDWARE_ID                  WAVETABLE_HARDWARE
#define HARDWARE_VERSION             "Wavetable Prototype"

#define USE_RGB_LED
#define USE_ADC
#define ADC_A 5
#define ADC_B 0
#define ADC_C 3
#define ADC_D 1
#define ADC_E 6
#define ADC_F 4
#define ADC_G 2
#define ADC_H 7
#define USE_CS4271
#define USE_USBD_FS
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

#define NOF_ADC_VALUES               8
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  4
