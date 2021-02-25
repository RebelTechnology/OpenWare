#include "device.h"
#include "VersionToken.h"

__attribute__ ((section (".bootloader_token")))
__attribute__ ((used))
static const struct VersionToken bootloader_token {
    BOOTLOADER_MAGIC,
    HARDWARE_ID,
    BOOTLOADER_VERSION,
};
