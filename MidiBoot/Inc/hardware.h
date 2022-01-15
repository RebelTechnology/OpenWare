#include "main.h"
#include "stm32f4xx_hal.h"
#include "hardware_ids.h"

#define USE_BOOTLOADER_MODE
#define USE_USBD_MIDI
#ifdef USE_USBD_FS
#define USBD_HANDLE hUsbDeviceFS
#else
#define USBD_HANDLE hUsbDeviceHS
#endif

#if defined OWL_MAGUS
  #define HARDWARE_VERSION    "OWL Magus Boot"
  #define HARDWARE_ID         MAGUS_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_SPI_FLASH
#elif defined OWL_LICH
  #define HARDWARE_VERSION    "OWL Lich Boot"
  #define HARDWARE_ID         LICH_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_BOOT1_PIN
  #define BOOT1_GPIO_Port GPIOC
  #define BOOT1_Pin  GPIO_PIN_10
  #define USE_LED
  #define LED1_Pin GPIO_PIN_7
  #define LED1_GPIO_Port GPIOF
  #define LED2_Pin GPIO_PIN_8
  #define LED2_GPIO_Port GPIOF
  #define USE_SPI_FLASH
#elif defined OWL_WITCH
  #define HARDWARE_VERSION    "OWL Witch Boot"
  #define HARDWARE_ID         WITCH_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_9
  #define BOOT1_GPIO_Port GPIOF
  #define USE_LED
  #define LED1_Pin GPIO_PIN_9
  #define LED1_GPIO_Port GPIOC
  #define LED2_Pin GPIO_PIN_8
  #define LED2_GPIO_Port GPIOA
  #define USE_SPI_FLASH
#elif defined OWL_ALCHEMIST
  #define HARDWARE_VERSION    "OWL Alchemist Boot"
  #define HARDWARE_ID         ALCHEMIST_HARDWARE
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_2
  #define BOOT1_GPIO_Port GPIOB
  #define USE_LED
  #define LED1_Pin GPIO_PIN_0
  #define LED1_GPIO_Port GPIOA
  #define LED2_Pin GPIO_PIN_1
  #define LED2_GPIO_Port GPIOA
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_SPI_FLASH
#elif defined OWL_WIZARD
  #define HARDWARE_VERSION    "OWL Wizard Boot"
  #define HARDWARE_ID         WIZARD_HARDWARE
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_2
  #define BOOT1_GPIO_Port GPIOB
  #define USE_LED
  #define LED1_Pin GPIO_PIN_0
  #define LED1_GPIO_Port GPIOA
  #define LED2_Pin GPIO_PIN_1
  #define LED2_GPIO_Port GPIOA
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_SPI_FLASH
#elif defined OWL_PEDAL
  #define HARDWARE_VERSION    "OWL Pedal Boot"
  #define HARDWARE_ID         OWL_PEDAL_HARDWARE
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_10 // only valid for green/Rev04
  #define BOOT1_GPIO_Port GPIOG
  #define USE_LED
  #define LED1_Pin GPIO_PIN_8
  #define LED1_GPIO_Port GPIOB
  #define LED2_Pin GPIO_PIN_9
  #define LED2_GPIO_Port GPIOB
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_SPI_FLASH
#elif defined OWL_NOCTUA
  #define HARDWARE_VERSION    "OWL Noctua Boot"
  #define HARDWARE_ID         NOCTUA_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WAVETABLE
  #define HARDWARE_VERSION    "OWL WaveTable Boot"
  #define HARDWARE_ID         WAVETABLE_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_BIOSIGNALS
  #define HARDWARE_VERSION    "OWL BioSignals Boot"
  #define HARDWARE_ID         BIOSIGNALS_HARDWARE
  #define APPLICATION_ADDRESS 0x08010000
#else
  #error Invalid configuration
#endif
