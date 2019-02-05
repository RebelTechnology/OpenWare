#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BIOSIGNALS
#define HARDWARE_ID                  ALCHEMIST_HARDWARE
#define HARDWARE_VERSION             "BioSignals"

#define USE_CODEC
#define USE_ADS1294
#define USE_USB_AUDIO

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE       (AUDIO_BITS_PER_SAMPLE/8)
#define USB_AUDIO_CHANNELS 4
#define ADC_CHANNEL_OFFSET 0
#define ADC_RIGHTSHIFT 16

/* #define USE_RGB_LED */
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
#define NOF_BUTTONS                  (2+1)
