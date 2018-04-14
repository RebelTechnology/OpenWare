#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include "device.h"
#include "basicmaths.h"
#include "ProgramVector.h"
// #include "HAL_Encoders.h"
#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "errorhandlers.h"

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);

// #define ENC_MULTIPLIER 12 // shift left by this many steps
/*    
screen 128 x 64, font 5x7
*/
template<uint8_t SIZE>
class ParameterController {
public:
  uint16_t ENC_MULTIPLIER = 2;

  char title[11] = "FX BOX";
  int16_t parameters[SIZE];
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t offsets[NOF_ENCODERS]; // last seen encoder values
  int16_t user[SIZE]; // user set values (ie by encoder or MIDI)
  char names[SIZE][12];
  uint8_t selectedPid[NOF_ENCODERS];
  enum DisplayMode {
    STANDARD, SELECTGLOBALPARAMETER, SELECTPROGRAM, ERROR
  };
  DisplayMode displayMode;

  ParameterController(){
    reset();
  }
  void reset(){
    drawCallback = defaultDrawCallback;
    for(int i=0; i<SIZE; ++i){
      strcpy(names[i], "Parameter ");
      names[i][9] = 'A'+i;
      user[i] = 0;
      parameters[i] = 0;
    }
    for(int i=0; i<NOF_ENCODERS; ++i){
      // encoders[i] = 0;
      offsets[i] = 0;
    }
    selectedPid[0] = PARAMETER_A;
    selectedPid[1] = PARAMETER_B;
    displayMode = STANDARD;
  }
 
  int16_t getEncoderValue(uint8_t eid){
    return (encoders[eid] - offsets[eid]) << ENC_MULTIPLIER;
    // value<<ENC_MULTIPLIER; // scale encoder values up
  }

  void setEncoderValue(uint8_t eid, int16_t value){
    offsets[eid] = encoders[eid] - (value >> ENC_MULTIPLIER);
  }

  void draw(uint8_t* pixels, uint16_t width, uint16_t height){
    ScreenBuffer screen(width, height);
    screen.setBuffer(pixels);
    draw(screen);
  }

  void drawParameter(int pid, int y, ScreenBuffer& screen){
    int x = 0;
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
    // 6px high by up to 128px long rectangle
    y -= 7;
    x += 64;
    screen.drawRectangle(x, y, max(1, min(64, parameters[pid]/64)), 6, WHITE);
  }

  void drawGlobalParameterNames(ScreenBuffer& screen){
    screen.setTextSize(1);
    if(selectedPid[0] > 0)
      screen.print(1, 24, names[selectedPid[0]-1]);
    screen.print(1, 24+10, names[selectedPid[0]]);
    if(selectedPid[0] < SIZE-1)
      screen.print(1, 24+20, names[selectedPid[0]+1]);
    screen.invert(0, 25, 128, 10);
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
    screen.print(110, 8, "cpu");
    screen.setCursor(110, 17);
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

  void drawTitle(ScreenBuffer& screen){
    drawTitle(title, screen);
  }

  void drawTitle(const char* title, ScreenBuffer& screen){
    // draw title
    screen.setTextSize(2);
    screen.print(0, 16, title);
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

  void draw(ScreenBuffer& screen){
    screen.clear();
    screen.setTextWrap(false);
    switch(displayMode){
    case STANDARD:
      drawParameter(selectedPid[0], 34, screen);
      drawParameter(selectedPid[1], 54, screen);

      // use callback to draw title and message
      drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
      break;
    case SELECTGLOBALPARAMETER:
      drawTitle(screen);
      drawGlobalParameterNames(screen);
      break;
    case SELECTPROGRAM:
      drawTitle("FX", screen);
      drawStats(screen);
      drawMessage(46, screen);
      // todo!
      // select: Scope, VU Meter, Patch Stats, Set Volume, Show MIDI, Reset Patch, Select Patch...
      break;
    case ERROR:
      drawTitle("ERROR", screen);
      drawError(screen);
      drawStats(screen);
      break;
    }
  }

  void setName(uint8_t pid, const char* name){
    if(pid < SIZE){
      strncpy(names[pid], name, 11);
    }
  }

  void setTitle(const char* str){
    strncpy(title, str, 10);    
  }

  uint8_t getSize(){
    return SIZE;
  }

  void selectGlobalParameter(int8_t pid){
    selectedPid[0] = max(0, min(SIZE-1, pid));
    setEncoderValue(0, user[selectedPid[0]]);
  }
  
  void updateEncoders(int16_t* data, uint8_t size){
    displayMode = STANDARD;
    // update encoder 0 top left
    int16_t value = data[0];
    if(encoders[0] != value){
      user[selectedPid[0]] = getEncoderValue(0);
      encoders[0] = value;
    }
    // update encoder 1 top right
    value = data[1];
    if(encoders[1] != value){
      user[selectedPid[1]] = getEncoderValue(1);
      encoders[1] = value;
    }
    if(displayMode == STANDARD && getErrorStatus() && getErrorMessage() != NULL)
      displayMode = ERROR;
  }

  // called by MIDI cc and/or from patch
  void setValue(uint8_t pid, int16_t value){
    user[pid] = value;
    // reset encoder value if associated through selectedPid to avoid skipping
    for(int i=0; i<NOF_ENCODERS; ++i)
      if(selectedPid[i] == pid)
        setEncoderValue(i, value);
    // TODO: store values set from patch somewhere and multiply with user[] value for outputs
    // graphics.params.updateOutput(i, getOutputValue(i));
  }

  // @param value is the modulation ADC value
  void updateValue(uint8_t pid, int16_t value){
    // smoothing at apprx 50Hz
    parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + value)>>1));
  }

  void updateOutput(uint8_t pid, int16_t value){
    parameters[pid] = max(0, min(4095, (((parameters[pid] + (user[pid]*value))>>12)>>1)));
  }

  // void updateValues(uint16_t* data, uint8_t size){
    // for(int i=0; i<16; ++i)
    //   parameters[selectedPid[i]] = encoders[i] + data[i];
  // }

  void encoderChanged(uint8_t encoder, int32_t delta){
  }

  void setCallback(void *callback){
    if(callback == NULL)
      drawCallback = defaultDrawCallback;
    else
      drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
  }
  void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
};

#endif // __ParameterController_hpp__
