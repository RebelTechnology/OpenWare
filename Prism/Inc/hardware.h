#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_PRISM
#define OWL_RACK

#define USE_DIGITALBUS

#define USE_USBD_HS
#define USE_SCREEN
#define SEPS114A
#define OLED_DMA
/* #define OLED_IT */
/* #define OLED_BITBANG */
#define OLED_SOFT_CS
#define OLED_SPI hspi1
#define USE_ENCODERS
#define ENCODER_TIM1 htim3
#define ENCODER_TIM2 htim1
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi2
#define BUS_HUART huart1

#ifdef OWL_RACK
#define NOF_ADC_VALUES               0
#else
#define NOF_ADC_VALUES               2
#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#endif

#define NOF_ENCODERS                 2
#define NOF_BUTTONS                  5
#define NOF_PARAMETERS               40
