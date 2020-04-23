#include "main.h"
#include "stm32f4xx_hal.h"

#ifndef OWLBOOT_MAGIC_NUMBER
#define OWLBOOT_MAGIC_NUMBER        (0xDADAB007)
#endif

#ifndef OWLBOOT_MAGIC_ADDRESS
#define OWLBOOT_MAGIC_ADDRESS       ((uint32_t*)0x2000FFF0)
#endif

#if defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus Boot"
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_ALCHEMIST
  #define HARDWARE_VERSION    "OWL Alchemist Boot"
  #define USE_BOOT1_PIN
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WIZARD
  #define HARDWARE_VERSION    "OWL Wizard Boot"
  #define USE_BOOT1_PIN
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_NOCTUA
  #define HARDWARE_VERSION    "OWL Noctua Boot"
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WAVETABLE
  #define HARDWARE_VERSION    "OWL WaveTable Boot"
  #define USE_USBD_FS
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_PEDAL
  #define HARDWARE_VERSION    "OWL Pedal Boot"
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08008000
  #define USE_BOOT1_PIN
#elif defined OWL_RACK
  #define HARDWARE_VERSION    "OWL Rack Boot"
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08008000
#elif defined OWL_BIOSIGNALS
  #define HARDWARE_VERSION    "OWL BioSignals Boot"
  #define USE_USBD_HS
  #define APPLICATION_ADDRESS 0x08010000
#else
  #error Invalid configuration
#endif


/* #define INIT_FMC */
