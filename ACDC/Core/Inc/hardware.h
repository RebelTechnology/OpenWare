#include "main.h"
#include "stm32h7xx_hal.h"

#define OWL_XIBECA
#define HARDWARE_ID                  XIBECA_HARDWARE
#define HARDWARE_VERSION             "AC/DC"
#define USBD_PRODUCT_STRING_FSHS     "OWL-ACDC"
#define USBD_SELF_POWERED            1U

/* #define NO_EXTERNAL_RAM */
/* #define NO_CCM_RAM */
#define DMA_RAM                      __attribute__ ((section (".dmadata")))

/* note that with H750, PLUS_RAM may be used by firmware */
#define USE_PLUS_RAM

#define USE_FAST_POW_RESOURCES

#ifdef NDEBUG
#define USE_ICACHE
#define USE_DCACHE
#endif

#define ARM_CYCLES_PER_SAMPLE       (480000000/AUDIO_SAMPLINGRATE) /* 480MHz / 48kHz */

#define USE_QSPI_FLASH
#define NO_INTERNAL_FLASH
#define USE_DFU_BOOTLOADER

// 84 too low, 92 too high
#define AUDIO_INPUT_GAIN            88 // half decibel steps
// 106 too low 114 too high
#define AUDIO_OUTPUT_GAIN           114 // half decibel steps

/* 88 / 114 : 10Vpp in, 5.3/4.99Vpp out */
/* 88 / 112 : 10Vpp in, 4.7/4.4Vpp out */
/* 84 / 106 : 10Vpp in, 2.6/2.4Vpp out */

/* -#define AUDIO_INPUT_GAIN            104 */
/* -#define AUDIO_OUTPUT_GAIN           114 */

#define USE_CODEC
#define USE_PCM3168A
#define CODEC_ADC_INVERT
#define CODEC_DAC_INVERT
/* #define CODEC_HP_FILTER */
#define CODEC_SPI hspi2

#define CODEC_BLOCKSIZE              256
#define AUDIO_BLOCK_SIZE             8

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              4
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_USBD_AUDIO
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_TX           // microphone
#define USE_USBD_AUDIO_RX           // speaker
#define USE_USBD_RX_FB
#define USE_USBD_FS
#define USBD_AUDIO_TX_CHANNELS      4
#define USBD_AUDIO_RX_CHANNELS      4
#define USBD_AUDIO_BUFFER_PACKETS   3

#define AUDIO_SAMPLINGRATE          48000

#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
  
#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  16
