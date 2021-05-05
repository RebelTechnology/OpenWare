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
#endif
#define OWLBOOT_MAGIC                (0xF00B4400)
#define MAX_SYSEX_BOOTLOADER_SIZE    (32 * 1024)

#define USE_CODEC
#define USE_WM8731
#define USE_USBD_FS
#define USE_USBD_AUDIO
#define USE_USBD_AUDIO_TX  // microphone
/* #define USE_USBD_AUDIO_RX  // speaker */
#define USBD_HANDLE hUsbDeviceFS
#define AUDIO_BITS_PER_SAMPLE       16
#define AUDIO_BYTES_PER_SAMPLE      (AUDIO_BITS_PER_SAMPLE/8)
#define AUDIO_CHANNELS              2
#define AUDIO_INT32_TO_SAMPLE(x)    ((x)>>8)
#define AUDIO_SAMPLE_TO_INT32(x)    ((int32_t)(x)<<8)

#ifdef OWL_RACK
#define NOF_ADC_VALUES               0
#else
#define USE_ADC
#define NOF_ADC_VALUES               5
#define ADC_PERIPH hadc3
#define ADC_A 0
#define ADC_B 1
#define ADC_C 2
#define ADC_D 3
#endif

#define NOF_PARAMETERS               40
#define NOF_BUTTONS                  (4 + 1)

#if 0
#define MIDI_INPUT_CHANNEL           MIDI_OMNI_CHANNEL
#define MIDI_OUTPUT_CHANNEL          0

#define DIGITAL_BUS_ENABLED          0
#define DIGITAL_BUS_FORWARD_MIDI     1

/* +0db in and out */
#define AUDIO_INPUT_GAIN             0x017
#define AUDIO_OUTPUT_GAIN            0x079
#define AUDIO_INPUT_OFFSET           0xffffefaa /* -0.06382 * 65535 */
#define AUDIO_INPUT_SCALAR           0xfffbb5c7 /* -4.290 * 65535 */
#define AUDIO_OUTPUT_OFFSET          0x00001eec /* 0.1208 * 65535 */
#define AUDIO_OUTPUT_SCALAR          0xfffb5bab /* -4.642 * 65535 */
#define DEFAULT_PROGRAM              1
#define BUTTON_PROGRAM_CHANGE
#define AUDIO_BIGEND
/* #define AUDIO_SATURATE_SAMPLES // SATURATE adds almost 500 cycles to 24-bit mode */
#define AUDIO_PROTOCOL               I2S_PROTOCOL_PHILIPS
#define AUDIO_BITDEPTH               24    /* bits per sample */
#define AUDIO_DATAFORMAT             24
#define AUDIO_CODEC_MASTER           true
#define AUDIO_CHANNELS               2
#define AUDIO_SAMPLINGRATE           48000

#endif
