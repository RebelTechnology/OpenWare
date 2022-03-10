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
  #define USE_BOOT1_PIN // SW_A (Button 1): PC10
  #define BOOT1_Pin GPIO_PIN_10
  #define BOOT1_GPIO_Port GPIOC
  #define USE_SPI_FLASH
  #define NO_INTERNAL_FLASH
#elif defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus Boot"
  #define HARDWARE_ID         MAGUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_SPI_FLASH
#elif defined OWL_XIBECA
  #define HARDWARE_VERSION    "OWL Xibeca Boot"
  #define HARDWARE_ID         XIBECA_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_DFU_BOOTLOADER
  #define USE_QSPI_FLASH
  #define NO_INTERNAL_FLASH
#else
  #error Invalid configuration
#endif
