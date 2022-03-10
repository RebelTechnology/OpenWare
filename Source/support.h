#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

   void device_dfu(); // reset to DFU bootloader
   void device_reset(); // reset and restart firmware
   void device_bootloader(); // reset to flash bootloader

   /* advanced: reset to provided address (or 0 for system reset) */
   void device_reset_to(uint32_t address);

   void device_cache_invalidate();
   void device_watchdog(); // tickle the IWDG to prevent reset

#ifdef __cplusplus
}
#endif


#endif /*  __SUPPORT_H__ */
