#include "main.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_flash_ex.h"

#define OWL_PLAYERF7

#define HARDWARE_ID                  PLAYER_HARDWARE
#define HARDWARE_VERSION             "Genius"

#define OWL_ARCH_F7
#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
/* #define PUSHBUTTON_Pin GP7_Pin */
/* #define PUSHBUTTON_GPIO_Port GP7_GPIO_Port */
#define USE_USBD_HS
#define USE_USB_HOST
#define USB_HOST_RX_BUFF_SIZE   64  /* Max Received data 64 bytes */
#define OWL2
#define USE_SCREEN
#define SSD1309
#define OLED_DMA
#define OLED_SOFT_CS
#define OLED_SPI hspi2
/* #define OLED_IT */
/* #define OLED_BITBANG */
#define USE_ENCODERS
#define ENCODER_TIM1 htim2
#define ENCODER_TIM2 htim3
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               16
#define NOF_BUTTONS                  5
