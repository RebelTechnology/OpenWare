#include "device.h"
#include "Graphics.h"
#include "errorhandlers.h"
#include "oled.h"
#include "callbacks.h"
#include "ProgramVector.h"

Graphics::Graphics() :
  screen(OLED_WIDTH, OLED_HEIGHT),
  drawCallback(defaultDrawCallback) {
  screen.clear();
}

void Graphics::begin(ParameterController* pc, SPI_HandleTypeDef *spi) {
  params = pc;
  oled_init(spi);
  screen.setBuffer(pixelbuffer);
  screen.clear();
  reset();
  display();
}

void Graphics::display(){
  oled_write(pixelbuffer, OLED_BUFFER_SIZE);
}

void Graphics::draw(){
  params->draw(screen);
}

// static void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
//   graphics.screen.setBuffer(pixels);
//   graphics.screen.clear();
//   graphics.params->draw(graphics.screen);
// }

void Graphics::reset(){
  drawCallback = defaultDrawCallback;
  params->reset();
}

void Graphics::setCallback(void *callback){
  if(callback == NULL)
    drawCallback = defaultDrawCallback;
  else
    drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
}

__weak void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
  // doDefaultDrawCallback(graphics.screen);
  ScreenBuffer screen = graphics.screen;

  // draw title
  screen.setTextSize(2);
  screen.print(0, 16, graphics.params->getTitle());

  // draw program message, if set
  ProgramVector* pv = getProgramVector();
  if(pv != NULL && pv->message != NULL){
    screen.setTextSize(1);
    screen.setTextWrap(true);
    screen.print(0, 26, pv->message);
    screen.setTextWrap(false);
  }
}
