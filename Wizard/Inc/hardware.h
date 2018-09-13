#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_WIZARD
/* #define FASCINATION_MACHINE */

#define USE_RGB_LED
#define USE_DAC
#define TRIG1_Pin GP7_Pin
#define TRIG1_GPIO_Port GP7_GPIO_Port
#define TRIG2_Pin GP8_Pin
#define TRIG2_GPIO_Port GP8_GPIO_Port
#define USE_ADC
#define ADC_PERIPH hadc3
#define ADC_A 2
#define ADC_B 1
#define ADC_C 0
#define ADC_D 4
#define ADC_E 3
#define USE_CODEC
#define USE_CS4271
#define CODEC_HP_FILTER
#define CODEC_SPI hspi4
#define USE_USBD_HS
#define USE_USB_HOST
#define USBH_HANDLE hUsbHostFS
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */

#define NOF_ADC_VALUES               5
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  5
