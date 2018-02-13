#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include "basicmaths.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "OpenWareMidiControl.h"
#include "Owl.h"

void defaultDrawCallback(uint8_t* pixels, uint16_t width, uint16_t height);

#define ENC_MULTIPLIER 6 // shift left by this many steps

/* shows a single parameter selected and controlled with a single encoder
 */
template<uint8_t SIZE>
class ParameterController {
public:
  char title[11] = "Prism";
  int16_t parameters[SIZE];
  int16_t encoders[2]; // last seen encoder values
  int16_t offsets[2]; // last seen encoder values
  int16_t user[SIZE]; // user set values (ie by encoder or MIDI)
  char names[SIZE][12];
  int8_t selected = 0;
  uint8_t selectedPid[2];

  enum ScreenMode {
    STANDARD, SELECTGLOBALPARAMETER, SELECTPROGRAM, ERROR
  };
  ScreenMode mode;
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
    for(int i=0; i<2; ++i){
      // encoders[i] = 0;
      offsets[i] = 0;
    }
    selectedPid[0] = PARAMETER_A;
    selectedPid[1] = 0;
    mode = STANDARD;
  }

  void draw(uint8_t* pixels, uint16_t width, uint16_t height){
    ScreenBuffer screen(width, height);
    screen.setBuffer(pixels);
    draw(screen);
  }

  void draw(ScreenBuffer& screen){
    // screen.clear();
    screen.setTextWrap(false);
    switch(mode){
    case STANDARD:
      screen.fill(BLUE);
      drawParameter(selectedPid[0], 54, screen);
      // use callback to draw title and message
      drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
      break;
    case SELECTGLOBALPARAMETER:
      screen.fill(GREEN);
      drawTitle(screen);
      drawGlobalParameterNames(screen);
      break;
    case SELECTPROGRAM:
      screen.fill(RED);
      drawTitle("Prism", screen);
      drawMessage(screen);
      drawStats(screen);
      // todo!
      // select: Scope, VU Meter, Patch Stats, Set Volume, Show MIDI, Reset Patch, Select Patch...
      break;
    case ERROR:
      screen.fill(CYAN);
      drawTitle("ERROR", screen);
      drawError(screen);
      drawStats(screen);
      break;
    }
#if 0      
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
#endif
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

  void drawParameter(int pid, int y, ScreenBuffer& screen){
    int x = 0;
    screen.setTextSize(1);
    screen.print(x, y, names[pid]);
    // 6px high by up to 96px long rectangle
    // y -= 7;
    // x += 64;
    screen.drawRectangle(x, y, max(1, min(95, parameters[pid]/42)), 6, WHITE);
  }

    void drawError(ScreenBuffer& screen){
    if(getErrorMessage() != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, 26, getErrorMessage());
      screen.setTextWrap(false);
    }
  }

  void drawStats(ScreenBuffer& screen){
    screen.setTextSize(1);
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL)
      screen.print(2, 36, pv->message);
    screen.print(2, 46, "cpu/mem: ");
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/35);
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
  }

  int16_t getEncoderValue(uint8_t eid){
    return (encoders[eid] - offsets[eid]) << ENC_MULTIPLIER;
    // value<<ENC_MULTIPLIER; // scale encoder values up
  }

  void setEncoderValue(uint8_t eid, int16_t value){
    offsets[eid] = encoders[eid] - (value >> ENC_MULTIPLIER);
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

  void setValue(uint8_t ch, int16_t value){
    parameters[ch] = value;
  }
    void drawTitle(ScreenBuffer& screen){
    drawTitle(title, screen);
  }

  void drawMessage(ScreenBuffer& screen){
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL){
      screen.setTextSize(1);
      screen.setTextWrap(true);
      screen.print(0, 26, pv->message);
    }    
  }

  void drawTitle(const char* title, ScreenBuffer& screen){
    // draw title
    screen.setTextSize(2);
    screen.print(0, 16, title);
  }

  void setCallback(void *callback){
    if(callback == NULL)
      drawCallback = defaultDrawCallback;
    else
      drawCallback = (void (*)(uint8_t*, uint16_t, uint16_t))callback;
  }

  void updateValue(uint8_t pid, int16_t value){
    // smoothing at apprx 50Hz
    parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + value)>>1));
  }

  void updateEncoders(int16_t* data, uint8_t size){
    mode = STANDARD;

    int16_t value = data[0];
    if(getButtonValue(BUTTON_B)){
      // update selected global parameter
      // TODO: add 'special' parameters: Volume, Freq, Gain, Gate
      mode = SELECTGLOBALPARAMETER;
      int16_t delta = value - encoders[0];
      if(delta < 0)
	selectGlobalParameter(selectedPid[0]-1);
      else if(delta > 0)
	selectGlobalParameter(selectedPid[0]+1);
      // selectedBlock = 0;
    }else{
      if(encoders[0] != value){
	user[selectedPid[0]] = getEncoderValue(0);
	// selectedBlock = 0;
      }
    }
    encoders[0] = value;

    value = data[1];
    if(getButtonValue(BUTTON_A)){
      mode = SELECTPROGRAM;
      setErrorStatus(NO_ERROR);
    }
    encoders[1] = value;

    if(mode == STANDARD && getErrorStatus() && getErrorMessage() != NULL)
      mode = ERROR;
  }

  void encoderChanged(uint8_t encoder, int32_t delta){
#if 0
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
#endif  
  }

  void selectGlobalParameter(int8_t pid){
    selectedPid[0] = max(0, min(SIZE-1, pid));
    setEncoderValue(0, user[selectedPid[0]]);
  }

private:
  void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
  bool sw1(){
    return HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_14) != GPIO_PIN_SET;
  }
  bool sw2(){
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) != GPIO_PIN_SET;
  }
};

#endif // __ParameterController_hpp__
