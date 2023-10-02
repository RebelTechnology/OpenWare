#include "main.h"
#include "stm32h7xx_hal.h"
#include "hardware_ids.h"

#define USE_BOOTLOADER_MODE
#define USBD_PRODUCT_STRING_FSHS "OWL-BOOT"

#define AUDIO_BITS_PER_SAMPLE       16
#define NO_INTERNAL_FLASH

#if defined OWL_GENIUS
  #define HARDWARE_VERSION    "OWL Genius Boot"
  #define HARDWARE_ID         GENIUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_BOOT1_PIN // SW_A (Button 1): PC10
  #define BOOT1_Pin GPIO_PIN_10
  #define BOOT1_GPIO_Port GPIOC
  #define USE_SPI_FLASH
  #define SPI_FLASH_HANDLE             hspi5
#elif defined OWL_LICH
  #define HARDWARE_VERSION    "OWL Lich3 Boot"
  #define HARDWARE_ID         LICH_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_BOOT1_PIN
  #define BOOT1_GPIO_Port GPIOC
  #define BOOT1_Pin  GPIO_PIN_10
  #define USE_LED
  #define LED1_Pin GPIO_PIN_7
  #define LED1_GPIO_Port GPIOF
  #define LED2_Pin GPIO_PIN_8
  #define LED2_GPIO_Port GPIOF
  #define USE_SPI_FLASH
  #define SPI_FLASH_HANDLE             hspi1
#elif defined OWL_WITCH
  #define HARDWARE_VERSION    "OWL Witch3 Boot"
  #define HARDWARE_ID         WITCH_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_9
  #define BOOT1_GPIO_Port GPIOF
  #define USE_LED
  #define LED1_Pin GPIO_PIN_9
  #define LED1_GPIO_Port GPIOC
  #define LED2_Pin GPIO_PIN_8
  #define LED2_GPIO_Port GPIOA
  #define USE_SPI_FLASH
  #define SPI_FLASH_HANDLE             hspi1
#elif defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus3 Boot"
  #define HARDWARE_ID         MAGUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_SPI_FLASH
#elif defined OWL_XIBECA
  #define HARDWARE_VERSION    "OWL Xibeca Boot"
  #define HARDWARE_ID         XIBECA_HARDWARE
  #define APPLICATION_ADDRESS 0x08020000
  #define USE_DFU_BOOTLOADER
  #define USE_QSPI_FLASH
#else
  #error Invalid configuration
#endif
