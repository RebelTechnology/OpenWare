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
#include "Codec.h"
#include "ProgramManager.h"
#include "PatchRegistry.h"
#include "ApplicationSettings.h"
#include "cmsis_os.h"

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
#define ENCODER_TOP 0
#define ENCODER_BOT 1
#define ENCODER_BUTTON_TOP 1
#define ENCODER_BUTTON_BOT 2
#define ENCODER_BUTTON_ANY (ENCODER_BUTTON_TOP | ENCODER_BUTTON_BOT)
#define GENIUS_ADC_OFFSET (-120)

enum DisplayMode {
  STANDARD_DISPLAY_MODE,
  PROGRESS_DISPLAY_MODE,
  SELECT_ONE_DISPLAY_MODE,
  SELECT_TWO_DISPLAY_MODE,
  EXIT_DISPLAY_MODE,
  ASSIGN_DISPLAY_MODE,
  ERROR_DISPLAY_MODE,
  CONFIG_STATS_DISPLAY_MODE,
  CONFIG_PATCH_DISPLAY_MODE,
  CONFIG_VOLUME_DISPLAY_MODE
};
// DisplayMode displayMode;
void setDisplayMode(DisplayMode mode);

static int16_t encoder_sensitivity = 2;
static int16_t encoder_mask = 0x03;

static bool sw1(){
  return HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin) != GPIO_PIN_SET;
}
static bool sw2(){
  return HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin) != GPIO_PIN_SET;
}

class Page {
protected:
  // static constexpr int16_t encoder_sensitivity = 1;
  // static constexpr int16_t encoder_mask = 0x01;
public:
  virtual void draw(ScreenBuffer& screen){}
  // virtual void updateEncoders(int16_t* data, uint8_t size){}
  virtual void enter(){}
  virtual void exit(){}
  virtual void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){}
  virtual void buttonsChanged(uint32_t current, uint32_t previous){}
  int16_t getDiscreteEncoderValue(int16_t current, int16_t previous){
    int32_t delta = (current - previous); // * encoder_sensitivity;
    if(delta > 0 && (current & encoder_mask) == encoder_mask)
      return 1;
    if(delta < 0 && (current & encoder_mask) == encoder_mask)
      return -1;
    return 0;
  }
  int16_t getContinuousEncoderValue(int16_t current, int16_t previous, int16_t scale = 20){
    int32_t delta = (current - previous) * encoder_sensitivity;    
    if(delta > 0)
      delta = scale << (delta/2);
    else
      delta = -scale << (-delta/2);
    return delta;
  }
  static bool pressed(uint32_t bid, uint32_t current, uint32_t previous){
    return (current & bid) && !(previous & bid);
  }
  static bool released(uint32_t bid, uint32_t current, uint32_t previous){
    return !(current & bid) && (previous & bid);
  }
};

class GeniusParameterController : public ParameterController {
private:
public:
  static constexpr size_t NOF_CV_VALUES = 4;
  Page* page;
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t user[NOF_CV_VALUES]; // user set values (ie by encoder or MIDI)
  int8_t cv_assign[NOF_CV_VALUES];
  uint32_t buttons;
  GeniusParameterController() {
    setDisplayMode(PROGRESS_DISPLAY_MODE);
    // encoders[0] = INT16_MAX/2;
    // encoders[1] = INT16_MAX/2;
    encoders[0] = 0;
    encoders[1] = 0;
  }
  void reset();
  uint8_t getAssignedCV(uint8_t cv){
    return cv_assign[cv];
  }
  void setAssignedCV(uint8_t cv, uint8_t pid){
    if(cv_assign[cv] != pid){
      parameters[cv_assign[cv]] = user[cv];
      user[cv] = parameters[pid];
      cv_assign[cv] = pid;
    }
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
  uint32_t readButtons(){
    return sw1() | (sw2() << 1);
  }
  void updateEncoders(int16_t* data, uint8_t size){
    uint32_t bstate = readButtons();
    if(bstate != buttons){
      page->buttonsChanged(bstate, buttons);
      buttons = bstate;
    }
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
  int16_t getEncoderValue(uint8_t encoder){
    return user[encoder];
  }
  void updateEncoderValue(uint8_t select, int16_t delta){
    if(select == cv_assign[0]){
      user[0] = std::clamp(user[0] + delta, 0, 4095);
    }else if(select == cv_assign[1]){
      user[1] = std::clamp(user[1] + delta, 0, 4095);
    }else{      
      parameters[select] = std::clamp(parameters[select] + delta, 0, 4095);
    }
  }
  void setName(uint8_t pid, const char* name){
    ParameterController::setName(pid, name);
    if(isInput(pid)){
      if(cv_assign[0] == 0 && !isInput(0))
  	cv_assign[0] = pid;
      else if(cv_assign[1] == 0 && cv_assign[0] != pid)
  	cv_assign[1] = pid;
    }else if(isOutput(pid)){
      if(cv_assign[2] == 0)
  	cv_assign[2] = pid;
      else if(cv_assign[3] == 0 && cv_assign[2] != pid)
  	cv_assign[3] = pid;
    }      
  }    
  void setValue(uint8_t pid, int16_t value){    
    if(pid == cv_assign[0]){
      user[0] = value;
    }else if(pid == cv_assign[1]){
      user[1] = value;
    }else{
      parameters[pid] = value;
    }
  }
  void updateValues(int16_t* values, size_t len){
    parameters[cv_assign[0]] = std::clamp(user[0] + values[0] + GENIUS_ADC_OFFSET, 0, 4095);
    parameters[cv_assign[1]] = std::clamp(user[1] + values[1] + GENIUS_ADC_OFFSET, 0, 4095);
  }
  void drawTitle(const char* title, ScreenBuffer& screen){
    if(strnlen(title, 11) < 11)
      screen.setTextSize(2);
    else
      screen.setTextSize(1);
    screen.print(0, 16, title);
  }
};

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
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(released(ENCODER_BUTTON_ANY, current, previous)){
      if(counter < TOGGLE_LIMIT){
	setDisplayMode(ctrl == 0 ? CONFIG_STATS_DISPLAY_MODE : ASSIGN_DISPLAY_MODE);
      }else{
	setDisplayMode(STANDARD_DISPLAY_MODE);
      }
    }
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ctrl)
      select = std::clamp(select + getDiscreteEncoderValue(current, previous), 0, NOF_PARAMETERS-1);
  }
  void draw(ScreenBuffer& screen){
    counter++;
    drawParameterNames(12, select, screen);
  }
  
  void drawParameterNames(int y, int pid, ScreenBuffer& screen){
    screen.setTextSize(1);
    for(int i=pid-3; i<pid+3; ++i){
      screen.print(1, y, params.getName(i));
      if(pid == i)
	screen.invert(0, y-9, screen.getWidth(), 10);
      y += 10;
    }
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
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_TOP, current, previous))
      setErrorStatus(0);
  }
  void draw(ScreenBuffer& screen){
    if(getErrorStatus() == NO_ERROR){
      setDisplayMode(STANDARD_DISPLAY_MODE);
    }else{
      params.drawTitle("ERROR", screen);
      screen.setTextSize(1);
      if(getErrorMessage() != NULL)
	screen.print(2, 26, getErrorMessage());
      if(getDebugMessage() != NULL)
	screen.print(2, 36, getDebugMessage());
    }
  }
};

SelectControlPage selectOnePage(0, 0);
SelectControlPage selectTwoPage(1, 1);

class AssignPage : public Page {
private:
  uint8_t select = 0;
  static constexpr const char* assignations[] = {"CV A In", "CV B In", "CV A Out", "CV B Out"};
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_TOP){
      select = std::clamp(select + getDiscreteEncoderValue(current, previous), 0, 3);
    }else{
      uint8_t assign = params.getAssignedCV(select);
      int16_t delta = getDiscreteEncoderValue(current, previous);
      assign = std::clamp(assign + delta, 0, NOF_PARAMETERS-1);
      if(select < 2 && delta){ // assigning input CV
	while(params.isOutput(assign))
	  assign += delta;
	if(params.isInput(assign))
	  params.setAssignedCV(select, assign);
      }else if(delta){ // assigning output CV
	while(params.isInput(assign))
	  assign += delta;
	if(params.isOutput(assign))
	  params.setAssignedCV(select, assign);	
      }      
    }
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(EXIT_DISPLAY_MODE);
  }
  void draw(ScreenBuffer& screen){
    uint8_t assign = params.getAssignedCV(select);
    params.drawTitle("ASSIGN", screen);
    screen.setTextSize(1);
    screen.print(1, 26, assignations[select]);
    screen.print(": ");
    screen.print(params.getName(assign));
  }
};

class StandardPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    int16_t value = getContinuousEncoderValue(current, previous);
    uint8_t select = encoder == ENCODER_TOP ? selectOnePage.select : selectTwoPage.select;
    // uint8_t select = getAssignedEncoder(encoder);
    params.updateEncoderValue(select, value);
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_TOP, current, previous))
      setDisplayMode(SELECT_ONE_DISPLAY_MODE);
    else if(pressed(ENCODER_BUTTON_BOT, current, previous))
      setDisplayMode(SELECT_TWO_DISPLAY_MODE);
  }
  void draw(ScreenBuffer& screen){
    int s1 = selectOnePage.select;
    int s2 = selectTwoPage.select;
    drawParameter(s1, 56, screen);
    drawParameter(s2, 64, screen);
    graphics.drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
  }
  
  void drawParameter(int pid, int y, ScreenBuffer& screen){
    int x = 0;
    screen.setTextSize(1);
    screen.print(x, y, params.getName(pid));
    // 6px high by up to 64px long rectangle
    y -= 7;
    x += 64;
#if 1
    screen.drawRectangle(x, y, max(1, min(64, params.getValue(pid)/64)), 6, WHITE);
    if(params.getAssignedCV(0) == pid){
      screen.fillRectangle(x, y+1, std::clamp(params.getEncoderValue(0)/64 - 2, 1, 64), 4, WHITE);
    }else if(params.getAssignedCV(1) == pid){
      screen.fillRectangle(x, y+1, std::clamp(params.getEncoderValue(1)/64 - 2, 1, 64), 4, WHITE);
    }
#else
    if(params.getAssignedCV(0) == pid){
      screen.drawRectangle(x, y, max(1, min(64, params.getEncoderValue(0)/64)), 6, WHITE);
      screen.fillRectangle(x, y+1, max(1, min(64, params.getValue(pid)/64)), 4, WHITE);
    }else if(params.getAssignedCV(1) == pid){
      screen.drawRectangle(x, y, max(1, min(64, params.getEncoderValue(1)/64)), 6, WHITE);
      screen.fillRectangle(x, y+1, max(1, min(64, params.getValue(pid)/64)), 4, WHITE);
    }else{
      screen.drawRectangle(x, y, max(1, min(64, params.getValue(pid)/64)), 6, WHITE);
    }
#endif
  }
};

class StatsPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_TOP){
      int mode = CONFIG_STATS_DISPLAY_MODE + getDiscreteEncoderValue(current, previous);
      setDisplayMode((DisplayMode)std::clamp(mode, (int)CONFIG_STATS_DISPLAY_MODE, (int)CONFIG_VOLUME_DISPLAY_MODE));
    }else if(encoder == ENCODER_BOT){
    }
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(EXIT_DISPLAY_MODE);
  }
  void draw(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(5, 16, "  Status >");
    drawStatus(screen);
  }
  void drawStatus(ScreenBuffer& screen){
    ProgramVector* pv = getProgramVector();
    screen.setTextSize(1);
    screen.print(1, 26, "cpu/mem: ");
    float percent = (pv->cycles_per_block/pv->audio_blocksize) / (float)ARM_CYCLES_PER_SAMPLE;
    screen.print((int)(percent*100));
    screen.print("% ");
    screen.print((int)(pv->heap_bytes_used)/1024);
    screen.print("kB");
    // draw firmware version
    screen.print(1, 36, getFirmwareVersion());
    if(pv->message != NULL)
      screen.print(2, 46, pv->message);
    // if (bootloader_token->magic == BOOTLOADER_MAGIC){
    //   screen.print(" bt.");
    //   screen.print(getBootloaderVersion());
    // }
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

class SelectPatchPage : public Page {
public:
  uint8_t patchselect;
  void reset(){
    patchselect = program.getProgramIndex();
  }
  void enter(){
    reset();
  }
  // void exit(){
  //   if(isEncoderPressed(ENCODER_BUTTON_TOP) && patchselect != program.getProgramIndex() &&
  //      registry.hasPatch(patchselect)){
  //     program.loadProgram(patchselect);
  //     program.resetProgram(false);
  //   }
  // } // only change patch when exiting through ExitPatch
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_TOP){
      reset();
      int mode = CONFIG_PATCH_DISPLAY_MODE + getDiscreteEncoderValue(current, previous);
      setDisplayMode((DisplayMode)std::clamp(mode, (int)CONFIG_STATS_DISPLAY_MODE, (int)CONFIG_VOLUME_DISPLAY_MODE));
    }else if(encoder == ENCODER_BOT){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      uint8_t pid = patchselect + delta;
      while(!(registry.hasPatch(pid) || pid == program.getProgramIndex())
	    && pid < registry.getNumberOfPatches() && delta)
	pid += delta;
      if(registry.hasPatch(pid) || pid == program.getProgramIndex())
	patchselect = pid;
    }
  }
  void draw(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(5, 16, "< Patch  >");
    drawPresetNames(patchselect, screen);
  }
  static int8_t prev(int8_t selected){
    for(int8_t i=selected-1; i>=0; --i){
      if(registry.hasPatch(i) || program.getProgramIndex() == i)
	return i;
    }
    return -1;
  }
  static int8_t next(int8_t selected){
    if(selected != -1){
      for(uint8_t i=selected+1; i<=registry.getNumberOfPatches(); ++i){
	if(registry.hasPatch(i))
	  return i;
      }
    }
    return -1;
  }
  void drawPresetNames(uint8_t selected, ScreenBuffer& screen){
    screen.setTextSize(1);
    int8_t pids[7];
    pids[0] = prev(prev(selected));
    pids[1] = prev(selected);
    pids[2] = selected;
    pids[3] = next(selected);
    pids[4] = next(pids[3]);
    pids[5] = next(pids[4]);
    pids[6] = next(pids[5]);
    int y = 24;
    for(int i=0; i < 7 && y < 65; ++i){
      int8_t pid = pids[i];
      if(pid != -1){
	screen.setCursor(1, y);
	screen.print((int)pid);
	screen.print(".");
	screen.print(registry.getPatchName(pid));
	if(pid == selected)
	  screen.invert(0, y-9, screen.getWidth(), 10);
	y += 10;
      }
    }
  }
};

class ConfigVolumePage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_TOP){
      int mode = CONFIG_VOLUME_DISPLAY_MODE + getDiscreteEncoderValue(current, previous);
      setDisplayMode((DisplayMode)std::clamp(mode, (int)CONFIG_STATS_DISPLAY_MODE, (int)CONFIG_VOLUME_DISPLAY_MODE));
    }else if(encoder == ENCODER_BOT){
      int16_t value = std::clamp(settings.audio_output_gain + getContinuousEncoderValue(current, previous, 1), 0, 127);
      if(settings.audio_output_gain != value){
	settings.audio_output_gain = value;
	codec.setOutputGain(value);
      }
    }
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(EXIT_DISPLAY_MODE);
  }
  void draw(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(5, 16, "< Volume");
    drawVolume(screen);
  }
  void drawVolume(ScreenBuffer& screen){
    screen.setTextSize(1);
    screen.print(1, 16 + 10, "Volume ");
    screen.print((int)settings.audio_output_gain);
    screen.drawRectangle(64, 16 + 1, 64, 8, WHITE);
    screen.fillRectangle(64, 16 + 1 + 2, (int)settings.audio_output_gain >> 1, 4, WHITE);
    // screen.invert(0, 16, 40, 10);
  }
};

class ExitPage : public Page {
public:
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(released(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(STANDARD_DISPLAY_MODE);
  }
  void draw(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(40, 26, "exit");
  }
};

StandardPage standardPage;
ProgressPage progressPage;
ErrorPage errorPage;
ExitPage exitPage;
AssignPage assignPage;
StatsPage statsPage;
SelectPatchPage selectPatchPage;
ConfigVolumePage configVolumePage;

void setDisplayMode(DisplayMode mode){
  switch(mode){
  case STANDARD_DISPLAY_MODE:
    params.changePage(&standardPage);
    break;
  case CONFIG_STATS_DISPLAY_MODE:
    params.changePage(&statsPage);
    break;
  case CONFIG_PATCH_DISPLAY_MODE:
    params.changePage(&selectPatchPage);
    break;
  case CONFIG_VOLUME_DISPLAY_MODE:
    params.changePage(&configVolumePage);
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
  case ASSIGN_DISPLAY_MODE:
    params.changePage(&assignPage);
    break;
  case ERROR_DISPLAY_MODE:
    params.changePage(&errorPage);
    break;
  case EXIT_DISPLAY_MODE: {
    params.changePage(&exitPage);
    uint8_t patchselect = selectPatchPage.patchselect;
    if(patchselect != program.getProgramIndex() &&
       registry.hasPatch(patchselect)){
      program.exitProgram(false);
      while(program.isProgramRunning())
	vTaskDelay(2);
      program.loadProgram(patchselect);
      program.startProgram(false);
    }
    break;
  }
  }
}

void GeniusParameterController::reset(){
  setTitle("Genius");
  ParameterController::reset();
  for(size_t i=0; i<NOF_CV_VALUES; ++i){
    user[i] = 0;
    cv_assign[i] = 0;
  }
  buttons = readButtons();
  selectOnePage.select = 0;
  selectTwoPage.select = 1;
}

#endif // __GeniusParameterController_hpp__
