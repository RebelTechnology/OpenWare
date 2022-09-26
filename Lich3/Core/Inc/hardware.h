#include "main.h"
#include "stm32h7xx_hal.h"

#define OWL_LICH
#define HARDWARE_ID                  LICH_HARDWARE
#define HARDWARE_VERSION             "Lich"
#define USBD_PRODUCT_STRING_FSHS     "OWL-LICH"

#define USE_SPI_FLASH
#define SPI_FLASH_HANDLE             hspi1

#define DMA_RAM                      __attribute__ ((section (".dmadata")))
#define USE_PLUS_RAM

#ifndef DEBUG
#define USE_ICACHE
#define USE_DCACHE
#endif

#define MAX_SYSEX_PROGRAM_SIZE      (512*1024)

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define USBD_AUDIO_RX_CHANNELS      AUDIO_CHANNELS
#define USBD_AUDIO_TX_CHANNELS      AUDIO_CHANNELS
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_USBD_AUDIO
#define USE_USBD_RX_FB
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_AUDIO_TX // microphone

#define USE_USBD_FS
#define USE_USBH_HS

/* #define USE_DIGITALBUS */
/* #define BUS_HUART huart2 */
/* #define DIGITAL_BUS_ENABLED 1 */
/* #define DIGITAL_BUS_FORWARD_MIDI 0 */

/* #define USE_BKPSRAM */

#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define AUDIO_OUTPUT_GAIN            127

#define USE_DAC
#define DAC_HANDLE                   hdac1
#define TRIG1_Pin GP7_Pin 	/* A17 */
#define TRIG1_GPIO_Port GP7_GPIO_Port
#define TRIG2_Pin GP8_Pin	/* A14 */
#define TRIG2_GPIO_Port GP8_GPIO_Port
#define USE_ADC
#define ADC_PERIPH hadc3
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4+2)

#define AUDIO_INPUT_OFFSET           ((uint32_t)(-0.0022*65535))
#define AUDIO_INPUT_SCALAR           ((uint32_t)(-10.75*65535))
#define AUDIO_OUTPUT_OFFSET          ((uint32_t)(0.0014*65535))
#define AUDIO_OUTPUT_SCALAR          ((uint32_t)(10.2*65535))
