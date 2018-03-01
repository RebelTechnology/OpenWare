#include "main.h"
#include "stm32f4xx_hal.h"

#define OWL_BOOT

#define USE_USBD_HS

#define EEPROM_PAGE_BEGIN            ((uint32_t)0x08060000)
#define EEPROM_PAGE_SIZE             (128*1024)
#define EEPROM_PAGE_END              ((uint32_t)0x08100000)
#define STORAGE_MAX_BLOCKS           64

#define MAX_SYSEX_FIRMWARE_SIZE      ((16+16+64+128+128)*1024) // FLASH sectors 2-6
#define MAX_SYSEX_PROGRAM_SIZE       (128*1024) // 128k, one flash sector
#define MAX_USER_PATCHES             4
#define MAX_NUMBER_OF_PATCHES        40
#define MAX_NUMBER_OF_RESOURCES      12

#define CCMRAM                      ((uint32_t)0x10000000)
/* #define PATCHRAM                    ((uint32_t)0x20040000) */
#define PATCHRAM                    ((uint32_t)0x2000c000)
/* #define EXTRAM                      ((uint32_t)0x68000000) */
#define EXTRAM                      ((uint32_t)0xD0000000)
