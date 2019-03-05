#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BIOSIGNALS
#define HARDWARE_ID                  ALCHEMIST_HARDWARE
#define HARDWARE_VERSION             "BioSignals"

#define USE_CODEC
#define USE_ADS1294
#define USE_USB_AUDIO

/* #define USE_KX122 */
#define KX122_LEFTSHIFT               16
#define KX122_ACTIVE_CHANNELS         3
/* #define KX122_AUDIO_FREQ              25600 */
#define KX122_AUDIO_FREQ              12800
/* #define KX122_AUDIO_FREQ              6400 */
#define KX122_TOTAL_CHANNELS          3 /* Always 3 */
#define KX122_BLOCKSIZE               1 /* Always 1 */
#define KX122_BUFFER_SIZE             (KX122_TOTAL_CHANNELS*KX122_BLOCKSIZE)
#define KX122_RINGBUFFER_SIZE         (KX122_ACTIVE_CHANNELS*128)
#define KX122_RINGBUFFER_OVER_LIMIT   (2*KX122_RINGBUFFER_SIZE/3)
#define KX122_RINGBUFFER_UNDER_LIMIT  (KX122_RINGBUFFER_SIZE/3)
#define KX122_HSPI                    hspi3

#define ADS_MAX_CHANNELS            4
#define ADS_BLOCKSIZE               CODEC_BLOCKSIZE
#define ADS_HSPI                    hspi1
#define AUDIO_RINGBUFFER_SIZE       (CODEC_BLOCKSIZE*USB_AUDIO_CHANNELS*12)

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       32
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define USB_AUDIO_CHANNELS          4
#define ADC_CHANNEL_OFFSET          0
#define ADC_RIGHTSHIFT              0

#if defined USE_KX122 && USB_AUDIO_CHANNELS != (ADS_MAX_CHANNELS + KX122_ACTIVE_CHANNELS)
#error "Invalid channel configuration"
#elif USB_AUDIO_CHANNELS != ADS_MAX_CHANNELS
#error "Invalid channel configuration"
#endif

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
#define NOF_PARAMETERS               5
#define NOF_BUTTONS                  (2+1)
