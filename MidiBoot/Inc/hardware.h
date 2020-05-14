#include "main.h"
#include "stm32f4xx_hal.h"

#define USE_USBD_MIDI
#define USBD_MAX_POWER              100 // 100mA for iPad compatibility
#ifdef USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#else
#define USBD_HANDLE hUsbDeviceHS
#endif

#if defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus Boot"
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_ALCHEMIST
  #define HARDWARE_VERSION    "OWL Alchemist Boot"
  #define USE_BOOT1_PIN
  #define USE_LED
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WIZARD
  #define HARDWARE_VERSION    "OWL Wizard Boot"
  #define USE_BOOT1_PIN
  #define USE_LED
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_NOCTUA
  #define HARDWARE_VERSION    "OWL Noctua Boot"
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WAVETABLE
  #define HARDWARE_VERSION    "OWL WaveTable Boot"
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_PEDAL
  #define HARDWARE_VERSION    "OWL Pedal Boot"
  #define APPLICATION_ADDRESS 0x08008000
  #define USE_BOOT1_PIN
  #define USE_LED
#elif defined OWL_RACK
  #define HARDWARE_VERSION    "OWL Rack Boot"
  #define APPLICATION_ADDRESS 0x08008000
#elif defined OWL_BIOSIGNALS
  #define HARDWARE_VERSION    "OWL BioSignals Boot"
  #define APPLICATION_ADDRESS 0x08010000
#else
  #error Invalid configuration
#endif

/* #define USE_IWDG */
/* #define INIT_FMC */
