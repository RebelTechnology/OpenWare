#ifndef _Graphics_h_
#define _Graphics_h_

#include <stdint.h>
#include "device.h"
#include "ScreenBuffer.h"

#if defined OWL_RACK
/* #include "PrismParameterController.hpp" */
#include "BollardsParameterController.hpp"
#elif defined OWL_MAGUS
#include "MagusParameterController.hpp"
#elif defined OWL_PRISM
#include "PrismParameterController.hpp"
#elif defined OWL_EFFECTSBOX
#include "EffectsBoxParameterController.hpp"
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
#ifdef OWL_MAGUS
  ParameterController<20> params;
#else
  ParameterController<NOF_PARAMETERS> params;
#endif
  ScreenBuffer screen;
private:
  uint8_t pixelbuffer[OLED_BUFFER_SIZE];
};

extern Graphics graphics;

#endif /* _Graphics_h_ */
