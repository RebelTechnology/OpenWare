#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include <string.h>
#include "errorhandlers.h"
#include "ProgramVector.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

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

#endif // __ParameterController_hpp__
