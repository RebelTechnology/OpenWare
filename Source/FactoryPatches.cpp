#include "PatchRegistry.h"
#include "FactoryPatches.h"
#include "PatchProcessor.h"
#include "sramalloc.h"
#include "device.h"
// #include "owlcontrol.h" // for setErrorMessage
#include "basicmaths.h"
#include "ProgramVector.h"

// #define STATIC_PROGRAM_STACK_SIZE   (64*1024)
// #define STATIC_PROGRAM_STACK_BASE   ((uint32_t*)CCMRAM)
#define STATIC_PROGRAM_STACK_BASE   0
#define STATIC_PROGRAM_STACK_SIZE   0

extern ProgramVector staticVector;
static PatchProcessor processor;// CCM; // 4kb SampleBuffer

PatchProcessor* getInitialisingPatchProcessor(){
  return &processor;
}

void onButtonChanged(uint8_t id, uint16_t value, uint16_t samples){
  if(processor.patch != NULL)
    processor.patch->buttonChanged((PatchButtonId)id, value, samples);
}

void onEncoderChanged(uint8_t id, int16_t delta, uint16_t samples){
  if(processor.patch != NULL)
    processor.patch->encoderChanged((PatchParameterId)id, delta, samples);
}

// void onDraw(uint8_t* pixels, uint16_t screen_width, uint16_t screen_height){
//   if(processor.patch != NULL){
//     static ScreenBuffer screen(OLED_WIDTH, OLED_HEIGHT);
//     screen.setBuffer(pixels);
//     processor.patch->processScreen(screen);
//   }
// }

bool FactoryPatchDefinition::verify(){
  return true;
}

void FactoryPatchDefinition::run(){
  extern char _EXTRAM;
  sram_init((char*)&_EXTRAM, 1024*1024);
  Patch* patch = create();
  ASSERT(patch != NULL, "Memory allocation failed");

  ProgramVector* pv = getProgramVector();
  // if(pv->checksum >= PROGRAM_VECTOR_CHECKSUM_V12){
    // set event callbacks
  pv->buttonChangedCallback = onButtonChanged;
  pv->encoderChangedCallback = onEncoderChanged;
  // pv->drawCallback = onDraw;
  // }else if(pv->checksum >= PROGRAM_VECTOR_CHECKSUM_V11){
  //   // no event callbacks
  // }else{
  //   error(CHECKSUM_ERROR_STATUS, "ProgramVector checksum error");
  //   return -1;
  // }

  processor.setPatch(patch);
  pv->heap_bytes_used = sram_used();
  processor.run();
}

#include "factory.h"

template<class T> struct Register {
  static Patch* construct() {
    return new T();
  }
};

int FACTORY_PATCH_COUNT = 0;
static FactoryPatchDefinition factorypatches[MAX_FACTORY_PATCHES];
void registerPatch(char* nm, uint8_t ins, uint8_t outs, PatchCreator c){
  if(FACTORY_PATCH_COUNT < MAX_FACTORY_PATCHES)
    factorypatches[FACTORY_PATCH_COUNT++].setup(nm, ins, outs, c);
}

#define REGISTER_PATCH(T, STR, IN, OUT) registerPatch((char*)STR, IN, OUT, Register<T>::construct);

// #undefine REGISTER_PATCH
// #define REGISTER_PATCH(T, STR, IN, OUT) registerPatch(STR, IN, OUT, Register<T>::construct)

// void registerPatch(const char* name, uint8_t inputs, uint8_t outputs,
// 		   PatchCreator creator){
// }

void FactoryPatchDefinition::init(){
  FACTORY_PATCH_COUNT = 0;
#include "factory.incl"
}

FactoryPatchDefinition::FactoryPatchDefinition() {
  stackBase = STATIC_PROGRAM_STACK_BASE;
  stackSize = STATIC_PROGRAM_STACK_SIZE;
  programVector = &staticVector;
}

FactoryPatchDefinition::FactoryPatchDefinition(char* name, uint8_t inputs, uint8_t outputs, PatchCreator c) :
  PatchDefinition(name, inputs, outputs), creator(c) {
  stackBase = STATIC_PROGRAM_STACK_BASE;
  stackSize = STATIC_PROGRAM_STACK_SIZE;
  programVector = &staticVector;
}

void FactoryPatchDefinition::setup(char* nm, uint8_t ins, uint8_t outs, PatchCreator c){
  name = nm;
  inputs = ins;
  outputs = outs;
  creator = c;
  registry.registerPatch(this);
}
