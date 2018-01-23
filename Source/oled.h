#include <stdint.h>
#include "device.h"

#ifdef __cplusplus
 extern "C" {
#endif

  void oled_init(SPI_HandleTypeDef *spi);
  void oled_write(const uint8_t* pixels, uint16_t size);

#ifdef __cplusplus
}
#endif
