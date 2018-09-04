#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"

#define OWL_QUADFM

#define ADC_A 1
#define ADC_B 3
#define ADC_C 2
#define ADC_D 0
#define PUSHBUTTON_Pin GP7_Pin
#define PUSHBUTTON_GPIO_Port GP7_GPIO_Port
#define USE_CS4271
#define USE_USBD_HS
#define OWL2

#define CS_CS_Pin CS_nCS_Pin
#define CS_CS_GPIO_Port CS_nCS_GPIO_Port
#define CS_RST_Pin CS_nRST_Pin
#define CS_RST_GPIO_Port CS_nRST_GPIO_Port

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  4
