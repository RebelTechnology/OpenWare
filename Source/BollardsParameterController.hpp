#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "OpenWareMidiControl.h"
#include "Owl.h"
#include "message.h"

// #include "AudioDisplay.hpp"

#ifdef USE_DIGITALBUS
#include "bus.h"
#endif

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);

#define ENC_MULTIPLIER 6 // shift left by this many steps
#define MAX_NAME_LEN 16

/* shows a single parameter selected and controlled with a single encoder
 */
template<uint8_t SIZE>
class ParameterController {
public:
  char title[MAX_NAME_LEN] = "Prism";
  int16_t parameters[SIZE];
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t offsets[NOF_ENCODERS];
  int16_t user[SIZE]; // user set values (ie by encoder or MIDI)
  char names[SIZE][MAX_NAME_LEN];
  int8_t selectedEnc;
  uint8_t selectedPid[NOF_ENCODERS];
#define PROGRAM_COUNT 5
  const char programnames[PROGRAM_COUNT][MAX_NAME_LEN] = { "< < <", "Stats", "MIDI", "Bus", "Reset"};
    
  enum ScreenMode {
    MODE_STANDARD, SELECTGLOBALPARAMETER, SELECTPROGRAM, MODE_ERROR
  };
  ScreenMode mode;

  enum DisplayMode {
    DISPLAY_STANDARD,
    DISPLAY_VUMETER,
    DISPLAY_SCOPE,
    DISPLAY_STATS_PATCH,
    DISPLAY_STATS_MIDI,
    DISPLAY_STATS_BUS
  };
  DisplayMode display;

  ParameterController(){
    reset();
  }
  void reset(){
    drawCallback = defaultDrawCallback;
    for(int i=0; i<SIZE; ++i){
      strcpy(names[i], "Parameter  ");
      names[i][10] = 'A'+i;
      parameters[i] = 0;
      user[i] = 0;
    }
    for(int i=0; i<NOF_ENCODERS; ++i)
      setEncoderValue(i, 0);
    selectedPid[0] = PARAMETER_A;
    selectedPid[1] = PARAMETER_B;
    selectedEnc = 0;
    mode = MODE_STANDARD;
    display = DISPLAY_STANDARD;
  }

  void draw(uint8_t* pixels, uint16_t width, uint16_t height){
    ScreenBuffer screen(width, height);
    screen.setBuffer(pixels);
    draw(screen);
  }

  void drawBusStats(int y, ScreenBuffer& screen){
#ifdef USE_DIGITALBUS
    extern int busstatus;
    extern uint32_t bus_tx_packets;
    extern uint32_t bus_rx_packets;
    screen.setTextSize(1);
    screen.setCursor(2, y);
    switch(busstatus){
    case BUS_STATUS_IDLE:
      screen.print("idle");
      break;
    case BUS_STATUS_DISCOVER:
      screen.print("disco");
      break;
    case BUS_STATUS_CONNECTED:
      screen.print("conn");
      break;
    case BUS_STATUS_ERROR:
      screen.print("err");
      break;
    }
    y += 10;
    screen.setCursor(2, y);    
    screen.print("rx");
    screen.setCursor(16, y);
    screen.print(int(bus_rx_packets));
    y += 10;
    screen.setCursor(2, y);    
    screen.print("tx");
    screen.setCursor(16, y);
    screen.print(int(bus_tx_packets));
#endif
  }

  void drawDisplayMode(ScreenBuffer& screen){
    screen.fill(BLUE);
    switch(display){
    case DISPLAY_STANDARD:
      drawParameter(86, selectedPid[selectedEnc], screen);
      // let callback draw title and message (optionally)
      drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
      break;
    case DISPLAY_VUMETER:
      break;
    case DISPLAY_SCOPE:
      break;
    case DISPLAY_STATS_PATCH:
      drawTitle(screen);
      drawStats(26, screen);
      drawMessage(46, screen);
      break;
    case DISPLAY_STATS_BUS:
      drawBusStats(46, screen);
      break;
    case DISPLAY_STATS_MIDI:
      // drawMidiStats(26, screen);
      break;
    }
  }
  
  void draw(ScreenBuffer& screen){
    // screen.clear();
    screen.setTextWrap(false);
    switch(mode){
    case MODE_STANDARD:
      drawDisplayMode(screen);
      break;
    case SELECTGLOBALPARAMETER:
      screen.fill(BLACK);
      drawGlobalParameterNames(46, screen);
      break;
    case SELECTPROGRAM:
      screen.fill(BLACK);
      drawProgramNames(46, screen);
      break;
    case MODE_ERROR:
      screen.fill(RED);
      drawTitle("ERROR", screen);
      drawError(26, screen);
      drawMessage(46, screen);
      break;
    }
  }

  void drawParameterNames(int y, int pid, const char names[][MAX_NAME_LEN], int size, ScreenBuffer& screen){
    screen.setTextSize(1);
    int selected = selectedPid[pid];
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
    drawParameterNames(y, selectedEnc, names, SIZE, screen);
  }

  void drawProgramNames(int y, ScreenBuffer& screen){    
    drawParameterNames(y, 1, programnames, PROGRAM_COUNT, screen);
  }

  void drawParameter(int y, int pid, ScreenBuffer& screen){
    int x = 0;
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
    // 6px high by up to 96px long rectangle
    // y -= 7;
    // x += 64;
    screen.drawRectangle(x, y, max(1, min(95, parameters[pid]/42)), 6, WHITE);
  }

  void drawError(int16_t y, ScreenBuffer& screen){
    if(getErrorMessage() != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, y, getErrorMessage());
      screen.setTextWrap(false);
    }
  }

  void drawStats(int16_t y, ScreenBuffer& screen){
    screen.setTextSize(1);
    ProgramVector* pv = getProgramVector();
    screen.print(1, y, "cpu/mem ");
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/35);
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
  }

  int16_t getEncoderValue(uint8_t eid){
    return (encoders[eid] - offsets[eid]) << ENC_MULTIPLIER; // offset and scale encoder values up
  }

  void setEncoderValue(uint8_t eid, int16_t value){
    offsets[eid] = encoders[eid] - (value >> ENC_MULTIPLIER);
  }

  void setName(uint8_t pid, const char* name){
    if(pid < SIZE)
      strncpy(names[pid], name, MAX_NAME_LEN-1);
  }

  void setTitle(const char* str){
    strncpy(title, str, MAX_NAME_LEN-1);
  }

  uint8_t getSize(){
    return SIZE;
  }

  void setValue(uint8_t pid, int16_t value){
    parameters[pid] = value;
    user[pid] = value;
    // reset encoder value if associated through selectedPid to avoid skipping
    for(int i=0; i<NOF_ENCODERS; ++i)
      if(selectedPid[i] == pid)
        setEncoderValue(i, value);
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

  void updateValue(uint8_t pid, int16_t value){
    // not used by bollards - parameters[] directly set
    // smoothing at apprx 50Hz
    parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + value)>>1));
  }

  void updateEncoders(int16_t* data, uint8_t size){
    mode = MODE_STANDARD;
    updateGlobalParameter(0, getButtonValue(BUTTON_A), data[1]);
    updateGlobalParameter(1, getButtonValue(BUTTON_B), data[0]);
    if(getErrorStatus() != NO_ERROR){
      if(mode == MODE_STANDARD){
	mode = MODE_ERROR;
      }else if(mode == SELECTGLOBALPARAMETER){
	// clear error message
	setErrorStatus(NO_ERROR);
	debugMessage(NULL);
      }
    }
  }

  void updateGlobalParameter(uint8_t enc, bool pressed, int16_t value){
    if(pressed){
      mode = SELECTGLOBALPARAMETER;
      selectedEnc = enc;
      int16_t delta = value - encoders[enc];
      if(delta < 0)
	selectGlobalParameter(enc, selectedPid[enc]-1);
      else if(delta > 0)
	selectGlobalParameter(enc, selectedPid[enc]+1);
      encoders[enc] = value;
    }else{
      if(encoders[enc] != value){
	encoders[enc] = value;
 	user[selectedPid[enc]] = getEncoderValue(enc);
	parameters[selectedPid[enc]] = getEncoderValue(enc); // for bollards only: direct setting
	selectedEnc = enc;
      }
    }
  }

  void encoderChanged(uint8_t encoder, int32_t delta){
  }

  void selectProgram(int8_t pid){
    // "< < <", "Stats", "MIDI", "Bus", "Reset"
    switch(pid){
    case 0:
      display = DISPLAY_STANDARD;
      break;
    case 1:
      display = DISPLAY_STATS_PATCH;
      break;
    case 2:
      display = DISPLAY_STATS_MIDI;
      break;
    case 3:
      display = DISPLAY_STATS_BUS;
      break;
    case 4: // Reset
      display = DISPLAY_STANDARD;
      setErrorStatus(NO_ERROR);
      debugMessage(NULL);
      break;
    default:
      display = DISPLAY_STANDARD;
      break;
    }
  }

  void selectProgramParameter(int8_t pid){
    selectedPid[1] = max(0, min(PROGRAM_COUNT-1, pid));
    setEncoderValue(1, user[selectedPid[1]]);
  }
  
  void selectGlobalParameter(int8_t enc, int8_t pid){
    selectedPid[enc] = max(0, min(SIZE-1, pid));
    setEncoderValue(enc, user[selectedPid[enc]]);
  }

private:
  void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
};

#endif // __ParameterController_hpp__
