#ifndef __GeniusParameterController_hpp__
#define __GeniusParameterController_hpp__

#include <stdint.h>
#include <string.h>
#include "device.h"
#include "errorhandlers.h"
#include "ProgramVector.h"
#include "VersionToken.h"
#include "ScreenBuffer.h"
#include "Owl.h"
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

/** 
 *shows a single parameter selected and controlled with a single encoder
 */
class GeniusParameterController : public ParameterController {
public:
  char names[NOF_PARAMETERS][12];
  int8_t selected = 0;
  int16_t encoders[NOF_ENCODERS]; // last seen encoder values
  int16_t user[NOF_PARAMETERS]; // user set values (ie by encoder or MIDI)
  GeniusParameterController(){
    encoders[0] = INT16_MAX/2;
    encoders[1] = INT16_MAX/2;
    reset();
  }
  void reset(){
    setTitle("Genius");
    for(int i=0; i<NOF_PARAMETERS; ++i){
      strcpy(names[i], "Parameter  ");
      names[i][10] = 'A'+i;
      parameters[i] = 0;
      user[i] = 0;
    }
  }

  void draw(ScreenBuffer& screen){
    screen.clear();
    if(sw1()){
      drawStats(screen);
      // todo: show patch name and next/previous patch
    }else if(sw2()){
      screen.fill(BLACK);
      drawGlobalParameterNames(42, screen);
    }else if(getErrorStatus() != NO_ERROR && getErrorMessage() != NULL){
      screen.setTextSize(1);
      screen.print(2, 20, getErrorMessage());
    }else{
      graphics.drawCallback(screen.getBuffer(), screen.getWidth(), screen.getHeight());
      screen.setTextSize(1);
      screen.print(2, 56, names[selected]);
      screen.print(": ");
      screen.print((int)parameters[selected]/41);
    }
  }

  void drawParameterNames(int y, int pid, const char names[][12], int size, ScreenBuffer& screen){
    screen.setTextSize(1);
    // int selected = selectedPid[pid];
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
    drawParameterNames(y, 0, names, NOF_PARAMETERS, screen);
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

  void updateEncoders(int16_t* data, uint8_t size){
    if(data[0] != encoders[0]){
      encoderChanged(0, data[0] - encoders[0]);
      encoders[0] = data[0];
    }
    if(data[1] != encoders[1]){
      encoderChanged(1, data[1] - encoders[1]);
      encoders[1] = data[1];
    }
  }

  void encoderChanged(uint8_t encoder, int32_t delta){
    if(encoder == 1){
      if(sw2()){
	if(delta > 1)
	  selected = min(NOF_PARAMETERS-1, selected+1);
	else if(delta < 1)
	  selected = max(0, selected-1);
      }else{
	// single clicks have delta +/- 2
	// parameters[selected] += delta*4095/200;	
	if(delta > 0)
	  parameters[selected] += 20 << (delta/2);
	else
	  parameters[selected] -= 20 << (-delta/2);
	parameters[selected] = min(4095, max(0, parameters[selected]));
      }
    } // todo: change patch with enc1/sw1
  }
  void setName(uint8_t pid, const char* name){
    if(pid < NOF_PARAMETERS)
      strncpy(names[pid], name, 11);
  }
  void setValue(uint8_t ch, int16_t value){
    parameters[ch] = value;
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

  // @param value is the modulation ADC value
  void updateValue(uint8_t pid, int16_t value){
    // smoothing at apprx 50Hz
    parameters[pid] = max(0, min(4095, (parameters[pid] + user[pid] + value)>>1));
  }

  int16_t getValue(uint8_t pid){
    return parameters[pid];
  }

private:
  bool sw1(){
    return HAL_GPIO_ReadPin(ENC1_SW_GPIO_Port, ENC1_SW_Pin) != GPIO_PIN_SET;
  }
  bool sw2(){
    return HAL_GPIO_ReadPin(ENC2_SW_GPIO_Port, ENC2_SW_Pin) != GPIO_PIN_SET;
  }
};

#endif // __GeniusParameterController_hpp__
