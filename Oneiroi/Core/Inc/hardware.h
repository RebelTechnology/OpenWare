#include "main.h"
#include "stm32h7xx_hal.h"

#define OWL_ONEIROI
#define HARDWARE_ID                  ONEIROI_HARDWARE
#define HARDWARE_VERSION             "Oneiroi"
#define USBD_PRODUCT_STRING_FSHS     "OWL-ONEIROI"

#define USE_SPI_FLASH
#define SPI_FLASH_HANDLE             hspi1
#define NO_INTERNAL_FLASH

#define DMA_RAM                      __attribute__ ((section (".dmadata")))
#define USE_PLUS_RAM

#ifndef DEBUG
#define USE_ICACHE
#define USE_DCACHE
#endif

#define ARM_CYCLES_PER_SAMPLE       (480000000/AUDIO_SAMPLINGRATE) /* 480MHz / 48kHz */

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
#define USBD_AUDIO_BUFFER_PACKETS   3
#define AUDIO_SAMPLINGRATE          48000

#define USE_USBD_FS
#define USE_USBH_HS

/* #define USE_DIGITALBUS */
/* #define BUS_HUART huart2 */
/* #define DIGITAL_BUS_ENABLED 1 */
/* #define DIGITAL_BUS_FORWARD_MIDI 0 */

#define USE_BKPSRAM

#define AUDIO_OUTPUT_GAIN            127

#define USE_DAC
#define DAC_HANDLE                   hdac1
#define USE_ADC
#define ADC_PERIPH hadc3
#define MUX_PERIPH hadc1
#define MUX_A 0
#define MUX_B 1
#define MUX_C 2
#define MUX_D 3
#define MUX_E 4
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

#define AUDIO_BLOCK_SIZE             32

#define NOF_ADC_VALUES               7
#define NOF_MUX_VALUES               5
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  15

#define AUDIO_INPUT_OFFSET           ((uint32_t)(-0.0022*65535))
#define AUDIO_INPUT_SCALAR           ((uint32_t)(-10.75*65535))
#define AUDIO_OUTPUT_OFFSET          ((uint32_t)(0.0014*65535))
#define AUDIO_OUTPUT_SCALAR          ((uint32_t)(10.2*65535))
