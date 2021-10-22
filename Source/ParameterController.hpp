#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include <string.h>
#include "errorhandlers.h"
#include "ProgramVector.h"

class ParameterController {
protected:
  char title[11];
  int16_t parameters[NOF_PARAMETERS];
  char names[NOF_PARAMETERS][12];
public:
  // ParameterController();
  virtual void reset(){
    for(int i=0; i<NOF_PARAMETERS; ++i){
      strcpy(names[i], "Parameter  ");
      names[i][10] = 'A'+i;
      parameters[i] = 0;
    }
  }
  virtual void draw(ScreenBuffer& screen) = 0;
  /* Update parameters with encoders */
  virtual void updateEncoders(int16_t* data, uint8_t size) = 0;
  /* Update parameters with ADC values */
  virtual void updateValues(int16_t* values, size_t len){};
  const char* getName(uint8_t pid){
    if(pid < NOF_PARAMETERS)
      return names[pid];
    return "";
  }
  void setName(uint8_t pid, const char* name){
    if(pid < NOF_PARAMETERS)
      strncpy(names[pid], name, 11);
  }
  void setValue(uint8_t pid, int16_t value){    
    parameters[pid] = value;
  }
  int16_t getValue(uint8_t pid){
    return parameters[pid];
  }
  int16_t* getParameters(){
    return parameters;
  }
  size_t getSize(){
    return NOF_PARAMETERS;
  }
  void setTitle(const char* str){
    strncpy(title, str, sizeof(title)-1);    
  }
  const char* getTitle(){
    return title;
  }
};  


#if 0
class Page {
public:
  virtual void draw(ScreenBuffer& screen){}
  void updateEncoders(int16_t* data, uint8_t size);
  void enter();
  void exit();
};

class SensitivityPage : public Page {
public:
  void draw(ScreenBuffer& screen){
  }
};

class PageManager {
private:
  Page* page;
public:
  virtual void draw(ScreenBuffer& screen){}  
  void updateEncoders(int16_t* data, uint8_t size);  
};

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);

/* shows a single parameter selected and controlled with a single encoder
 */
template<uint8_t SIZE>
class ParameterController {
public:
  char title[11] = "Prism";
  int16_t parameters[SIZE];
  char names[SIZE][12];
  int8_t selected = 0;
  ParameterController(){
    reset();
  }
  void setTitle(const char* str){}
  void reset(){
    drawCallback = defaultDrawCallback;
    for(int i=0; i<SIZE; ++i){
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
      screen.setTextSize(1);
      screen.print(2, 0, names[selected]);
      screen.setTextSize(3);
      screen.setCursor(30, 20);
      screen.print(parameters[selected]/41); // assuming parameter value [0-4095]
      screen.print("%");
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

  void drawStats(ScreenBuffer& screen){
    screen.setTextSize(1);
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL)
      screen.print(2, 36, pv->message);
    screen.print(2, 46, "cpu/mem: ");
    float percent = (pv->cycles_per_block/pv->audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
    screen.print((int)(percent*100));
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
  }

  void encoderChanged(uint8_t encoder, int32_t delta){
    if(encoder == 0){
      if(sw2()){
	if(delta > 1)
	  selected = min(SIZE-1, selected+1);
	else if(delta < 1)
	  selected = max(0, selected-1);
      }else{
	parameters[selected] += delta*10;
	parameters[selected] = min(4095, max(0, parameters[selected]));
      }
    } // todo: change patch with enc1/sw1
  }
  void setName(uint8_t pid, const char* name){
    if(pid < SIZE)
      strncpy(names[pid], name, 11);
  }
  uint8_t getSize(){
    return SIZE;
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
#endif

#endif // __ParameterController_hpp__
