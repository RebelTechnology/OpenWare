#ifndef __XibecaParameterController_hpp__
#define __XibecaParameterController_hpp__

#include <stdint.h>
#include <string.h>
#include "device.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "VersionToken.h"
#include "ScreenBuffer.h"
#include "Owl.h"
#include "ParameterController.hpp"
#ifdef USE_DIGITALBUS
#include "DigitalBusReader.h"
extern DigitalBusReader bus;
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

extern VersionToken* bootloader_token;

#define NOF_ENCODERS 4

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);

/* shows a single parameter selected and controlled with a single encoder
 */
class XibecaParameterController : public ParameterController {
public:
  char title[11] = "Xibeca";
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t parameters[NOF_PARAMETERS];
  char names[NOF_PARAMETERS][12];
  int8_t selected = 0;
  XibecaParameterController(){
    reset();
  }
  void setTitle(const char* str){
    strncpy(title, str, 10);    
  }
  void reset(){
    drawCallback = defaultDrawCallback;
    for(int i=0; i<NOF_PARAMETERS; ++i){
      strcpy(names[i], "Parameter  ");
      names[i][10] = 'A'+i;
      parameters[i] = 0;
    }
  }
  void draw(uint8_t* pixels, uint16_t width, uint16_t height){
    ScreenBuffer screen(width, height);
    screen.setBuffer(pixels);
    draw(screen);
  }

  void draw(ScreenBuffer& screen){
    screen.clear();
    if(sw1()){
      drawStats(screen);
      // todo: show patch name and next/previous patch
    }else if(sw2()){
      screen.fill(BLACK);
      drawGlobalParameterNames(42, screen);
    }else if(getErrorStatus() != NO_ERROR && getErrorMessage() != NULL){
      screen.setTextSize(1);
      screen.print(2, 20, getErrorMessage());
    }else{
      drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
      screen.setTextSize(1);
      screen.print(2, 56, names[selected]);
      screen.print(": ");
      screen.print((int)parameters[selected]/41);
    }
  }

  void drawParameterNames(int y, int pid, const char names[][12], int size, ScreenBuffer& screen){
    screen.setTextSize(1);
    // int selected = selectedPid[pid];
    if(selected > 2)
      screen.print(1, y-30, names[selected-3]);
    if(selected > 1)
      screen.print(1, y-20, names[selected-2]);
    if(selected > 0)
      screen.print(1, y-10, names[selected-1]);
    screen.print(1, y, names[selected]);
    if(selected < size-1)
      screen.print(1, y+10, names[selected+1]);
    if(selected < size-2)
      screen.print(1, y+20, names[selected+2]);
    if(selected < size-3)
      screen.print(1, y+30, names[selected+3]);
    screen.invert(0, y-9, screen.getWidth(), 10);
  }

  void drawGlobalParameterNames(int y, ScreenBuffer& screen){    
    drawParameterNames(y, 0, names, NOF_PARAMETERS, screen);
  }

  void drawStats(ScreenBuffer& screen){
    screen.setTextSize(1);
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL)
      screen.print(2, 16, pv->message);
    screen.print(2, 26, "cpu/mem: ");
    float percent = (pv->cycles_per_block/pv->audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
    screen.print((int)(percent*100));
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");

    // draw firmware version
    screen.print(1, 36, getFirmwareVersion());
    if (bootloader_token->magic == BOOTLOADER_MAGIC){
      screen.print(" bt.");
      screen.print(getBootloaderVersion());
    }
#ifdef USE_DIGITALBUS
    screen.print(1, 56, "Bus: ");
    screen.print(bus.getStatusString());
    screen.print(" ");
    extern uint32_t bus_tx_packets, bus_rx_packets;
    screen.print((int)bus_tx_packets);
    screen.print("/");
    screen.print((int)bus_rx_packets);
    if (bus.getStatus() == BUS_STATUS_CONNECTED) {
      screen.print(", ");
      screen.print(bus.getPeers());
      screen.print(" peers");
    }
#endif
  }

  void updateEncoders(int16_t* data, uint8_t size){
    if(data[0] != encoders[0]){
      encoderChanged(0, data[0] - encoders[0]);
      encoders[0] = data[0];
    }
    if(data[1] != encoders[1]){
      encoderChanged(1, data[1] - encoders[1]);
      encoders[1] = data[1];
    }
  }
  void encoderChanged(uint8_t encoder, int32_t delta){
    if(encoder == 0){
      if(sw2()){
	if(delta > 1)
	  selected = min(NOF_PARAMETERS-1, selected+1);
	else if(delta < 1)
	  selected = max(0, selected-1);
      }else{
	parameters[selected] += delta*10;
	parameters[selected] = min(4095, max(0, parameters[selected]));
      }
    } // todo: change patch with enc1/sw1
  }
  void setName(uint8_t pid, const char* name){
    if(pid < NOF_PARAMETERS)
      strncpy(names[pid], name, 11);
  }
  uint8_t getSize(){
    return NOF_PARAMETERS;
  }
  void setValue(uint8_t ch, int16_t value){
    parameters[ch] = value;
  }

  void drawMessage(int16_t y, ScreenBuffer& screen){
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, y, pv->message);
      screen.setTextWrap(false);
    }    
  }

  void drawTitle(ScreenBuffer& screen){
    drawTitle(title, screen);
  }

  void drawTitle(const char* title, ScreenBuffer& screen){
    // draw title
    screen.setTextSize(2);
    screen.print(1, 17, title);
  }

  void setCallback(void *callback){
    if(callback == NULL)
      drawCallback = defaultDrawCallback;
    else
      drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
  }
  
private:
  void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
  bool sw1(){
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_SET;
  }
  bool sw2(){
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
  }
};

#endif // __XibecaParameterController_hpp__
