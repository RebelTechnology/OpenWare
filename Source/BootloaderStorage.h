#ifndef __BOOTLOADER_STORAGE_H__
#define __BOOTLOADER_STORAGE_H__

#include <inttypes.h>

class BootloaderStorage {
public:
    BootloaderStorage(); 
    /**
     * Erase all bootloader sections
     * @result: true on success
     */
    bool erase();
    /**
     * Store data of known size (make sure it fits in advance)
     * @result: true on success
     */
    bool store(void* source, size_t size);
    /**
     * Unlock write protection
     * @result: true on success
     */
    bool unlock();
    /**
     * Lock write protection
     * @result: true on success
     */
    bool lock();
    /**
     * Get map of all write protected sectors. This should be used to check if
     * there's one or more sectors that requires disabling write protection.
     */
    uint32_t getWriteProtectedSectors() const;
    /**
     * Check that all bootloader sectors have write protection enabled.
     */
    bool isWriteProtected() const;
private:
    uint32_t bootloader_sectors;
};

extern BootloaderStorage bootloader;
#endif
