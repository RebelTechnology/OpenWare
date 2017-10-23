#include "device.h"
#include "Graphics.h"
#include "errorhandlers.h"
#include "oled.h"

void Graphics::begin(SPI_HandleTypeDef *spi) {
  oled_init(spi);
}

void Graphics::display(){
  oled_write(pixelbuffer, OLED_BUFFER_SIZE);
}

void Graphics::draw(){
  drawCallback(pixelbuffer, OLED_WIDTH, OLED_HEIGHT);
  // params.draw(pixelbuffer, OLED_WIDTH, OLED_HEIGHT);
  params.draw(screen);
}

static void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
  graphics.screen.setBuffer(pixels);
  graphics.screen.clear();
  graphics.screen.setTextSize(1);
  graphics.screen.print(20, 0, "Rebel Technology");
}

void Graphics::setCallback(void *callback){
  if(callback == NULL)
    drawCallback = defaultDrawCallback;
  else
    drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
}

Graphics::Graphics() :
  screen(OLED_WIDTH, OLED_HEIGHT),
  drawCallback(defaultDrawCallback)
{  
}
