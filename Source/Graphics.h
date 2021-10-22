#ifndef _Graphics_h_
#define _Graphics_h_

#include <stdint.h>
#include "device.h"
#include "ScreenBuffer.h"
#include "ParameterController.hpp"

class Graphics {
public:
  Graphics();
  void begin(ParameterController* params, SPI_HandleTypeDef* spi);
  void display();
  void draw();
  void reset();
  void setCallback(void* callback);
  ScreenBuffer screen;
  ParameterController* params;
  void (*drawCallback)(uint8_t* pixels, uint16_t width, uint16_t height);
private:
  uint8_t pixelbuffer[OLED_BUFFER_SIZE];
};

extern Graphics graphics;

#endif /* _Graphics_h_ */
