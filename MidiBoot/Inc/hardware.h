#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BOOT
#define HARDWARE_VERSION             "OWL MIDI Boot"

/* #define OWL_MAGUS */

#ifdef OWL_MAGUS
/* Magus */
#define USE_USBD_FS
#undef USE_BOOT1_PIN
#else
/* Alchemist and Wizard */
#define USE_USBD_HS
#define USE_BOOT1_PIN
#endif

#define APPLICATION_ADDRESS 0x08010000

/* #define INIT_FMC */
