#ifndef __MagusParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
#ifdef OWL_MAGUS
#include "errorhandlers.h"
#endif
#include "ProgramVector.h"
// #include "HAL_Encoders.h"

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
  int16_t encoders[6]; // last seen encoder values
  int16_t user[SIZE]; // user set values
  char names[SIZE][22]; // max 21 chars/128px
  char blocknames[4][6] = {"OSC", "FLT", "ENV", "LFO"} ; // 4 times up to 5 letters/32px
  uint8_t selectedBlock = 0;
  uint8_t selectedPid[4] = {0,2,4,6};
  uint8_t global = 3;
  enum ScreenMode {
    STANDARD, SELECTBLOCKPARAMETER, SELECTGLOBALPARAMETER, ERROR
  };
  ScreenMode mode = STANDARD;
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
      screen.print(x+1, y, blocknames[i]);
      if(selectedBlock == i)
	screen.invert(x, y-10, 32, 10);
      x += 32;
    }
    drawBlockValues(screen);
  }

  void drawGlobalParameterNames(ScreenBuffer& screen){
    screen.setTextSize(1);
    if(global > 0)
      screen.print(1, 24, names[global-1]);
    screen.print(1, 24+10, names[global]);
    if(global < SIZE)
      screen.print(1, 24+20, names[global+1]);
    screen.invert(0, 25, 128, 10);
  }

  void drawBlockParameterNames(ScreenBuffer& screen){
    int y = 29;
    screen.setTextSize(1);
    // int i = (selected % 8) & 0xe;
    int selected = selectedPid[selectedBlock];
    int i = selected & 0x06;
    screen.print(1, y, names[i]);
    if(selectedPid[selectedBlock] == i)
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
    int block = 0;
    for(int i=0; i<16; ++i){
      // 4px high by up to 16px long rectangle, filled if selected
      if(i == selectedPid[block])
	screen.fillRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
      else
	screen.drawRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
      x += 16;
      if(i & 0x01)
	block++;
      if(i == 7){
	x = 0;
	y += 3;
	block = 0;
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

  void drawError(ScreenBuffer& screen){
    if(getErrorMessage() != NULL){
      screen.setTextWrap(true);
      screen.print(0, 29, getErrorMessage());
      screen.setTextWrap(false);
    }
  }

  void draw(ScreenBuffer& screen){
    screen.clear();
    screen.setTextWrap(false);
    drawStatus(screen);
    switch(mode){
    case STANDARD:
      // standard mode stacked
      // drawParameter(global, 63-32-6, screen);
      // drawParameter(selected, 63-17-6, screen);
      drawParameter(global, 29, screen);
      drawParameter(selectedPid[selectedBlock], 41, screen);
      break;
    case SELECTBLOCKPARAMETER:
      // select block parameter mode
      drawBlockParameterNames(screen);
      break;
    case SELECTGLOBALPARAMETER:
      drawGlobalParameterNames(screen);
      break;
    case ERROR:
      drawError(screen);
      break;
    }      
    drawBlocks(screen);
  }

  void setName(uint8_t pid, const char* name){
    if(pid < SIZE)
      strncpy(names[pid], name, 11);
  }

  uint8_t getSize(){
    return SIZE;
  }

  void updateEncoders(int16_t* data, uint8_t size){
    uint16_t pressed = data[0];
    mode = STANDARD;
    for(uint8_t i=0; i<4; ++i){
      int16_t value = data[i+3];
      if(pressed&(1<<(i+2))){
	// update selected block parameter. TODO: reset encoder value
	selectedBlock = i;
	mode = SELECTBLOCKPARAMETER;
	if(value < encoders[i+2]){
	  encoders[i+2] = value;
	  selectedPid[i]--;
	  if(selectedPid[i] < i*2+8 && selectedPid[i] > i*2+1)
	    selectedPid[i] = i*2+1;
	  selectedPid[i] = max(i*2, min(i*2+9, selectedPid[i]));
	}else if(value > encoders[i+2]){
	  encoders[i+2] = value;
	  selectedPid[i]++;
	  if(selectedPid[i] < i*2+8 && selectedPid[i] > i*2+1)
	    selectedPid[i] = i*2+8;
	  selectedPid[i] = max(i*2, min(i*2+9, selectedPid[i]));
	}
      }else{
	if(encoders[i+2] != value){
	  selectedBlock = i;
	  encoders[i+2] = value;
	  int pid = selectedPid[i];
	  user[pid] = value;
	}
      }
    }
    if(pressed&(1<<0)){
      // update selected global parameter. TODO: reset encoder value
      // TODO: add 'special' parameters: Volume, Freq, Gain, Gate
      mode = SELECTGLOBALPARAMETER;
      int16_t value = data[1];
      if(value < encoders[0]){
	encoders[0] = value;
	global = max(0, min(SIZE-1, global-1));
      }else if(value > encoders[0]){
	encoders[0] = value;
	global = max(0, min(SIZE-1, global+1));
      }
    }else{
      int16_t value = data[1];
      encoders[0] = value;
      user[global] = value;
    }
    if(pressed&(1<<1)){
      // TODO
      // mode = SELECTPRESET;
    }
  }

  void updateValue(uint8_t port, int16_t value){
    static int16_t multiplier = 32;
    parameters[port] = user[port]*multiplier + value;
  }

  // void updateValues(uint16_t* data, uint8_t size){
    // for(int i=0; i<16; ++i)
    //   parameters[selectedPid[i]] = encoders[i] + data[i];
  // }

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

  // private:
//   bool sw1(){
//     return HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14) != GPIO_PIN_SET;
//   }
//   bool sw2(){
//     return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
//   }
};

#endif // __ParameterController_hpp__
