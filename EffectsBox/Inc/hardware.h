#include "main.h"
#include "stm32f4xx_hal.h"

/* #define OWL_MICROLAB_LED */
/* #define OWL_MICROLAB */

#define OWL_EFFECTSBOX
#define HARDWARE_ID                  EFFECTSBOX_HARDWARE
#define HARDWARE_VERSION             "FX Box"

/* #define USE_RGB_LED */

#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 2
#define ADC_B 3
#define ADC_C 0
#define ADC_D 1
#define LED1 1
#define LED2 2
#define LED3 3
#define LED4 4
#define USE_CODEC
#define USE_CS4271
#define CODEC_HP_FILTER
#define CODEC_SPI hspi4
#define USE_USBD_HS
#define USE_SCREEN
#define SSD1309
#define OLED_SPI hspi5
/* #define OLED_DMA */
#define OLED_SOFT_CS

#define USE_ENCODERS
#define ENCODER_TIM1 htim4
#define ENCODER_TIM2 htim2
#define NOF_ENCODERS                 2
#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               20
#define NOF_BUTTONS                  8
