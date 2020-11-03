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
#elif defined OWL_LICH
  #define HARDWARE_VERSION    "OWL Lich Boot"
  #define APPLICATION_ADDRESS 0x08010000
  #define USE_BOOT1_PIN
  #define BOOT1_GPIO_Port GPIOC
  #define BOOT1_Pin  GPIO_PIN_10
  #define USE_LED
  #define LED1_Pin GPIO_PIN_7
  #define LED1_GPIO_Port GPIOF
  #define LED2_Pin GPIO_PIN_8
  #define LED2_GPIO_Port GPIOF
#elif defined OWL_WITCH
  #define HARDWARE_VERSION    "OWL Witch Boot"
  #define APPLICATION_ADDRESS 0x08010000
  /* #define USE_BOOT1_PIN todo: change  */
/* #define BOOT1_Pin GPIO_PIN_2 */
/* #define BOOT1_GPIO_Port GPIOB */
#elif defined OWL_ALCHEMIST
  #define HARDWARE_VERSION    "OWL Alchemist Boot"
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_2
  #define BOOT1_GPIO_Port GPIOB
  #define USE_LED
  #define LED1_Pin GPIO_PIN_0
  #define LED1_GPIO_Port GPIOA
  #define LED2_Pin GPIO_PIN_1
  #define LED2_GPIO_Port GPIOA
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WIZARD
  #define HARDWARE_VERSION    "OWL Wizard Boot"
  #define USE_BOOT1_PIN
  #define BOOT1_Pin GPIO_PIN_2
  #define BOOT1_GPIO_Port GPIOB
  #define USE_LED
  #define LED1_Pin GPIO_PIN_0
  #define LED1_GPIO_Port GPIOA
  #define LED2_Pin GPIO_PIN_1
  #define LED2_GPIO_Port GPIOA
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_NOCTUA
  #define HARDWARE_VERSION    "OWL Noctua Boot"
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_WAVETABLE
  #define HARDWARE_VERSION    "OWL WaveTable Boot"
  #define APPLICATION_ADDRESS 0x08010000
#elif defined OWL_BIOSIGNALS
  #define HARDWARE_VERSION    "OWL BioSignals Boot"
  #define APPLICATION_ADDRESS 0x08010000
#else
  #error Invalid configuration
#endif

/* #define USE_IWDG */
/* #define INIT_FMC */
