#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "hardware.h"
#include "support.h"

#define FIRMWARE_VERSION "v22.5.0"

#ifdef USE_SPI_FLASH
#define USE_NOR_FLASH
#ifndef SPI_FLASH_HANDLE
#ifdef STM32H7xx
#define SPI_FLASH_HANDLE             hspi5
#else
#define SPI_FLASH_HANDLE             hspi1
#endif
#endif
#endif

#ifdef USE_QSPI_FLASH
#define USE_NOR_FLASH
#define QSPI_FLASH_HANDLE            hqspi
#endif

#ifdef USE_NOR_FLASH
#define MAX_SPI_FLASH_HEADERS        48
#define FLASH_DEFAULT_FLAGS          RESOURCE_PORT_MAPPED
#define EXTERNAL_STORAGE_SIZE        (8*1024*1024) // 8M / 64Mbit
#else
#define MAX_SPI_FLASH_HEADERS        0
#define FLASH_DEFAULT_FLAGS          RESOURCE_MEMORY_MAPPED
#endif

#ifdef NO_INTERNAL_FLASH
#define MAX_RESOURCE_HEADERS         MAX_SPI_FLASH_HEADERS
#else
#define USE_FLASH
#define MAX_RESOURCE_HEADERS         (8+MAX_SPI_FLASH_HEADERS)
#endif

#ifndef AUDIO_OUTPUT_GAIN
#define AUDIO_OUTPUT_GAIN            112
#endif
#ifndef AUDIO_INPUT_GAIN
#define AUDIO_INPUT_GAIN             92
#endif

#define MIDI_OMNI_CHANNEL            (-1)
#ifndef MIDI_INPUT_CHANNEL
#define MIDI_INPUT_CHANNEL           MIDI_OMNI_CHANNEL
#endif
#ifndef MIDI_OUTPUT_CHANNEL
#define MIDI_OUTPUT_CHANNEL          0
#endif
#ifndef DIGITAL_BUS_ENABLED
#define DIGITAL_BUS_ENABLED          0
#define DIGITAL_BUS_FORWARD_MIDI     0
#endif
#define USE_MIDI_TX_BUFFER
#define USE_MIDI_CALLBACK
#define MIDI_OUTPUT_BUFFER_SIZE      1024
#define MIDI_INPUT_BUFFER_SIZE       64
#define MIDI_SYSEX_BUFFER_SIZE       256
#define USE_MESSAGE_CALLBACK

#ifndef USBD_MAX_POWER
#define USBD_MAX_POWER               100 // 200mA
#endif

#ifndef OWLBOOT_MAGIC_NUMBER
#define OWLBOOT_MAGIC_NUMBER        0xDADAB007
#endif
#define OWLBOOT_DFU_NUMBER          0xDADADEAF
#define OWLBOOT_LOOP_NUMBER         0xDADADEAD
#define OWLBOOT_MAGIC_ADDRESS       ((volatile uint32_t*)0x2000FFF0)

#ifndef STORAGE_MAX_BLOCKS
#define STORAGE_MAX_BLOCKS           64
#endif

#define DEBUG_DWT

/* #define DEBUG_STACK */
#define DEBUG_STORAGE
/* #define DEBUG_BOOTLOADER */

#ifdef SSD1331
#define OLED_WIDTH		     96
#define OLED_HEIGHT		     64
#define OLED_BUFFER_SIZE             (OLED_WIDTH*OLED_HEIGHT/8)
#elif defined SEPS114A
#define OLED_WIDTH		     96
#define OLED_HEIGHT		     96
#define OLED_BUFFER_SIZE             (OLED_WIDTH*OLED_HEIGHT*sizeof(uint16_t))
#elif defined SSD1309
#define OLED_WIDTH		     128
#define OLED_HEIGHT	             64
#define OLED_BUFFER_SIZE             (OLED_WIDTH*OLED_HEIGHT/8)
#endif

#ifndef MAX_SYSEX_FIRMWARE_SIZE
#define MAX_SYSEX_FIRMWARE_SIZE      ((16+16+64+128+128)*1024) // FLASH sectors 2-6
#endif
#ifndef MAX_SYSEX_BOOTLOADER_SIZE
#define MAX_SYSEX_BOOTLOADER_SIZE    (64 * 1024) // OWL1 uses 32kb, must be overridden
#endif
#ifndef MAX_SYSEX_PROGRAM_SIZE
#define MAX_SYSEX_PROGRAM_SIZE       (144 * 1024)
#endif
#ifdef USE_BOOTLOADER_MODE // Flag to choose if we're flashing firmware or bootloader from SySex
#define MAX_SYSEX_PAYLOAD_SIZE       MAX_SYSEX_FIRMWARE_SIZE
#else
#define MAX_SYSEX_PAYLOAD_SIZE       (1 * 1024 * 1024) // Maximum resource size
#endif
#define BOOTLOADER_MAGIC             0xB007C0DE
#define BOOTLOADER_VERSION           FIRMWARE_VERSION

#ifndef OWL_XIBECA
#define USE_FFT_TABLES
#endif
#define USE_FAST_POW

#ifndef MAX_NUMBER_OF_PATCHES
#define MAX_NUMBER_OF_PATCHES        40
#endif
#define APPLICATION_SETTINGS_RESOURCE_INDEX (MAX_NUMBER_OF_PATCHES + 1)
#define APPLICATION_SETTINGS_NAME    "__SETTINGS__"
#ifndef MAX_NUMBER_OF_RESOURCES
#define MAX_NUMBER_OF_RESOURCES      12
#endif

/* +0db in and out */
#ifndef AUDIO_INPUT_OFFSET
#define AUDIO_INPUT_OFFSET           0xffffefaa /* -0.06382 * 65535 */
#endif
#ifndef AUDIO_INPUT_SCALAR
#define AUDIO_INPUT_SCALAR           0xfffbb5c7 /* -4.290 * 65535 */
#endif
#ifndef AUDIO_OUTPUT_OFFSET
#define AUDIO_OUTPUT_OFFSET          0x00001eec /* 0.1208 * 65535 */
#endif
#ifndef AUDIO_OUTPUT_SCALAR
#define AUDIO_OUTPUT_SCALAR          0xfffb5bab /* -4.642 * 65535 */
#endif
#define DEFAULT_PROGRAM              1
#define BUTTON_PROGRAM_CHANGE
#define AUDIO_BITDEPTH               24    /* bits per sample */
#define AUDIO_DATAFORMAT             24
#define AUDIO_CODEC_MASTER           true
#ifndef AUDIO_CHANNELS
#define AUDIO_CHANNELS               2
#endif
#ifndef AUDIO_SAMPLINGRATE
#define AUDIO_SAMPLINGRATE           48000
#endif

#ifndef AUDIO_BLOCK_SIZE
#define AUDIO_BLOCK_SIZE             32 /* default size in samples of a single channel audio block */
#endif
#ifndef CODEC_BLOCKSIZE
#define CODEC_BLOCKSIZE              512 /* maximum audio blocksize */
#endif
#define CODEC_BUFFER_SIZE            (2*AUDIO_CHANNELS*CODEC_BLOCKSIZE)


#define USBD_AUDIO_RX_FREQ           AUDIO_SAMPLINGRATE
#define USBD_AUDIO_TX_FREQ           AUDIO_SAMPLINGRATE
#ifndef USBD_AUDIO_RX_CHANNELS
#define USBD_AUDIO_RX_CHANNELS       AUDIO_CHANNELS
#endif
#ifndef USBD_AUDIO_TX_CHANNELS
#define USBD_AUDIO_TX_CHANNELS       AUDIO_CHANNELS
#endif

#ifndef MAIN_LOOP_SLEEP_MS
#define MAIN_LOOP_SLEEP_MS           2
#endif

#ifndef SCREEN_LOOP_SLEEP_MS
#define SCREEN_LOOP_SLEEP_MS         40 /* 40mS = 25 fps */
#endif

#ifndef LOAD_INDICATOR_PARAMETER
#define LOAD_INDICATOR_PARAMETER     PARAMETER_A
#endif

#ifndef LEDS_BRIGHTNESS
#define LEDS_BRIGHTNESS              20 /* default value - 0 = off, 63 = supernova */
#endif

#define PROGRAM_TASK_STACK_SIZE      (8*1024/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_STACK_SIZE      (2*1024/sizeof(portSTACK_TYPE))
#define SCREEN_TASK_STACK_SIZE       (4*1024/sizeof(portSTACK_TYPE))
#define UTILITY_TASK_STACK_SIZE      (1*1024/sizeof(portSTACK_TYPE))

#ifndef ARM_CYCLES_PER_SAMPLE
#define ARM_CYCLES_PER_SAMPLE        (168000000/AUDIO_SAMPLINGRATE) /* 168MHz / 48kHz */
#endif

#define USE_IWDG                     // compile with support for IWDG watchdog

#ifndef NO_EXTERNAL_RAM
#define USE_EXTERNAL_RAM
#endif

#ifdef NO_CCM_RAM
#define CCM_RAM
#else
#define USE_CCM_RAM
#define CCM_RAM                      __attribute__ ((section (".ccmdata")))
#endif

#ifndef DMA_RAM
#define DMA_RAM
#endif

#ifndef DAC_HANDLE
#define DAC_HANDLE                   hdac
#endif

#if defined USE_USBD_FS
#define USE_USB_DEVICE
#define USE_USBD_MIDI
#define USBD_DESC FS_Desc
#define USBD_HSFS DEVICE_FS
#define USBD_HANDLE hUsbDeviceFS
#define USBD_PCD_HANDLE hpcd_USB_OTG_FS
#define USB_OTG_BASE_ADDRESS  USB_OTG_FS
#elif defined USE_USBD_HS
#define USE_USB_DEVICE
#define USE_USBD_MIDI
#define USBD_DESC HS_Desc
#define USBD_HSFS DEVICE_HS
#define USBD_HANDLE hUsbDeviceHS
#define USBD_PCD_HANDLE hpcd_USB_OTG_HS
#define USB_OTG_BASE_ADDRESS  USB_OTG_HS
#endif

#if defined USE_USBH_HS
#define USE_USB_HOST
#define USE_USBH_MIDI
#define USB_HOST_RX_BUFF_SIZE       256
#define USBH_HANDLE                 hUsbHostHS
#define USBH_HCD_HANDLE             hhcd_USB_OTG_HS
#define USBH_HSFS                   HOST_HS
#elif defined USE_USBH_FS
#define USE_USB_HOST
#define USE_USBH_MIDI
#define USB_HOST_RX_BUFF_SIZE       256
#define USBH_HANDLE                 hUsbHostFS
#define USBH_HCD_HANDLE             hhcd_USB_OTG_FS
#define USBH_HSFS                   HOST_FS
#endif

#endif /* __DEVICE_H__ */
