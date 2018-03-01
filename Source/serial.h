#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   void serial_read(uint8_t* data, uint16_t size);
   void serial_write(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif
