#ifndef __MagusParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
// #include "errorhandlers.h"
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
  char title[11] = "Magus"; // max 5 chars/64px
  int16_t parameters[SIZE];
  char names[SIZE][22]; // max 21 chars/128px
  char blocknames[4][6] = {"OSC", "FLT", "ENV", "LFO"} ; // 4 times up to 5 letters/32px
  int8_t selected = 0;
  int8_t global = 0;
  int8_t mode = 0;
  ParameterController(){
    reset();
  }
  void reset(){
    for(int i=0; i<SIZE; ++i){
      strcpy(names[i], "Parameter ");
      names[i][9] = 'A'+i;
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
#if 0
    // stacked
    // 6px high by up to 128px long rectangle
    screen.drawRectangle(x, y, max(1, min(128, parameters[pid]/32)), 6, WHITE);
    // y -= 7;
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
#endif
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
    // 6px high by up to 128px long rectangle
    y -= 7;
    x += 64;
    screen.drawRectangle(x, y, max(1, min(64, parameters[pid]/64)), 6, WHITE);
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

  void drawBlockParameterNames(ScreenBuffer& screen){
    int y = 29;
    screen.setTextSize(1);
    // int i = (selected % 8) & 0xe;
    int i = selected & 0x06;
    screen.print(1, y, names[i]);
    if(selected == i)
      screen.invert(0, y-10, 64, 10);
    i += 1;
    screen.print(65, y, names[i]);
    if(selected == i)
      screen.invert(64, y-10, 64, 10);
    y += 12;
    i += 7;
    screen.print(1, y, names[i]);
    if(selected == i)
      screen.invert(0, y-10, 64, 10);
    i += 1;
    screen.print(65, y, names[i]);
    if(selected == i)
      screen.invert(64, y-10, 64, 10);
  }

  void drawBlockValues(ScreenBuffer& screen){
    // draw 4x2x2 levels on bottom 8px row
    int x = 0;
    int y = 63-7;
    for(int i=0; i<16; ++i){
      // 4px high by up to 16px long rectangle, filled if selected
      if(i == selected)
	screen.fillRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
      else
	screen.drawRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
      x += 16;
      if(i == 7){
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
    screen.clear();
    screen.setTextWrap(false);
    drawStatus(screen);
    switch(mode){
    case 0:
      // standard mode stacked
      // drawParameter(global, 63-32-6, screen);
      // drawParameter(selected, 63-17-6, screen);
      drawParameter(global, 29, screen);
      drawParameter(selected, 41, screen);
      break;
    case 1:
      // select block parameter mode
      drawBlockParameterNames(screen);
      break;
    }      
    drawBlocks(screen);
  }

  void encoderChanged(uint8_t encoder, int32_t delta){
    // if(encoder == 0){
    //   if(sw2()){
    // 	if(delta > 1)
    // 	  selected = min(SIZE-1, selected+1);
    // 	else if(delta < 1)
    // 	  selected = max(0, selected-1);
    //   }else{
    // 	parameters[selected] += delta*10;
    // 	parameters[selected] = min(4095, max(0, parameters[selected]));
    //   }
    // } // todo: change patch with enc1/sw1
  }
  void setName(uint8_t pid, const char* name){
    if(pid < SIZE)
      strncpy(names[pid], name, 11);
  }
  uint8_t getSize(){
    return SIZE;
  }
// private:
//   bool sw1(){
//     return HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14) != GPIO_PIN_SET;
//   }
//   bool sw2(){
//     return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
//   }
};

#endif // __ParameterController_hpp__
