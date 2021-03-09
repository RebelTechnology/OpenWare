#include "main.h"
#include "stm32h7xx_hal.h"
#include "hardware_ids.h"

#define USE_BOOTLOADER_MODE
#define USE_USBD_MIDI
#define USBD_MAX_POWER              100 // 100mA for iPad compatibility
#ifdef USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#else
#define USBD_HANDLE hUsbDeviceHS
#endif

#if defined OWL_GENIUS
  #define HARDWARE_VERSION    "OWL Genius Boot"
  #define HARDWARE_ID         GENIUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
#else
  #error Invalid configuration
#endif

/* #define USE_IWDG */
/* #define INIT_FMC */
