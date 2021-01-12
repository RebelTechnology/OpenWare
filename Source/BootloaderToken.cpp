#include "device.h"
#include "BootloaderToken.h"

__attribute__ ((section (".bootloader_token")))
__attribute__ ((used))
static const struct BootloaderToken bootloader_token {
    BOOTLOADER_MAGIC,
    HARDWARE_ID,
    (BOOTLOADER_VERSION_MAJOR << 16) | BOOTLOADER_VERSION_MINOR,
    0x0, // This is not use for anything but alignment for now
};
