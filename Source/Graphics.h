#ifndef _Graphics_h_
#define _Graphics_h_

#include <stdint.h>
#include "device.h"
#include "ScreenBuffer.h"

#ifdef OWL_MAGUS
#include "MagusParameterController.hpp"
#else
#include "ParameterController.hpp"
#endif

class Graphics {
public:
  Graphics();
  void begin(SPI_HandleTypeDef *spi);
  void display();
  void draw();
  void setCallback(void *callback);
  ParameterController<NOF_PARAMETERS> params;
  ScreenBuffer screen;
private:
  uint8_t pixelbuffer[OLED_BUFFER_SIZE];
  void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
};

extern Graphics graphics;

#endif /* _Graphics_h_ */
