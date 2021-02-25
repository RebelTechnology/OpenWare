#ifndef __ProgramManager_H__
#define __ProgramManager_H__

#include <inttypes.h>
#include "PatchDefinition.hpp"
#include "ProgramVector.h"
#include "Owl.h"

extern "C" {
  void updateProgramVector(ProgramVector* pv, PatchDefinition* def);
  void onProgramReady(void);
  void onProgramStatus(ProgramVectorAudioStatus status);
  void onResourceUpdate(void);
  void onAudioReady(void);
  void audioCallback(int32_t* rx, int32_t* tx, uint16_t size);
  /* void encoderChanged(uint8_t encoder, int32_t value); */
  /* uint16_t getAnalogValue(uint8_t index); */
  /* void setAnalogValue(uint8_t ch, uint16_t value); */

  /* int16_t getParameterValue(uint8_t ch); */
  /* void setParameterValue(uint8_t ch, int16_t value); */

  /* uint8_t getButtonValue(uint8_t ch); */
  /* void setButtonValue(uint8_t ch, uint8_t value); */
  /* uint16_t getSampleCounter(); */
}

class ProgramManager {
private:
  uint8_t patchindex = 0;
  PatchDefinition* patchdef = NULL;
  void notifyManager(uint32_t ulValue);
  void notifyManagerFromISR(uint32_t ulValue);
public:
  ProgramManager();

  void loadProgram(uint8_t index);
  void loadStaticProgram(PatchDefinition* def);
  void loadDynamicProgram(void* address, uint32_t length);
  void startManager();
  void runManager();
  void startProgram(bool isr);
  void exitProgram(bool isr);
  void resetProgram(bool isr); /* exit and restart program */
  void startProgramChange(bool isr);
  /* void sendMidiData(int type, bool isr); */
  
  // void audioReady();
  // void programReady();
  // void programStatus(int);

  uint32_t getProgramStackUsed();
  uint32_t getProgramStackAllocation();
  uint32_t getManagerStackUsed();
  uint32_t getManagerStackAllocation();
  uint32_t getFreeHeapSize();

  /* void eraseProgramFromFlash(uint8_t sector); */
  /* void saveProgramToFlash(uint8_t sector, void* address, uint32_t length); */
  PatchDefinition* getPatchDefinitionFromFlash(uint8_t sector);

  void eraseFromFlash(uint8_t sector);
  void saveToFlash(uint8_t sector, void* address, uint32_t length);

  uint32_t getCyclesPerBlock();
  uint32_t getHeapMemoryUsed();
  uint8_t getProgramIndex();
  PatchDefinition* getPatchDefinition(){
    return patchdef;
  }
private:
  void updateProgramIndex(uint8_t index);
};

extern ProgramManager program;

#endif // __ProgramManager_H__
