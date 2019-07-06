#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BIOSIGNALS
#define HARDWARE_ID                  ALCHEMIST_HARDWARE
#define HARDWARE_VERSION             "BioSignals"

#define USE_CODEC
#define USE_ADS1294
#define USE_USB_AUDIO
#define USE_USBD_MIDI
#define USE_USBD_AUDIO_IN // microphone
#define AUDIO_BYPASS

/* #define USE_KX122 */
#define KX122_ACTIVE_CHANNELS         3
/* #define KX122_AUDIO_FREQ              25600 */
/* #define KX122_AUDIO_FREQ              12800 */
#define KX122_AUDIO_FREQ              6400
#define KX122_TOTAL_CHANNELS          3 /* Always 3 */
#define KX122_HSPI                    hspi3

#define ADS_AUDIO_FREQ              8000
#define ADS_GAIN                    4
#define ADS_MAX_CHANNELS            4
#define ADS_ACTIVE_CHANNELS         4
#define ADS_HSPI                    hspi1

#define AUDIO_RINGBUFFER_SIZE       (CODEC_BLOCKSIZE*USB_AUDIO_CHANNELS*4)

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       32
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define USBD_AUDIO_FREQ             8000

#define TIM8_PERIOD                 (871*48000/USBD_AUDIO_FREQ) /* experimentally determined */

#if defined USE_KX122 && defined USE_CODEC
#define USB_AUDIO_CHANNELS          (ADS_ACTIVE_CHANNELS + KX122_ACTIVE_CHANNELS)
#elif defined USE_CODEC
#define USB_AUDIO_CHANNELS          ADS_ACTIVE_CHANNELS
#elif defined USE_KX122
#define USB_AUDIO_CHANNELS          KX122_ACTIVE_CHANNELS
#else
#error "Invalid configuration"
#endif

/* #define USE_LED */

#define USE_USBD_HS

#define NOF_ADC_VALUES               0
#define NOF_PARAMETERS               5
#define NOF_BUTTONS                  (2+1)
