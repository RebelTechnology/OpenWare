#include "device.h"
#include "Graphics.h"
#include "errorhandlers.h"
#include "oled.h"

#ifdef OWL_PRISM
#include "SerialBuffer.hpp"
#include "bus.h"
#endif

void Graphics::begin(SPI_HandleTypeDef *spi) {
  oled_init(spi);
  screen.setBuffer(pixelbuffer);
  screen.clear();
  display();
}

void Graphics::display(){
  oled_write(pixelbuffer, OLED_BUFFER_SIZE);
}

void Graphics::draw(){
  // drawCallback(pixelbuffer, OLED_WIDTH, OLED_HEIGHT);
  // // params.draw(pixelbuffer, OLED_WIDTH, OLED_HEIGHT);
  params.draw(screen);
}

// static void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
//   graphics.screen.setBuffer(pixels);
//   graphics.screen.clear();
//   graphics.params.draw(graphics.screen);
// }

void Graphics::setCallback(void *callback){
  params.setCallback(callback);
  // if(callback == NULL)
  //   drawCallback = defaultDrawCallback;
  // else
  //   drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
}

Graphics::Graphics() :
  screen(OLED_WIDTH, OLED_HEIGHT)
  // drawCallback(defaultDrawCallback)
{  
}

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height){
  graphics.params.drawTitle(graphics.screen);
  graphics.params.drawMessage(26, graphics.screen);

#ifdef OWL_PRISM
  extern int busstatus;
  extern SerialBuffer<128> bus_tx_buf;
  extern SerialBuffer<128> bus_rx_buf;
  graphics.screen.setTextSize(1);
  graphics.screen.setCursor(2, 46);
  if(busstatus == BUS_STATUS_IDLE)
    graphics.screen.print("idl");
  if(busstatus == BUS_STATUS_DISCOVER)
    graphics.screen.print("dis");
  if(busstatus == BUS_STATUS_CONNECTED)
    graphics.screen.print("con");
  if(busstatus == BUS_STATUS_ERROR)
    graphics.screen.print("err");
  graphics.screen.setCursor(24, 46);
  graphics.screen.print(int(bus_rx_buf.available()));
  graphics.screen.setCursor(46, 46);
  graphics.screen.print(int(bus_tx_buf.available()));
  extern UART_HandleTypeDef huart1;
  UART_HandleTypeDef *huart = &huart1;
  graphics.screen.setCursor(2, 56);
  graphics.screen.print(int(huart->RxXferSize));
  graphics.screen.setCursor(24, 56);
  graphics.screen.print(int(huart->RxXferCount));
  graphics.screen.setCursor(46, 56);
  graphics.screen.print(int(huart->TxXferSize));
  graphics.screen.setCursor(70, 56);
  graphics.screen.print(int(huart->TxXferCount));
#endif
}
