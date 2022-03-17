#ifndef __MagusParameterController_hpp__
#define __MagusParameterController_hpp__

#include <stdint.h>
#include <string.h>
#include <algorithm>
#include "device.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "HAL_Encoders.h"
#include "Owl.h"
#include "OpenWareMidiControl.h"
#include "PatchRegistry.h"
#include "Storage.h"
#include "ApplicationSettings.h"
#include "ProgramManager.h"
#include "Codec.h"
#include "message.h"
#include "VersionToken.h"
#include "ScreenBuffer.h"
#include "HAL_TLC5946.h"

#define NOF_ENCODERS 6

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

#define PORT_UNI_INPUT 1
#define PORT_UNI_OUTPUT 2
#define PORT_BI_INPUT 3
#define PORT_BI_OUTPUT 4
void setPortMode(uint8_t index, uint8_t mode);
uint8_t getPortMode(uint8_t index);

enum EncoderIdentifier {
  ENCODER_L = 0, // top left
  ENCODER_R,     // top right
  ENCODER_1,     // bottom row
  ENCODER_2,
  ENCODER_3,
  ENCODER_4,
};

enum EncoderButton {
  ENCODER_BUTTON_L =    0b00000001,
  ENCODER_BUTTON_R =    0b00000010,
  ENCODER_BUTTON_1 =    0b00000100,
  ENCODER_BUTTON_2 =    0b00001000,
  ENCODER_BUTTON_3 =    0b00010000,
  ENCODER_BUTTON_4 =    0b00100000,
  ENCODER_BUTTON_1234 = 0b00111100,
  ENCODER_BUTTON_ANY =  0b00111111,
};

enum EncoderSensitivity {
  SENS_SUPER_FINE = 1,
  SENS_FINE = 2,
  SENS_STANDARD = 3,
  SENS_COARSE = 4, 
  SENS_SUPER_COARSE = 5
};
static EncoderSensitivity encoder_sensitivity = SENS_FINE;
// Sensitivity is currently not stored in settings, but it's not reset either.

enum DisplayMode {
  STANDARD_DISPLAY_MODE,
  PROGRESS_DISPLAY_MODE,
  ERROR_DISPLAY_MODE,
  SELECT_BLOCK_DISPLAY_MODE,
  SELECT_GLOBAL_DISPLAY_MODE,
  STATUS_DISPLAY_MODE,
  CONFIG_PATCH_DISPLAY_MODE,
  CONFIG_VOLUME_DISPLAY_MODE,
  CONFIG_SENSITIVITY_DISPLAY_MODE,
  CONFIG_LEDS_DISPLAY_MODE,
  CONFIG_EXIT_DISPLAY_MODE,
};
// DisplayMode displayMode;
void setDisplayMode(uint8_t mode);

class Page {
protected:
  // static constexpr int16_t encoder_sensitivity = 1;
  static constexpr int16_t encoder_mask = 0x01;
public:
  virtual void draw(ScreenBuffer& screen){}
  virtual void enter(){}
  virtual void exit(){}
  virtual void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){}
  virtual void buttonsChanged(uint32_t current, uint32_t previous){}
  // helper functions
  static int16_t getDiscreteEncoderValue(int16_t current, int16_t previous){
    int32_t delta = (current - previous) * encoder_sensitivity;    
    if(delta > 0 && (current & encoder_mask) == encoder_mask)
      return 1;
    if(delta < 0 && (current & encoder_mask) == encoder_mask)
      return -1;
    return 0;
  }
  static int16_t getContinuousEncoderValue(int16_t current, int16_t previous, int16_t scale){
    int32_t delta = (current - previous) * encoder_sensitivity;    
    if(delta > 0)
      delta = scale << (delta/3);
    else
      delta = -scale << (-delta/3);
    return delta;
  }
  static bool pressed(uint32_t bid, uint32_t current, uint32_t previous){
    return (current & bid) && !(previous & bid);
  }
  static bool released(uint32_t bid, uint32_t current, uint32_t previous){
    return !(current & bid) && (previous & bid);
  }
};

/*    
screen 128 x 64, font 5x7
4 blocks, 32px per each, 3-4 letters each

4 bottom encoders
press once to toggle mode: update > select
turn to scroll through 4 functions
press again to select parameter: select > update

*/
class MagusParameterController : public ParameterController {
public:
  Page* page = NULL;
  uint16_t buttons;
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t user[NOF_PARAMETERS]; // user set values (ie by encoder or MIDI)
  uint8_t selectedBlock;
  uint8_t selectedPid[NOF_ENCODERS];

  MagusParameterController(){
    setDisplayMode(PROGRESS_DISPLAY_MODE);
  }

  void reset(){
    setTitle("Magus");
    ParameterController::reset();
    for(int i=0; i<NOF_PARAMETERS; ++i){
      user[i] = 0;
    }
    selectedBlock = 0;
    selectedPid[0] = PARAMETER_BA;
    selectedPid[1] = 0;
    selectedPid[2] = PARAMETER_A;
    selectedPid[3] = PARAMETER_C;
    selectedPid[4] = PARAMETER_E;
    selectedPid[5] = PARAMETER_G;
#ifdef OWL_MAGUS
    for(int i=0; i<20; ++i)
      setPortMode(i, PORT_UNI_INPUT);
#endif
  }

  void changePage(Page* page){
    if(this->page != page){
      if(this->page != NULL)
	this->page->exit();
      this->page = page;
      page->enter();
    }
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

  void drawParameter(ScreenBuffer& screen){
    drawParameter(selectedPid[selectedBlock], 54, screen);
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
    // 6px high by up to 64px long rectangle
    y -= 7;
    x += 64;
    screen.drawRectangle(x, y, max(1, min(64, parameters[pid]/64)), 6, WHITE);
    screen.fillRectangle(x, y+1, max(1, min(64, user[pid]/64)), 4, WHITE);
  }

  void drawBlocks(ScreenBuffer& screen){
    drawBlockValues(screen);
    int x = 0;
    int y = 63-8;
    for(int i=2; i<NOF_ENCODERS; ++i){
      // screen.print(x+1, y, blocknames[i-1]);
      if(selectedBlock == i)
        screen.drawHorizontalLine(x, y, 32, WHITE);
      // screen.invert(x, 63-8, 32, 8);
      // screen.invert(x, y-10, 32, 10);
      x += 32;
    }
  }

  void drawGlobalParameterNames(ScreenBuffer& screen){
    screen.setTextSize(1);
    if(selectedPid[0] > 0)
      screen.print(1, 24, names[selectedPid[0]-1]);
    screen.print(1, 24+10, names[selectedPid[0]]);
    if(selectedPid[0] < NOF_PARAMETERS-1)
      screen.print(1, 24+20, names[selectedPid[0]+1]);
    screen.invert(0, 25, 64, 10);
  }

  void drawBlockParameterNames(ScreenBuffer& screen){
    int y = 29;
    screen.setTextSize(1);
    int selected = selectedPid[selectedBlock];
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
    int block = 2;
    for(int i=0; i<16; ++i){
      // 4px high by up to 16px long rectangle, filled if selected
      if(i == selectedPid[block]){
        screen.fillRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
      }else{
        screen.drawRectangle(x, y, max(1, min(16, parameters[i]/255)), 4, WHITE);
        // screen.drawRectangle(x, y+1, max(1, min(16, user[i]/255)), 2, WHITE);
      }
      x += 16;
      if(i == 7){
        x = 0;
        y += 3;
        block = 2;
      }else if(i & 0x01){
        block++;
      }
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

  void draw(ScreenBuffer& screen){
    screen.clear();
    screen.setTextWrap(false);
    page->draw(screen);
    drawBlocks(screen);
  }

  void setName(uint8_t pid, const char* name){
    ParameterController::setName(pid, name);
    if(isOutput(pid))
      setPortMode(pid, PORT_UNI_OUTPUT);
    else
      setPortMode(pid, PORT_UNI_INPUT);
  }

  bool isEncoderPushed(EncoderButton enc){
    return buttons & enc;
  }

  void updateEncoders(int16_t* data, uint8_t size){
    if(buttons != data[0]){
      page->buttonsChanged(data[0], buttons);
      buttons = data[0];
    }
    for(size_t i=0; i<NOF_ENCODERS; ++i){
      int16_t value = data[i+1];
      if(value != encoders[i]){
	page->encoderChanged(i, value, encoders[i]);
	encoders[i] = value;
      }
    }
  }

// called by MIDI cc and/or from patch
  void setValue(uint8_t pid, int16_t value){
    user[pid] = value;
    // TODO: store values set from patch somewhere and multiply with user[] value for outputs
    // graphics.params.updateOutput(i, getOutputValue(i));
  }

  // @param value is the modulation ADC value
  void updateValue(uint8_t pid, int16_t value){
    // smoothing at apprx 50Hz
    parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + value)>>1));
  }

  int16_t getValue(uint8_t pid){
    return parameters[pid];
  }

  void updateOutput(uint8_t pid, int16_t value){
    parameters[pid] = max(0, min(4095, (((parameters[pid] + (user[pid]*value))>>12)>>1)));
  }

  void changeValue(uint8_t enc, int16_t delta){
    user[selectedPid[enc]] = std::clamp(user[selectedPid[enc]] + delta, 0, 4095);
    selectedBlock = enc;
  }

  void changeAssignGlobal(uint8_t enc, int16_t delta){
    selectedPid[enc] = std::clamp(selectedPid[enc] + delta, 0, NOF_PARAMETERS-1);
    selectedBlock = enc;
  }

  void changeAssignBlock(uint8_t enc, int16_t delta){
    int8_t pid = selectedPid[enc] + delta;
    uint8_t i = enc-2;
    if(pid == i*2+2)
      pid = i*2+8; // skip up
    if(pid == i*2+7)
      pid = i*2+1; // skip down
    selectedPid[enc] = std::clamp((int)pid, i*2, i*2+9);
  }
};

extern MagusParameterController params;

class StandardPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      // Quick selection of global parameter by right encoder - without popover menu
      params.changeAssignGlobal(ENCODER_L, getDiscreteEncoderValue(current, previous));
    }else{
      params.changeValue(encoder, getContinuousEncoderValue(current, previous, 40));
    }
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    switch(current){
    case ENCODER_BUTTON_L:
      setDisplayMode(SELECT_GLOBAL_DISPLAY_MODE);
      break;
    case ENCODER_BUTTON_R:
      setDisplayMode(STATUS_DISPLAY_MODE);
      break;
    case ENCODER_BUTTON_1:
      params.selectedBlock = ENCODER_1;
      setDisplayMode(SELECT_BLOCK_DISPLAY_MODE);
      break;
    case ENCODER_BUTTON_2:
      params.selectedBlock = ENCODER_2;
      setDisplayMode(SELECT_BLOCK_DISPLAY_MODE);
      break;
    case ENCODER_BUTTON_3:
      params.selectedBlock = ENCODER_3;
      setDisplayMode(SELECT_BLOCK_DISPLAY_MODE);
      break;
    case ENCODER_BUTTON_4:
      params.selectedBlock = ENCODER_4;
      setDisplayMode(SELECT_BLOCK_DISPLAY_MODE);
      break;
    }
  }
  void draw(ScreenBuffer& screen){
    if(getErrorStatus())
      setDisplayMode(ERROR_DISPLAY_MODE);
    params.drawParameter(screen); // display last changed parameter
    // use callback to draw title and message
    graphics.drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
  }
};

class ErrorPage : public Page {
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setErrorStatus(0);      
  }
  void draw(ScreenBuffer& screen){
    if(getErrorStatus() == NO_ERROR){
      setDisplayMode(STANDARD_DISPLAY_MODE);
    }else{
      params.drawTitle("ERROR", screen);
      drawError(screen);
      drawStats(screen);
    }
  }

  // void drawError(ScreenBuffer& screen){
  //   if(getErrorMessage() != NULL){
  //     params.drawTitle("ERROR", screen);
  //     screen.setTextSize(1);
  //     screen.setTextWrap(true);
  //     screen.print(0, 26, getErrorMessage());
  //     screen.setTextWrap(false);
  //   }
  //   drawMessage(51, screen);
  // }

  void drawError(ScreenBuffer& screen){
    screen.setTextSize(2);
    screen.print(1, 16, "ERROR");
    screen.setTextSize(1);
    if(getErrorMessage() != NULL)
      screen.print(2, 25, getErrorMessage());
    if(getDebugMessage() != NULL)
      screen.print(2, 32, getDebugMessage());
  }

  void drawStats(ScreenBuffer& screen){
    int offset = 0;
    screen.setTextSize(1);
    // draw memory use

    // two columns
    screen.print(80, offset+8, "mem");
    ProgramVector* pv = getProgramVector();
    screen.setCursor(80, offset+17);
    int mem = (int)(pv->heap_bytes_used)/1024;
    if(mem > 999){
      screen.print(mem/1024);
      screen.print("M");
    }else{
      screen.print(mem);
      screen.print("k");
    }
    // draw CPU load
    screen.print(110, offset+8, "cpu");
    screen.setCursor(110, offset+17);
    screen.print((int)((pv->cycles_per_block)/pv->audio_blocksize)/(ARM_CYCLES_PER_SAMPLE/100));
    screen.print("%");
  }
};

class SelectGlobalPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_L){
      // update selected global parameter
      // TODO: add 'special' parameters: Volume, Freq, Gain, Gate
      params.changeAssignGlobal(ENCODER_L, getDiscreteEncoderValue(current, previous));
    }
  }
  void draw(ScreenBuffer& screen){
    if(!params.isEncoderPushed(ENCODER_BUTTON_L))
      setDisplayMode(STANDARD_DISPLAY_MODE);
    params.drawTitle(screen);
    params.drawGlobalParameterNames(screen);
  }
};

class SelectBlockPage : public Page {
public:
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    // if(params.isEncoderPushed(ENCODER_BUTTON_1234)){
    // if(params.isEncoderPushed(ENCODER_BUTTON_1234)){
    if(encoder == params.selectedBlock){
      // update selected block parameter
      params.changeAssignBlock(encoder, getDiscreteEncoderValue(current, previous));
    }
  }
  void draw(ScreenBuffer& screen){
    if(!params.isEncoderPushed(EncoderButton(1<<params.selectedBlock)))
      setDisplayMode(STANDARD_DISPLAY_MODE);
    params.drawTitle(screen);
    params.drawBlockParameterNames(screen);
  }
};

class EncoderSensitivityPage : public Page {
public:
 void enter(){
   // selectedPid[1] = encoder_sensitivity; // initialise control value
 }
 void exit(){}

  void draw(ScreenBuffer& screen){
    params.drawTitle("< Encode >", screen);
    int offset = 16;
    screen.setTextSize(1);
    screen.print(1, offset + 33, "Encoder sensitivity");
    screen.drawCircle(24, offset + 15, 8, WHITE);
    screen.drawCircle(44, offset + 15, 8, WHITE);
    screen.drawCircle(64, offset + 15, 8, WHITE);
    screen.drawCircle(84, offset + 15, 8, WHITE);
    screen.drawCircle(104, offset + 15, 8, WHITE);
    screen.fillCircle(24, offset + 15, 6, WHITE);
    if (encoder_sensitivity >= SENS_FINE)
      screen.fillCircle(44, offset + 15, 6, WHITE);
    if (encoder_sensitivity >= SENS_STANDARD)
      screen.fillCircle(64, offset + 15, 6, WHITE);
    if (encoder_sensitivity >= SENS_COARSE)
      screen.fillCircle(84, offset + 15, 6, WHITE);
    if (encoder_sensitivity >= SENS_SUPER_COARSE)
      screen.fillCircle(104, offset + 15, 6, WHITE);
  }
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(CONFIG_EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      setDisplayMode(std::clamp(CONFIG_SENSITIVITY_DISPLAY_MODE + delta, (int)STATUS_DISPLAY_MODE, (int)CONFIG_LEDS_DISPLAY_MODE));
    }else if(encoder == ENCODER_L){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      encoder_sensitivity = (EncoderSensitivity)std::clamp(encoder_sensitivity + delta, (int)SENS_SUPER_FINE, (int)SENS_SUPER_COARSE);
    }
  }
};

class ConfigVolumePage : public Page {
public:
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(CONFIG_EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      setDisplayMode(std::clamp(CONFIG_VOLUME_DISPLAY_MODE + delta, (int)STATUS_DISPLAY_MODE, (int)CONFIG_LEDS_DISPLAY_MODE));
    }else if(encoder == ENCODER_L){
      int16_t value = std::clamp(settings.audio_output_gain + getContinuousEncoderValue(current, previous, 1), 0, 127);
      if(settings.audio_output_gain != value){
	settings.audio_output_gain = value;
	codec.setOutputGain(value);
      }
    }
  }
  void draw(ScreenBuffer& screen){
    params.drawTitle("< Volume >", screen);    
    drawVolume(screen);
  }
  void drawVolume(ScreenBuffer& screen){
    screen.setTextSize(1);
    screen.print(1, 24 + 10, "Volume ");
    screen.print((int)settings.audio_output_gain);
    screen.drawRectangle(64, 24 + 1, 64, 8, WHITE);
    screen.fillRectangle(64, 24 + 1 + 2, (int)settings.audio_output_gain >> 1, 4, WHITE);
    screen.invert(0, 24, 40, 10);
  }
};

class ConfigLedsPage : public Page {
public:
  // void exit(){
  //   extern uint8_t ENCODER_CS_DELAY_US;
  //   ENCODER_CS_DELAY_US = settings.leds_brightness;
  // }  
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(CONFIG_EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      setDisplayMode(std::clamp(CONFIG_LEDS_DISPLAY_MODE + delta, (int)STATUS_DISPLAY_MODE, (int)CONFIG_LEDS_DISPLAY_MODE));
    }else if(encoder == ENCODER_L){
      // todo: set red/green/blue individually with different encoders
      int8_t value = std::clamp(settings.leds_brightness + getContinuousEncoderValue(current, previous, 1), 0, 63);
      if(settings.leds_brightness != value){
	settings.leds_brightness = value;
	TLC5946_setAll_DC(value);
	TLC5946_Refresh_DC();
      }
    }
  }
  void draw(ScreenBuffer& screen){
    // "  Play   >",
    // "< Status >",
    // "< Preset >",
    // "< Data   >",
    // "< Volume >",
    // "< LEDs",
    params.drawTitle("< LEDs", screen);    
    drawLeds(screen);
  }

  void drawLeds(ScreenBuffer& screen){
    screen.setTextSize(1);
    screen.print(1, 24 + 10, "Level  ");
    screen.print((int)settings.leds_brightness);
    screen.drawRectangle(64, 24 + 1, 64, 8, WHITE);
    screen.fillRectangle(64, 24 + 1 + 2, (int)settings.leds_brightness, 4, WHITE);
    screen.invert(0, 24, 40, 10);
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
  //   if(isEncoderPressed(ENCODER_BUTTON_L) && patchselect != program.getProgramIndex() &&
  //      registry.hasPatch(patchselect)){
  //     program.loadProgram(patchselect);
  //     program.resetProgram(false);
  //   }
  // } // only change patch when exiting through ExitPatch
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(CONFIG_EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      reset();
      int16_t delta = getDiscreteEncoderValue(current, previous);
      setDisplayMode(std::clamp(CONFIG_PATCH_DISPLAY_MODE + delta, (int)STATUS_DISPLAY_MODE, (int)CONFIG_LEDS_DISPLAY_MODE));
    }else if(encoder == ENCODER_L){
      patchselect = std::clamp(patchselect + getDiscreteEncoderValue(current, previous), 1, (int)registry.getNumberOfPatches()-1);
    }
  }
  void draw(ScreenBuffer& screen){
    params.drawTitle("< Patch  >", screen);
    drawPresetNames(patchselect, screen);
  }  
  void drawPresetNames(uint8_t selected, ScreenBuffer& screen){
    screen.setTextSize(1);
    selected = min(selected, (int)registry.getNumberOfPatches()-1);
    if(selected > 1) {
      screen.setCursor(1, 24);
      screen.print((int)selected - 1);
      screen.print(".");
      screen.print(registry.getPatchName(selected - 1));
    };
    screen.setCursor(1, 24+10);
    screen.print((int)selected);
    screen.print(".");
    screen.print(registry.getPatchName(selected));
    if(selected+1 < (int)registry.getNumberOfPatches()) {
      screen.setCursor(1, 24+20);
      screen.print((int)selected + 1);
      screen.print(".");
      screen.print(registry.getPatchName(selected+1));
    }
    screen.invert(0, 25, 128, 10);
  }
};

class StatusPage : public Page {
public:
  void buttonsChanged(uint32_t current, uint32_t previous){
    if(pressed(ENCODER_BUTTON_ANY, current, previous))
      setDisplayMode(CONFIG_EXIT_DISPLAY_MODE);
  }
  void encoderChanged(uint8_t encoder, int32_t current, int32_t previous){
    if(encoder == ENCODER_R){
      int16_t delta = getDiscreteEncoderValue(current, previous);
      setDisplayMode(std::clamp(STATUS_DISPLAY_MODE + delta, (int)STATUS_DISPLAY_MODE, (int)CONFIG_LEDS_DISPLAY_MODE));
    }
  }
  void draw(ScreenBuffer& screen){
    params.drawTitle("  Status >", screen);
    drawStatus(screen);
  }
  
  void drawStatus(ScreenBuffer& screen){
    int offset = 16;
    screen.setTextSize(1);
    // single row
    screen.print(1, offset+8, "mem ");
    ProgramVector* pv = getProgramVector();
    int mem = (int)(pv->heap_bytes_used)/1024;
    if(mem > 999){
      screen.print(mem/1024);
      screen.print("M");
    }else{
      screen.print(mem);
      screen.print("k");
    }

    // draw flash usage
    int flash_used = storage.getUsedSize() / 1024;
    int flash_total = storage.getTotalCapacity() / 1024;
    screen.print(64, offset + 8, "flash ");
    screen.print(flash_used * 100 / flash_total);
    screen.print("%");
    screen.setCursor(64, offset + 17);
    if (flash_used > 999) {
      screen.print(flash_used / 1024);
      screen.print(".");
      screen.print((int)((flash_used  % 1024) * 10 / 1024));
      screen.print("M/");
    }
    else {
      screen.print(flash_used);
      screen.print("k/");
    }
    if (flash_total > 999) {
      screen.print(flash_total / 1024);
      screen.print(".");
      screen.print((int)((flash_total  % 1024) * 10 / 1024));
      screen.print("M");
    }
    else {
      screen.print(flash_total);
      screen.print("k");
    }

    // draw CPU load
    screen.print(1, offset + 17, "cpu ");
    screen.print((int)((pv->cycles_per_block) / pv->audio_blocksize) / 35);
    screen.print("%");

    // draw firmware version
    screen.print(1, offset+26, getFirmwareVersion());
#ifdef DEBUG_BOOTLOADER
    if (bootloader_token->magic == BOOTLOADER_MAGIC)
      screen.print(1, offset+35, getBootloaderVersion());
#endif
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

  // void drawLoadProgress(ScreenBuffer &screen){
  //   extern char* progress_message;
  //   extern uint16_t progress_counter;
  //   // uint16_t progress_counter = user[LOAD_INDICATOR_PARAMETER];
  //   if(progress_message != NULL && progress_counter != 4095){
  //   // if(progress_counter && progress_counter != 4095){
  //     screen.drawRectangle(0, 30, 128, 20, WHITE);
  //     screen.setCursor(32, 40);
  //     screen.setTextSize(1);
  //     screen.print(progress_message);
  //     screen.fillRectangle(0, 44, progress_counter * 128 / 4095, 5, WHITE);
  //   }
  // }
};

  // const char controlModeNames[NOF_CONTROL_MODES][12] = {
  //   "  Play   >",
  //   "< Status >",
  //   "< Preset >",
  //   "< Data   >",
  //   "< Volume >",
  //   "< LEDs   >",
  //   "< V/Oct   " };

StandardPage standardPage;
ProgressPage progressPage;
SelectBlockPage selectBlockPage;
SelectGlobalPage selectGlobalPage;
StatusPage statusPage;
EncoderSensitivityPage sensitivityPage;
SelectPatchPage selectPatchPage;
ConfigVolumePage configVolumePage;
ConfigLedsPage configLedsPage;
ErrorPage errorPage;

class ExitPage : public Page {
public:
  void enter(){
    uint8_t patchselect = selectPatchPage.patchselect;
    if(patchselect != program.getProgramIndex() &&
       registry.hasPatch(patchselect)){
      program.loadProgram(patchselect);
      program.resetProgram(false);
    }
  } // todo: save settings if exited with ENCODER_BUTTON_R
  void draw(ScreenBuffer& screen){
    if(!params.isEncoderPushed(ENCODER_BUTTON_ANY)){
      // leave configuration mode on release
      setDisplayMode(STANDARD_DISPLAY_MODE);
    }else{
      screen.setTextSize(2);
      screen.print(40, 26, "exit");
    }
  }
};

ExitPage exitPage;


void setDisplayMode(uint8_t mode){
  switch(mode){
  case STANDARD_DISPLAY_MODE:
    params.changePage(&standardPage);
    break;
  case PROGRESS_DISPLAY_MODE:
    params.changePage(&progressPage);
    break;
  case SELECT_BLOCK_DISPLAY_MODE:
    params.changePage(&selectBlockPage);
    break;
  case SELECT_GLOBAL_DISPLAY_MODE:
    params.changePage(&selectGlobalPage);
    break;
  case STATUS_DISPLAY_MODE:
    params.changePage(&statusPage);
    break;
  case CONFIG_PATCH_DISPLAY_MODE:
    params.changePage(&selectPatchPage);
    break;
  case CONFIG_VOLUME_DISPLAY_MODE:
    params.changePage(&configVolumePage);
    break;
  case CONFIG_SENSITIVITY_DISPLAY_MODE:
    params.changePage(&sensitivityPage);
    break;
  case CONFIG_LEDS_DISPLAY_MODE:
    params.changePage(&configLedsPage);
  break;
  case CONFIG_EXIT_DISPLAY_MODE:
    params.changePage(&exitPage);
    break;
  case ERROR_DISPLAY_MODE:
    params.changePage(&errorPage);
    break;
  }
}

#endif // __MagusParameterController_hpp__
