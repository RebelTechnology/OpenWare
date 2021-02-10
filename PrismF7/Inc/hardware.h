#include "main.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_flash_ex.h"

#define OWL_PRISMF7
#define HARDWARE_ID                  PRISM_HARDWARE
#define HARDWARE_VERSION             "PrismF7"

#define NO_EXTERNAL_RAM
#define NO_CCM_RAM

#define OWL_ARCH_F7
#define USE_USBD_HS
#define OWL2
#define USE_SCREEN
#define SEPS114A
#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3

/* #define OLED_DMA */

/* #define OLED_IT */
/* #define OLED_BITBANG */
#define OLED_SOFT_CS
#define OLED_SPI hspi1
#define USE_ENCODERS
#define ENCODER_TIM1 htim1
#define ENCODER_TIM2 htim3
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi2
/* #define OVERRIDE_CODEC_CONFIG */

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               16
#define NOF_BUTTONS                  5
