#include "main.h"
#include "stm32f4xx_hal.h"

#define HARDWARE_VERSION             "OWL MIDI Boot"

#if defined OWL_MAGUS
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_ALCHEMIST || defined OWL_WIZARD
  #define USE_USBD_HS
  #define USE_BOOT1_PIN
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WAVETABLE
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_PEDAL
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08008000
  #define USE_BOOT1_PIN
#elif defined OWL_RACK
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08008000
#else
  #error Invalid configuration
#endif


/* #define INIT_FMC */
