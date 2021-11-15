#include "main.h"
#include "stm32h7xx_hal.h"
#include "hardware_ids.h"

#define USE_BOOTLOADER_MODE
#define USE_USBD_MIDI
#ifdef USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#else
#define USBD_HANDLE hUsbDeviceHS
#endif

#if defined OWL_GENIUS
  #define HARDWARE_VERSION    "OWL Genius Boot"
  #define HARDWARE_ID         GENIUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
#elif defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus Boot"
  #define HARDWARE_ID         MAGUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
#else
  #error Invalid configuration
#endif
