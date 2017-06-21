#ifndef _Graphics_h_
#define _Graphics_h_
#include <stdint.h>
#include "stm32f7xx_hal.h"

class Graphics {
public:
  Graphics() : hspi(NULL) {}
  void begin(SPI_HandleTypeDef *spi);
  void display(uint8_t* pixels, uint16_t size);
  void on();
  void off();
  bool isReady();
  /* void complete(); // call after finishing a screen update */
protected:
  void zero();
   void spiwrite(uint8_t data);
   void spiwrite(const uint8_t* data, size_t size);
   void spiwritesync(const uint8_t* data, size_t size);
private:
  void writeCommand(uint8_t c);
  void writeCommand(uint8_t reg, uint8_t value);
  void writeCommands(const uint8_t* cmd, uint8_t length);
  /* void setRegister(uint8_t reg, uint8_t val); */
  void chipInit();
  void commonInit();
  SPI_HandleTypeDef *hspi;
};

extern Graphics graphics;

#endif /* _Graphics_h_ */
