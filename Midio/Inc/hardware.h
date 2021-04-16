#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_MIDIO
#define HARDWARE_ID                  MIDIO_HARDWARE
#define HARDWARE_VERSION             "Midio"

#define NO_EXTERNAL_RAM
#define NO_CCM_RAM

/* #define USE_SCREEN */
#define SSD1309
#define OLED_SOFT_CS
#define OLED_SPI hspi1
/* #define OLED_DMA */
/* #define OLED_IT */
/* #define OLED_BITBANG */

#define MAX11300_SPI hspi2

#define ENCODER_TIM1 htim2


/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)
#define USE_USBD_AUDIO
/* #define USE_USBD_AUDIO_TX  // microphone */
/* #define USE_USBD_AUDIO_RX // speaker */
#define USBD_HANDLE hUsbDeviceHS

#define USBH_HANDLE hUsbHostFS
#define USE_USB_HOST
#define USB_HOST_RX_BUFF_SIZE 256  /* Max Received data 64 bytes */
#define USE_USBH_MIDI

/* #define USE_BKPSRAM */

/* #define USE_ADC */
/* #define ADC_PERIPH hadc3 */
/* #define ADC_A 2 */
/* #define ADC_B 3 */
/* #define ADC_C 0 */
/* #define ADC_D 1 */
/* #define USE_CODEC */
/* #define USE_CS4271 */
/* #define CODEC_HP_FILTER */
/* #define CODEC_SPI hspi4 */
#define USE_USBD_HS

#define NOF_ADC_VALUES               0
#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (2+4)