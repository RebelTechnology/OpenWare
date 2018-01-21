#ifndef __MagusParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "HAL_Encoders.h"

/*    
screen 128 x 64, font 5x7
4 blocks, 32px per each, 3-4 letters each

4 bottom encoders
press once to toggle mode: update > select
turn to scroll through 4 functions
press again to select parameter: select > update

todo:
- update parameter / encoderChanged
- select parameter
- select global parameter
- select preset mode
*/
template<uint8_t SIZE>
class ParameterController {
public:
  char title[11] = "PolySub"; // max 10 chars/64px
  int16_t parameters[SIZE];
  char names[SIZE][22]; // max 21 chars/128px
  char blocknames[4][6] = {"osc", "flt", "env", "lfo"} ; // 4 times up to 5 letters/32px
  int8_t selected = 0;
  int8_t global = 0;
  ParameterController(){
    reset();
  }
  void reset(){
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

  void drawParameter(int pid, int y, ScreenBuffer& screen){
    int x = 0;
    // 6px high by up to 128px long rectangle
    screen.drawRectangle(x, y, max(0, min(128, parameters[pid]/32)), 6, WHITE);
    y -= 7;
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
  }

  void drawBlocks(ScreenBuffer& screen){
    int x = 0;
    int y = 63-8;
    screen.setTextSize(1);
    for(int i=0; i<4; ++i){
      screen.print(x, y, blocknames[i]);
      x += 32;
    }
    drawBlockValues(screen);
  }

  void drawBlockValues(ScreenBuffer& screen){
    // draw 4x2x2 levels on bottom 8px row
    int x = 0;
    int y = 63-8+4;
    for(int i=0; i<16; ++i){
      // 4px high by up to 16px long rectangle, filled if selected
      if(i == selected)
	screen.fillRectangle(x, y, max(0, min(16, parameters[i]/255)), 4, WHITE);
      else
	screen.drawRectangle(x, y, max(0, min(16, parameters[i]/255)), 4, WHITE);
      x += 16;
      if(i == 4){
	x = 0;
	y += 3;
      }
    }
  }

  void drawStatus(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(0, 16, title);

    screen.setTextSize(1);
    screen.print(64, 8, "cpu");
    screen.print(96, 8, "mem");

    screen.setCursor(64, 17);
    ProgramVector* pv = getProgramVector();
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/35);
    screen.print("%");
    screen.setCursor(96, 17);
    int mem = (int)(pv->heap_bytes_used)/1024;
    if(mem > 999){
      screen.print(mem/1024.0f);
      screen.print("M");
    }else{
      screen.print(mem);
      screen.print("k");
    }    
  }

  void draw(ScreenBuffer& screen){
    screen.setTextWrap(false);
    drawStatus(screen);
    drawParameter(global, 63-32, screen);
    drawParameter(selected, 63-17, screen);
    drawBlocks(screen);
    
    // if(sw1()){
    //   screen.clear();
    //   drawStats(screen);
    //   // todo: show patch name and next/previous patch
    // }else if(sw2()){
    //   screen.clear();
    //   screen.setTextSize(1);
    //   screen.print(2, 0, names[selected]);
    //   screen.setTextSize(3);
    //   screen.setCursor(30, 20);
    //   screen.print(parameters[selected]/41); // assuming parameter value [0-4095]
    //   screen.print("%");
    // }else if(getErrorStatus() != NO_ERROR && getErrorMessage() != NULL){
    //   screen.clear();
    //   screen.setTextSize(1);
    //   screen.print(2, 20, getErrorMessage());
    // }else{
    //   screen.setTextSize(1);
    //   screen.print(2, 56, names[selected]);
    //   screen.print(": ");
    //   screen.print(parameters[selected]/41);
    // }
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
private:
  bool sw1(){
    return HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14) != GPIO_PIN_SET;
  }
  bool sw2(){
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
  }
};

#endif // __ParameterController_hpp__
