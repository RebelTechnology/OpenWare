#include "main.h"
#include "stm32h7xx_hal.h"

#define OWL_XIBECA
#define HARDWARE_ID                  XIBECA_HARDWARE
#define HARDWARE_VERSION             "Xibeca"
/* #define NO_EXTERNAL_RAM */
/* #define NO_CCM_RAM */
#define DMA_RAM                      __attribute__ ((section (".dmadata")))
#define USE_PLUS_RAM
/* #define USE_ICACHE */
/* #define USE_DCACHE */

#define ARM_CYCLES_PER_SAMPLE       (480000000/AUDIO_SAMPLINGRATE) /* 480MHz / 48kHz */
#define MAIN_LOOP_SLEEP_MS          20

#define USE_SCREEN
#define OLED_UPSIDE_DOWN
#define SSD1309
#define SSD1306
#define OLED_SPI hspi3

// todo: quad SPI
/* #define USE_SPI_FLASH */

#define USE_CODEC
#define USE_PCM3168A
/* #define CODEC_HP_FILTER */
#define CODEC_SPI hspi2

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              8
/* #define AUDIO_INT32_TO_SAMPLE(x)    (__REV16((x)>>8)) */
/* #define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(__REV16(x))<<8) */
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)
#define USBD_AUDIO_RX_CHANNELS       4
#define USBD_AUDIO_TX_CHANNELS       4

#define USE_USBD_AUDIO
#define USE_USBD_AUDIO_TX           // microphone
/* #define USE_USBD_AUDIO_RX           // speaker */
#define USE_USBD_FS
#define USBD_HANDLE                 hUsbDeviceFS
/* #define USBD_MAX_POWER              100 // 200mA for iPad compatibility */

/* #define USE_UART_MIDI_RX */
/* #define USE_UART_MIDI_TX */
/* #define UART_MIDI_HANDLE            huart2 */
/* #define UART_MIDI_RX_BUFFER_SIZE    256 */

#define AUDIO_SAMPLINGRATE          48000

#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
  
#define NOF_ADC_VALUES               4
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)
