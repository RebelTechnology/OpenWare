#include "main.h"
#include "stm32f4xx_hal.h"
#include "hardware_ids.h"

#define USE_BOOTLOADER_MODE
#define USBD_PRODUCT_STRING_FSHS     "OWL-BOOT"
#define USE_USBD_MIDI
#define AUDIO_BITS_PER_SAMPLE       16
#define MAX_SYSEX_FIRMWARE_SIZE (352 * 1024)

#if defined OWL_MODULAR
  #define HARDWARE_VERSION    "OWL Modular Boot"
  #define HARDWARE_ID         OWL_MODULAR_HARDWARE
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08008000
  #define USE_BOOT1_PIN
  #define BOOT1_Pin           GPIO_PIN_2
  #define BOOT1_GPIO_Port     GPIOE
#elif defined OWL_PEDAL
  #define HARDWARE_VERSION    "OWL Pedal Boot"
  #define HARDWARE_ID         OWL_PEDAL_HARDWARE
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08008000
  #define USE_BOOT1_PIN
  #define BOOT1_Pin           GPIO_PIN_2
  #define BOOT1_GPIO_Port     GPIOE
#elif defined OWL_RACK
  #define HARDWARE_VERSION    "OWL Rack Boot"
  #define HARDWARE_ID         OWL_RACK_HARDWARE
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08008000
#else
  #error Invalid configuration
#endif

#define OWLBOOT_MAGIC_NUMBER  0xF00B4400

/* #define USE_IWDG */
/* #define INIT_FMC */
