#include "main.h"
#include "stm32f4xx_hal.h"

#if defined OWL_RACK
#define USE_DIGITALBUS
#define BUS_HUART huart4
#define HARDWARE_ID                  OWL_RACK_HARDWARE
#define HARDWARE_VERSION             "OWL Rack"
#elif defined OWL_MODULAR
#define HARDWARE_ID                  OWL_MODULAR_HARDWARE
#define HARDWARE_VERSION             "OWL Modular"
#define AUDIO_INPUT_GAIN             92
#define AUDIO_OUTPUT_GAIN            121
#elif defined OWL_PEDAL
#define HARDWARE_ID                  OWL_PEDAL_HARDWARE
#define HARDWARE_VERSION             "OWL Pedal"
#define AUDIO_INPUT_GAIN             108
#define AUDIO_OUTPUT_GAIN            115
#define OWL_PEDAL_LEGACY
#endif
#define MAX_SYSEX_BOOTLOADER_SIZE    (32 * 1024)


#define EXPRESSION_MODE_EXP_TRS      1 // TRS expression pedal
#define EXPRESSION_MODE_EXP_RTS      2 // TRS expression pedal
#define EXPRESSION_MODE_FS_TS        3 // single footswitch
#define EXPRESSION_MODE_FS_TRS       4 // dual footswitch
#define EXPRESSION_MODE_EXP_ITRS     5 // inverted TRS expression pedal
#define EXPRESSION_MODE_EXP_IRTS     6 // inverted RTS expression pedal
#define EXPRESSION_MODE              EXPRESSION_MODE_EXP_TRS

#define OWLBOOT_MAGIC_NUMBER         0xF00B4400

#define USE_CODEC
#define USE_WM8731

#define USE_USBD_FS
#define USBD_DESC FS_Desc
#define USBD_HSFS DEVICE_FS
#define USBD_HANDLE hUsbDeviceFS
#define USBD_PCD_HANDLE hpcd_USB_OTG_FS

/* USB audio settings */
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define USBD_AUDIO_RX_CHANNELS      AUDIO_CHANNELS
#define USBD_AUDIO_TX_CHANNELS      AUDIO_CHANNELS
#define AUDIO_INT32_TO_SAMPLE(x)    (x & 0xffff)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x))

#define USE_USBD_AUDIO
#define USE_USBD_RX_FB
#define USE_USBD_AUDIO_FEATURES
#define USE_USBD_AUDIO_RX // speaker
#define USE_USBD_AUDIO_TX  // microphone

/* #define USE_BKPSRAM */

#ifdef OWL_RACK
#define NOF_ADC_VALUES               0
#else
#define USE_ADC
#define NOF_ADC_VALUES               6
#define ADC_PERIPH hadc3
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#define ADC_E 5
#define ADC_F 4
#endif

#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4 + 1)
