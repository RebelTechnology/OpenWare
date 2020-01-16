#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BIOSIGNALS
#define HARDWARE_ID                  BIOSIGNALS_HARDWARE
#define HARDWARE_VERSION             "AccelerometerTest"

#define USE_BLE_MIDI
#define BLE_SPI hspi2
#define USE_BKPSRAM
#define USE_LED
/* #define USE_LED_PWM */

#define LED_RED_Pin RGB_3_Pin
#define LED_RED_GPIO_Port RGB_3_GPIO_Port
#define LED_GREEN_Pin RGB_1_Pin
#define LED_GREEN_GPIO_Port RGB_1_GPIO_Port

#define USE_CODEC
#define USE_IIS3DWB
#define USE_USB_AUDIO
#define USE_USBD_AUDIO_IN // microphone
#define USE_USBD_MIDI
/* #define USE_USBD_AUDIO_OUT // speaker */
#define USBD_HANDLE hUsbDeviceHS
/* #define AUDIO_BYPASS */

#define IIS_AUDIO_FREQ              8000
#define ADS_GAIN                    4
#define ADS_MAX_CHANNELS            4
#define ADS_ACTIVE_CHANNELS         4
#define ADS_HSPI                    hspi1

#define ADS_AUDIO_FREQ              8000
#define ADS_GAIN                    4
#define ADS_MAX_CHANNELS            4
#define ADS_ACTIVE_CHANNELS         4
#define ADS_HSPI                    hspi1

#define AUDIO_RINGBUFFER_SIZE       (CODEC_BLOCKSIZE*USB_AUDIO_CHANNELS*4)

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       32
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_SAMPLINGRATE          5000

#define TIM8_PERIOD                 (871*48000/AUDIO_SAMPLINGRATE) /* experimentally determined */

#define USB_AUDIO_CHANNELS           3
#define AUDIO_CHANNELS               USB_AUDIO_CHANNELS

#define NOF_ADC_VALUES               0
#define NOF_PARAMETERS               5
#define NOF_BUTTONS                  (2+1)
