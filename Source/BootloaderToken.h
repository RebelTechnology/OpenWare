#ifndef __BOOTLOADER_TOKEN_H__
#define __BOOTLOADER_TOKEN_H__

struct BootloaderToken {
    uint32_t magic;
    uint32_t hardware_id;
    uint32_t version;
    uint32_t _reserved;
};

#endif
