#ifndef __GeniusParameterController_hpp__
#define __GeniusParameterController_hpp__

#include <stdint.h>
#include <string.h>
#include <algorithm>
#include "device.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "VersionToken.h"
#include "ScreenBuffer.h"
#include "Owl.h"
#include "message.h"
#ifdef USE_DIGITALBUS
#include "DigitalBusReader.h"
extern DigitalBusReader bus;
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

extern VersionToken* bootloader_token;

#define NOF_ENCODERS 2
#define GENIUS_ADC_OFFSET (-120)

enum DisplayMode {
  STANDARD_DISPLAY_MODE, CONFIGURATION_DISPLAY_MODE, PROGRESS_DISPLAY_MODE, SELECT_ONE_DISPLAY_MODE, SELECT_TWO_DISPLAY_MODE, EXIT_DISPLAY_MODE, ASSIGN_DISPLAY_MODE, ERROR_DISPLAY_MODE
};
// DisplayMode displayMode;
void setDisplayMode(DisplayMode mode);

class Page {
protected:
  static constexpr int16_t encoder_sensitivity = 1;
  static constexpr int16_t encoder_mask = 0x01;
public:
  virtual void draw(ScreenBuffer& screen){}
  // virtual void updateEncoders(int16_t* data, uint8_t size){}
  virtual void enter(){}
  virtual void exit(){}
  virtual void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){}
  int16_t getDiscreteEncoderValue(int16_t current, int16_t previous){
    int32_t delta = (current - previous) * encoder_sensitivity;    
    if(delta > 0 && (current & encoder_mask) == encoder_mask)
      return 1;
    if(delta < 0 && (current & encoder_mask) == encoder_mask)
      return -1;
    return 0;
  }

  int16_t getContinuousEncoderValue(int16_t current, int16_t previous){
    int32_t delta = (current - previous) * encoder_sensitivity;    
    if(delta > 0)
      delta = 20 << (delta/2);
    else
      delta = -20 << (-delta/2);
    return delta;
  }
};

class GeniusParameterController : public ParameterController {
private:
public:
  Page* page;
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t user[NOF_ADC_VALUES]; // user set values (ie by encoder or MIDI)
  // for assignable CV / modulations
  // int16_t user[NOF_PARAMETERS]; // user set values (ie by encoder or MIDI)
  GeniusParameterController() {
    // encoders[0] = INT16_MAX/2;
    // encoders[1] = INT16_MAX/2;
    encoders[0] = 0;
    encoders[1] = 0;
    reset();
    setDisplayMode(PROGRESS_DISPLAY_MODE);
  }
  void reset(){
    setTitle("Genius");
    ParameterController::reset();
    for(int i=0; i<NOF_ADC_VALUES; ++i)
    // for(int i=0; i<NOF_PARAMETERS; ++i)
      user[i] = 0;
  }

  void changePage(Page* page){
    if(this->page != page){
      if(this->page != NULL)
	this->page->exit();
      this->page = page;
      page->enter();
    }
  }

  void draw(ScreenBuffer& screen){
    screen.clear();
    page->draw(screen);
  }
  void updateEncoders(int16_t* data, uint8_t size){
    if(data[0] != encoders[0]){
      page->encoderChanged(0, data[0], encoders[0]);
      encoders[0] = data[0];
    }
    if(data[1] != encoders[1]){
      page->encoderChanged(1, data[1], encoders[1]);
      encoders[1] = data[1];
    }
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    page->encoderChanged(encoder, current, previous);
  }
  void setValue(uint8_t pid, int16_t value){    
    if(pid < NOF_ADC_VALUES){
      user[pid] = value;
    }else{
      parameters[pid] = value;
    }
  }

  int16_t getUserValue(uint8_t ch){
    if(ch < NOF_ADC_VALUES)
      return user[ch];
    return parameters[ch];
  }
  // @param values the modulation ADC values
  void updateValues(int16_t* values, size_t len){
    for(size_t pid=0; pid<NOF_ADC_VALUES; ++pid)
      parameters[pid] = max(0, min(4095, user[pid] + values[pid] + GENIUS_ADC_OFFSET));
      // parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + values[pid])>>1));
  }
private:
};

static bool sw1(){
  return HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin) != GPIO_PIN_SET;
}
static bool sw2(){
  return HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin) != GPIO_PIN_SET;
}
static bool sw(uint8_t ctrl){
  return ctrl == 0 ? sw1() : sw2();
}

extern GeniusParameterController params;

class SelectControlPage : public Page {
  const uint8_t ctrl;
  size_t counter; // ticks since switch was pressed down
  static constexpr size_t TOGGLE_LIMIT = (400/SCREEN_LOOP_SLEEP_MS);
public:
  int8_t select;
  SelectControlPage(uint8_t ctrl, int8_t select): ctrl(ctrl), select(select){}
  void enter(){
    counter = 0;
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ctrl)
      select = std::clamp(select + getDiscreteEncoderValue(current, previous), 0, NOF_PARAMETERS-1);
    }
  void draw(ScreenBuffer& screen){
    if(!sw(ctrl)){
      // encoder switch released
      if(counter < TOGGLE_LIMIT){
	setDisplayMode(ctrl == 0 ? CONFIGURATION_DISPLAY_MODE : ASSIGN_DISPLAY_MODE);
      }else{
	setDisplayMode(STANDARD_DISPLAY_MODE);
      }
    }else{
      counter++;
    }
    drawGlobalParameterNames(42, screen);
  }
  
  void drawParameterNames(int y, int pid, int size, ScreenBuffer& screen){
    screen.setTextSize(1);
    // int select = selectPid[pid];
    if(select > 2)
      screen.print(1, y-30, params.getName(select-3));
    if(select > 1)
      screen.print(1, y-20, params.getName(select-2));
    if(select > 0)
      screen.print(1, y-10, params.getName(select-1));
    screen.print(1, y, params.getName(select));
    if(select < size-1)
      screen.print(1, y+10, params.getName(select+1));
    if(select < size-2)
      screen.print(1, y+20, params.getName(select+2));
    if(select < size-3)
      screen.print(1, y+30, params.getName(select+3));
    screen.invert(0, y-9, screen.getWidth(), 10);
  }

  void drawGlobalParameterNames(int y, ScreenBuffer& screen){
    drawParameterNames(y, 0, NOF_PARAMETERS, screen);
  }
};

class ProgressPage : public Page {
  void draw(ScreenBuffer& screen){
    drawLoadProgress(screen);
  }
  void drawLoadProgress(ScreenBuffer &screen){
    extern char* progress_message;
    extern uint16_t progress_counter;
    screen.drawRectangle(0, 30, 128, 20, WHITE);
    // if(progress_counter != 4095)
    screen.fillRectangle(0, 44, progress_counter * 128 / 4095, 5, WHITE);
    screen.setCursor(33, 40);
    screen.setTextSize(1);
    if(progress_message == NULL)
      screen.print("Loading");
    else
      screen.print(progress_message);
  }
};

class ErrorPage : public Page {
  void draw(ScreenBuffer& screen){
    if(sw1())
      setErrorStatus(0);
    if(getErrorStatus() == NO_ERROR){
      setDisplayMode(STANDARD_DISPLAY_MODE);
    }else{
      screen.setTextSize(2);
      screen.print(1, 16, "ERROR");
      screen.setTextSize(1);
      if(getErrorMessage() != NULL)
	screen.print(2, 25, getErrorMessage());
      if(getDebugMessage() != NULL)
	screen.print(2, 32, getDebugMessage());
    }
  }
};

class ExitPage : public Page {
public:
  void draw(ScreenBuffer& screen){
    if(!(sw1() || sw2())){ // leave configuration mode on release
      setDisplayMode(STANDARD_DISPLAY_MODE);
    }else{
      screen.setTextSize(2);
      screen.print(40, 26, "exit");
    }
  }
};

class StatsPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){}
  void draw(ScreenBuffer& screen){
    if(sw1() || sw2()){
      setDisplayMode(EXIT_DISPLAY_MODE);
    }else{
      drawStats(screen);
    }
  }
  void drawStats(ScreenBuffer& screen){
    screen.setTextSize(1);
    ProgramVector* pv = getProgramVector();
    if(pv->message != NULL)
      screen.print(2, 16, pv->message);
    screen.print(2, 26, "cpu/mem: ");
    float percent = (pv->cycles_per_block/pv->audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
    screen.print((int)(percent*100));
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
    // draw firmware version
    screen.print(1, 36, getFirmwareVersion());
    if (bootloader_token->magic == BOOTLOADER_MAGIC){
      screen.print(" bt.");
      screen.print(getBootloaderVersion());
    }
#ifdef USE_DIGITALBUS
    screen.print(1, 56, "Bus: ");
    screen.print(bus.getStatusString());
    screen.print(" ");
    extern uint32_t bus_tx_packets, bus_rx_packets;
    screen.print((int)bus_tx_packets);
    screen.print("/");
    screen.print((int)bus_rx_packets);
    if (bus.getStatus() == BUS_STATUS_CONNECTED) {
      screen.print(", ");
      screen.print(bus.getPeers());
      screen.print(" peers");
    }
#endif
  }
};

SelectControlPage selectOnePage(0, 0);
SelectControlPage selectTwoPage(1, 1);

class AssignPage : public Page {
private:
  uint8_t select;
  uint8_t assign;
  static constexpr const char* assignations[] = {"CV A In", "CV B In", "CV A Out", "CV B Out"};
  int16_t enc, del;
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == 0){
      select = std::clamp(select + getDiscreteEncoderValue(current, previous), 0, 3);
    }else{
      assign = std::clamp(assign + getDiscreteEncoderValue(current, previous), 0, NOF_PARAMETERS-1);
    }
    enc = current;
    del = current - previous;
  }
  void enter(){
    select = 0;
    assign = 0;
  }
  void draw(ScreenBuffer& screen){
    if(sw1() || sw2()){
      setDisplayMode(EXIT_DISPLAY_MODE);
    }else{
      screen.setTextSize(2);
      screen.print(1, 16, "ASSIGN");
      screen.setTextSize(1);
      screen.print(1, 26, assignations[select]);
      screen.print(": ");
      screen.print(params.getName(assign));
      screen.setCursor(1, 36);
      screen.print(enc);
      screen.setCursor(1, 46);
      screen.print(del);
    }
  }
};

class StandardPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    uint8_t select = encoder == 0 ? selectOnePage.select : selectTwoPage.select;
    int16_t value = getContinuousEncoderValue(current, previous);
    value = std::clamp(params.getUserValue(select) + value, 0, 4095);
    params.setValue(select, value);
  }
  void draw(ScreenBuffer& screen){
    if(sw1()){
      setDisplayMode(SELECT_ONE_DISPLAY_MODE);
    }else if(sw2()){
      setDisplayMode(SELECT_TWO_DISPLAY_MODE);
    }else{
      int s1 = selectOnePage.select;
      int s2 = selectTwoPage.select;
      screen.setTextSize(1);
      screen.print(2, 56, params.getName(s1));
      screen.print(": ");
      screen.print((int)params.getValue(s1)/41);
      screen.print(2, 64, params.getName(s2));
      screen.print(": ");
      screen.print((int)params.getValue(s2)/41);
      graphics.drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
    }
  }
};

StandardPage standardPage;
ProgressPage progressPage;
ErrorPage errorPage;
ExitPage exitPage;
StatsPage configurationPage;
AssignPage assignPage;

void setDisplayMode(DisplayMode mode){
  switch(mode){
  case STANDARD_DISPLAY_MODE:
    params.changePage(&standardPage);
    break;
  case CONFIGURATION_DISPLAY_MODE:
    params.changePage(&configurationPage);
    break;
  case PROGRESS_DISPLAY_MODE:
    params.changePage(&progressPage);
    break;
  case SELECT_ONE_DISPLAY_MODE:
    params.changePage(&selectOnePage);
    break;
  case SELECT_TWO_DISPLAY_MODE:
    params.changePage(&selectTwoPage);
    break;
  case EXIT_DISPLAY_MODE:
    params.changePage(&exitPage);
    break;
  case ASSIGN_DISPLAY_MODE:
    params.changePage(&assignPage);
    break;
  case ERROR_DISPLAY_MODE:
    params.changePage(&errorPage);
    break;
  }
}

#endif // __GeniusParameterController_hpp__
