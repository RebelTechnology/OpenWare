#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
#ifdef OWL_MAGUS
#include "errorhandlers.h"
#endif
#include "ProgramVector.h"
// #include "HAL_Encoders.h"


#define ENC_MULTIPLIER 6 // shift left by this many steps
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
  char title[11] = "Magus";
  int16_t parameters[SIZE];
  int16_t encoders[6]; // last seen encoder values
  int16_t user[SIZE]; // user set values (ie by encoder or MIDI)
  char names[SIZE][22]; // max 21 chars/128px
  char blocknames[4][6] = {"OSC", "FLT", "ENV", "LFO"} ; // 4 times up to 5 letters/32px
  uint8_t selectedBlock;;
  uint8_t selectedPid[4];
  uint8_t global;
  uint8_t mostRecentPid;
  enum ScreenMode {
    STANDARD, SELECTBLOCKPARAMETER, SELECTGLOBALPARAMETER, SELECTPROGRAM, ERROR
  };
  ScreenMode mode;
  ParameterController(){
    reset();
  }
  void reset(){
    for(int i=0; i<SIZE; ++i){
      strcpy(names[i], "Parameter ");
      names[i][9] = 'A'+i;
      parameters[i] = 0;
    }
    selectedBlock = -1;
    selectedPid[0] = 0;
    selectedPid[1] = 2;
    selectedPid[2] = 4;
    selectedPid[3] = 6;
    global = 0;
    mostRecentPid = global;
    mode = STANDARD;
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
    if(global < SIZE-1)
      screen.print(1, 24+20, names[global+1]);
    screen.invert(0, 25, 128, 10);
  }

  void drawBlockParameterNames(ScreenBuffer& screen){
    int y = 29;
    screen.setTextSize(1);
    // int i = (selected % 8) & 0xe;
    selectedBlock &= 0x03;
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

  void drawStats(ScreenBuffer& screen){
    screen.setTextSize(1);
    // screen.clear(86, 0, 128-86, 16);
    // draw memory use
    screen.print(80, 8, "mem");
    ProgramVector* pv = getProgramVector();
    screen.setCursor(80, 17);
    int mem = (int)(pv->heap_bytes_used)/1024;
    if(mem > 999){
      screen.print(mem/1024);
      screen.print("M");
    }else{
      screen.print(mem);
      screen.print("k");
    }
    // draw CPU load
    screen.print(111, 8, "cpu");
    screen.setCursor(111, 17);
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/35);
    screen.print("%");
  }

  void drawError(ScreenBuffer& screen){
    if(getErrorMessage() != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, 26, getErrorMessage());
      screen.setTextWrap(false);
    }
  }

  void drawTitle(const char* title, ScreenBuffer& screen){
    // draw title
    screen.setTextSize(2);
    screen.print(0, 16, title);
  }

  void drawMessage(ScreenBuffer& screen){
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, 26, pv->message);
    }    
  }

  void draw(ScreenBuffer& screen){
    screen.clear();
    screen.setTextWrap(false);
    switch(mode){
    case STANDARD:
      // standard mode stacked
      // drawParameter(global, 29, screen);
      // drawParameter(selectedPid[selectedBlock], 41, screen);
      drawTitle(title, screen);
      drawMessage(screen);
      drawParameter(mostRecentPid, 44, screen);
      break;
    case SELECTBLOCKPARAMETER:
      drawTitle(title, screen);
      drawBlockParameterNames(screen);
      break;
    case SELECTGLOBALPARAMETER:
      drawTitle(title, screen);
      drawGlobalParameterNames(screen);
      break;
    case SELECTPROGRAM:
      drawTitle("Magus", screen);
      drawMessage(screen);
      drawStats(screen);
      // todo!
      break;
    case ERROR:
      drawTitle("ERROR", screen);
      drawError(screen);
      drawStats(screen);
      break;
    }
    drawBlocks(screen);
  }

  void setName(uint8_t pid, const char* name){
    if(pid < SIZE)
      strncpy(names[pid], name, 11);
  }

  void setTitle(const char* str){
    strncpy(title, str, 10);    
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
	if(value < encoders[i+2]-1){
	  encoders[i+2] = value;
	  selectedPid[i]--;
	  if(selectedPid[i] < i*2+8 && selectedPid[i] > i*2+1)
	    selectedPid[i] = i*2+1;
	  selectedPid[i] = max(i*2, min(i*2+9, selectedPid[i]));
	}else if(value > encoders[i+2]+1){
	  encoders[i+2] = value;
	  selectedPid[i]++;
	  if(selectedPid[i] < i*2+8 && selectedPid[i] > i*2+1)
	    selectedPid[i] = i*2+8;
	  selectedPid[i] = max(i*2, min(i*2+9, selectedPid[i]));
	}
	mostRecentPid = selectedPid[i];
      }else{
	if(encoders[i+2] != value){
	  selectedBlock = i;
	  encoders[i+2] = value;
	  int pid = selectedPid[i];
	  user[pid] = value<<ENC_MULTIPLIER; // scale encoder values up
	  mostRecentPid = selectedPid[i];
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
      mostRecentPid = global;
      selectedBlock = -1;
    }else{
      int16_t value = data[1];
      if(encoders[0] != value){
	encoders[0] = value;
	user[global] = value<<ENC_MULTIPLIER; // scale encoder values up
	mostRecentPid = global;
	selectedBlock = -1;
      }
    }
    if(pressed&(1<<1)){
      mode = SELECTPROGRAM;
      setErrorStatus(NO_ERROR);
    }
    if(mode == STANDARD && getErrorStatus() && getErrorMessage() != NULL)
      mode = ERROR;
  }

  void setValue(uint8_t port, int16_t value){
    user[port] = value;
    // called by MIDI cc
    // todo: reset encoder value if associated through global or selectedPid
  }

  void updateValue(uint8_t port, int16_t value){
    // parameters[port] = (parameters[port]*3 + user[port]*multiplier + value)>>2;
    // smoothing at apprx 50Hz
    parameters[port] = max(0, min(4095, (parameters[port] + user[port] + value)>>1));
  }

  void updateOutput(uint8_t port, int16_t value){
    parameters[port] = max(0, min(4095, (((parameters[port] + (user[port]*value))>>12)>>1)));
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
