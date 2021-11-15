#include "main.h"
#include "stm32h7xx_hal.h"

#define OWL_GENIUS
#define HARDWARE_ID                  GENIUS_HARDWARE
#define HARDWARE_VERSION             "Genius"

#define USE_SPI_FLASH
#define SPI_FLASH_HSPI               hspi5

/* #define USE_USBH_HID */

/* #define NO_EXTERNAL_RAM */
/* #define NO_CCM_RAM */
#define DMA_RAM                      __attribute__ ((section (".dmadata")))

#define USE_PLUS_RAM

#ifndef DEBUG
#define USE_ICACHE
#define USE_DCACHE
#endif

/* #define USE_BKPSRAM */

#define MAX_SYSEX_PROGRAM_SIZE      (512*1024)

#define hdac hdac1

#define USE_SCREEN
#define SSD1309
#define OLED_DMA

#define ENCODER_TIM1 htim3
#define ENCODER_TIM2 htim2

/* #define OLED_DMA */
#define OLED_SOFT_CS
#define OLED_SPI hspi2
#define OLED_UPSIDE_DOWN
#define USE_CODEC
#define USE_CS4271
#define CODEC_SPI hspi4

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define AUDIO_OUTPUT_GAIN           121 /* -6dB */
#define USBD_AUDIO_RX_CHANNELS      AUDIO_CHANNELS
#define USBD_AUDIO_TX_CHANNELS      AUDIO_CHANNELS
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#define USE_USBD_AUDIO
#define USE_USBD_RX_FB
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_AUDIO_TX  // microphone
#define USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#define USBH_HANDLE hUsbHostHS
#define USE_USB_HOST
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */
#define USE_USBH_MIDI


#define ARM_CYCLES_PER_SAMPLE       (480000000/AUDIO_SAMPLINGRATE) /* 480MHz / 48kHz */

// Serial MIDI
#define USE_UART_MIDI_RX
#define USE_UART_MIDI_TX
#define UART_MIDI_HANDLE huart5
#define UART_MIDI_RX_BUFFER_SIZE 256

// Digital bus
/* #define USE_DIGITALBUS */
/* #define DIGITAL_BUS_ENABLED 1 */
/* #define DIGITAL_BUS_FORWARD_MIDI 0 */
/* #define BUS_HUART huart2 */

#define USE_ADC
#define ADC_PERIPH hadc1
#define ADC_A 0
#define ADC_B 1
#define NOF_ADC_VALUES               2
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4+2)
#define USE_DAC

