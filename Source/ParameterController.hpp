#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"

class ParameterController {
public:
  const uint8_t parameters_size = 8;
  int16_t parameters[8];
  char names[8][12];
  int8_t selected = 0;
  ParameterController(){
    reset();
  }
  void reset(){
    for(int i=0; i<8; ++i){
      strcpy(names[i], "Parameter  ");
      names[i][10] = 'A'+i;
    }
  }
  void draw(uint8_t* pixels, uint16_t width, uint16_t height){
    static ScreenBuffer screen(width, height);
    screen.setBuffer(pixels);
    draw(screen);
  }
  void draw(ScreenBuffer& screen){
    if(sw1()){
      screen.clear();
      drawStats(screen);
      // todo: show patch name and next/previous patch
    }else if(sw2()){
      screen.clear();
      screen.setTextSize(1);
      screen.print(2, 0, names[selected]);
      screen.setTextSize(3);
      screen.setCursor(30, 20);
      screen.print(parameters[selected]/41); // assuming parameter value [0-4095]
      screen.print("%");
    }else if(getErrorStatus() != NO_ERROR && getErrorMessage() != NULL){
      screen.clear();
      screen.setTextSize(1);
      screen.print(2, 20, getErrorMessage());
    }else{
      screen.setTextSize(1);
      screen.print(2, 56, names[selected]);
      screen.print(": ");
      screen.print(parameters[selected]/41);
    }
  }
  void drawStats(ScreenBuffer& screen){
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL)
      screen.print(2, 36, pv->message);
    screen.print(2, 46, "cpu/mem: ");
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/35);
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
  }
  void encoderChanged(uint8_t encoder, int32_t delta){
    if(encoder == 0){
      if(sw2()){
	if(delta > 1)
	  selected = min(parameters_size-1, selected+1);
	else if(delta < 1)
	  selected = max(0, selected-1);
      }else{
	parameters[selected] += delta*10;
	parameters[selected] = min(4095, max(0, parameters[selected]));
      }
    } // todo: change patch with enc1/sw1
  }
  void setName(uint8_t pid, const char* name){
    if(pid < parameters_size)
      strncpy(names[pid], name, 11);
  }
private:
  bool tr1(){
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_11) != GPIO_PIN_SET;
  }
  bool tr2(){
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) != GPIO_PIN_SET;
  }
  bool sw1(){
    return HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14) != GPIO_PIN_SET;
  }
  bool sw2(){
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
  }
};

#endif // __ParameterController_hpp__
