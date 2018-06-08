#include <string.h>
#include "device.h"
#include "cmsis_os.h"
#include "Owl.h"
#include "PatchRegistry.h"
#include "ProgramManager.h"
#include "ProgramVector.h"
#include "DynamicPatchDefinition.hpp"
#include "errorhandlers.h"
#include "Codec.h"
#include "ServiceCall.h"
#include "FlashStorage.h"
#include "BitState.hpp"
#ifdef USE_MIDI_CALLBACK
#include "MidiReader.h"
#endif /* USE_MIDI_CALLBACK */
#ifdef USE_SCREEN
#include "Graphics.h"
#endif
#ifdef USE_DIGITALBUS
#include "bus.h"
#endif

// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).
// #define SCREEN_TASK_STACK_SIZE (2*1024/sizeof(portSTACK_TYPE))
#define AUDIO_TASK_STACK_SIZE  (2*1024/sizeof(portSTACK_TYPE))
#define AUDIO_TASK_PRIORITY  4

// #define MANAGER_TASK_STACK_SIZE  (2*1024/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_PRIORITY  (AUDIO_TASK_PRIORITY | portPRIVILEGE_BIT)
// audio and manager task priority must be the same so that the program can stop itself in case of errors
#define FLASH_TASK_PRIORITY 5
#define FLASH_TASK_STACK_SIZE (512/sizeof(portSTACK_TYPE))

const uint32_t PROGRAMSTACK_SIZE = 8*1024; // size in bytes

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define PROGRAM_FLASH_NOTIFICATION  0x04
#define ERASE_FLASH_NOTIFICATION    0x08

ProgramManager program;
PatchRegistry registry;
ProgramVector staticVector;
ProgramVector* programVector = &staticVector;
static TaskHandle_t audioTask = NULL;
static TaskHandle_t managerTask = NULL;
static TaskHandle_t utilityTask = NULL;
static DynamicPatchDefinition dynamo;

#ifdef USE_ADC
extern uint16_t adc_values[NOF_ADC_VALUES];
#endif
#ifndef USE_SCREEN
int16_t parameter_values[NOF_PARAMETERS];
#endif
BitState32 stateChanged;
uint16_t button_values;
uint16_t timestamps[NOF_BUTTONS]; 

ProgramVector* getProgramVector() { return programVector; }

#if 0
static int16_t encoders[NOF_ENCODERS] = {INT16_MAX/2, INT16_MAX/2};
static int16_t deltas[NOF_ENCODERS] = {0, 0};
void encoderChanged(uint8_t encoder, int16_t value){
  // // todo: debounce
  // // pass encoder change event to patch
  int32_t delta = value - encoders[encoder];
  encoders[encoder] = value;
  deltas[encoder] = delta;

#if defined USE_SCREEN && !defined OWL_PRISM
  graphics.params.encoderChanged(encoder, delta);
#endif

  // todo: save changes and pass at programReady()
  // if(getProgramVector()->encoderChangedCallback != NULL)
  //   getProgramVector()->encoderChangedCallback(encoder, delta, 0);
}
#endif

PatchDefinition* getPatchDefinition(){
  return program.getPatchDefinition();
}

void audioCallback(int32_t* rx, int32_t* tx, uint16_t size){
  getProgramVector()->audio_input = rx;
  getProgramVector()->audio_output = tx;
  getProgramVector()->audio_blocksize = size;
  // vTaskSuspend(screenTask);
  if(audioTask != NULL){
    BaseType_t yield;
    vTaskNotifyGiveFromISR(audioTask, &yield);
    portYIELD_FROM_ISR(yield);
  }
}

/* called by the program when an error or anomaly has occured */
void onProgramStatus(ProgramVectorAudioStatus status){
  // setLed(RED);
  program.exitProgram(false);
  char msg[] = "Err xx";
  msg[4] = '0'+(status/10);
  msg[5] = '0'+(status%10);
  error(PROGRAM_ERROR, msg);
  for(;;);
}

int16_t getParameterValue(uint8_t pid){
  if(pid < NOF_PARAMETERS)
#ifdef USE_SCREEN
    return graphics.params.parameters[pid];
#else
    return parameter_values[pid];
#endif
  return 0;
}

void setParameterValue(uint8_t pid, int16_t value){
  // called from program, MIDI, or (potentially) digital bus
  if(pid < NOF_PARAMETERS)
#ifdef USE_SCREEN
    graphics.params.setValue(pid, value);
  // graphics.params.parameters[pid] = value;
#else
    parameter_values[pid] = value;
#endif
}

uint8_t getButtonValue(uint8_t ch){
  return bool(button_values & (1<<ch));
}

void setButtonValue(uint8_t ch, uint8_t value){
  if(ch < NOF_BUTTONS){
    timestamps[ch] = getSampleCounter();
    stateChanged.set(ch);
  // if(value)
  //   button_values |= (1<<ch);
  // else
  //   button_values &= ~(1<<ch);
  }
  button_values &= ~((!value)<<ch);
  button_values |= (bool(value)<<ch);
}

#ifdef USE_ADC
void updateParameters(){
  // IIR exponential filter with lambda 0.75
#if defined OWL_MODULAR || defined OWL_TESSERACT /* inverting ADCs */
  parameter_values[0] = (parameter_values[0]*3 + 4095-adc_values[ADC_A])>>2;
  parameter_values[1] = (parameter_values[1]*3 + 4095-adc_values[ADC_B])>>2;
  parameter_values[2] = (parameter_values[2]*3 + 4095-adc_values[ADC_C])>>2;
  parameter_values[3] = (parameter_values[3]*3 + 4095-adc_values[ADC_D])>>2;
#elif defined USE_SCREEN
  // Player todo: route input CVs to parameters
#else
#ifdef ADC_A
  parameter_values[0] = (parameter_values[0]*3 + adc_values[ADC_A])>>2;
#endif
#ifdef ADC_B
  parameter_values[1] = (parameter_values[1]*3 + adc_values[ADC_B])>>2;
#endif
#ifdef ADC_C
  parameter_values[2] = (parameter_values[2]*3 + adc_values[ADC_C])>>2;
#endif
#ifdef ADC_D
  parameter_values[3] = (parameter_values[3]*3 + adc_values[ADC_D])>>2;
#endif
#ifdef ADC_E
  parameter_values[4] = adc_values[ADC_E];
#endif
  // parameter_values[0] = 4095-adc_values[0];
  // parameter_values[1] = 4095-adc_values[1];
  // parameter_values[2] = 4095-adc_values[2];
  // parameter_values[3] = 4095-adc_values[3];
#endif
}
#else
void updateParameters(){
}
#endif

/* called by the program when a block has been processed */
void onProgramReady(){
  ProgramVector* pv = getProgramVector();
#ifdef DEBUG_DWT
  pv->cycles_per_block = DWT->CYCCNT;
#endif
  uint32_t ulNotifiedValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
#ifdef DEBUG_DWT
  DWT->CYCCNT = 0;
#endif
  if(ulNotifiedValue > 16){
    // midi.sendProgramStats();
    error(PROGRAM_ERROR, "CPU overrun");
    program.exitProgram(false);
  }
  updateParameters();
  pv->buttons = button_values;
  // ready to run block again
  if(pv->buttonChangedCallback != NULL && stateChanged.getState()){
    int bid = stateChanged.getFirstSetIndex();
    do{
      pv->buttonChangedCallback(bid, getButtonValue(bid)?4095:0, timestamps[bid]);
      timestamps[bid] = 0;
      stateChanged.clear(bid);
      bid = stateChanged.getFirstSetIndex();
    }while(bid > 0); // bid 0 is bypass button which we ignore
  }
  // HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

}

// called from program
void onSetPatchParameter(uint8_t pid, int16_t value){
// #ifdef USE_SCREEN
//   graphics.params.setDynamicValue(ch, value);
// #else
//   parameter_values[ch] = value;
// #endif
  setParameterValue(pid, value);
#ifdef USE_DIGITALBUS
  if(settings.bus_enabled){
    bus_tx_parameter(pid, value);
  }
#endif
}
// called from program
void onSetButton(uint8_t bid, uint16_t state, uint16_t samples){
  setButtonValue(bid, state);
}

// called from program
void onRegisterPatchParameter(uint8_t id, const char* name){
#ifdef USE_SCREEN 
  graphics.params.setName(id, name);
#endif /* USE_SCREEN */
}

// called from program
void onRegisterPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
#if defined OWL_MAGUS || defined OWL_PRISM
  graphics.params.setTitle(name);
#endif /* OWL_MAGUS */
}

void updateProgramVector(ProgramVector* pv){
#if defined OWL_TESSERACT
  pv->hardware_version = TESSERACT_HARDWARE;
#elif defined OWL_MICROLAB
  pv->hardware_version = MICROLAB_HARDWARE;
#elif defined OWL_MINILAB
  pv->hardware_version = MINILAB_HARDWARE;
#elif defined OWL_RACK
  pv->hardware_version = OWL_RACK_HARDWARE;
#elif defined OWL_PEDAL
  pv->hardware_version = OWL_PEDAL_HARDWARE;
#elif defined OWL_MODULAR
  pv->hardware_version = OWL_MODULAR_HARDWARE;
#elif defined OWL_PLAYERF7
  pv->hardware_version = PLAYER_HARDWARE;
#elif defined OWL_PRISMF7
  pv->hardware_version = PRISM_HARDWARE;
#elif defined OWL_PRISM
  pv->hardware_version = PRISM_HARDWARE;
#elif defined OWL_MAGUS
  pv->hardware_version = MAGUS_HARDWARE;
#elif defined OWL_EFFECTSBOX
  pv->hardware_version = EFFECTSBOX_HARDWARE;
#else
#error "invalid configuration"
#endif
  pv->checksum = PROGRAM_VECTOR_CHECKSUM;
#ifdef USE_SCREEN
  pv->parameters_size = graphics.params.getSize();
  pv->parameters = graphics.params.parameters;
#else
  pv->parameters_size = NOF_PARAMETERS;
  pv->parameters = parameter_values;
#endif
  pv->audio_samplingrate = 48000;
  pv->audio_blocksize = codec.getBlockSize();
  pv->buttons = button_values;
  pv->registerPatch = onRegisterPatch;
  pv->registerPatchParameter = onRegisterPatchParameter;
  pv->cycles_per_block = 0;
  pv->heap_bytes_used = 0;
  pv->programReady = onProgramReady;
  pv->programStatus = onProgramStatus;
  pv->serviceCall = serviceCall;
  pv->setButton = onSetButton;
  pv->setPatchParameter = onSetPatchParameter;
  pv->buttonChangedCallback = NULL;
#ifdef PROGRAM_VECTOR_V12
  pv->audio_bitdepth = 24;
  pv->encoderChangedCallback = NULL;
#endif
#ifdef PROGRAM_VECTOR_V13
  extern char _EXTRAM, _EXTRAM_END;
#ifdef OWL_ARCH_F7
  static MemorySegment heapSegments[] = {
    { (uint8_t*)&_EXTRAM, (uint32_t)(&_EXTRAM_END - &_EXTRAM) },
    { NULL, 0 }
  };
#else
  extern char _CCMRAM, _CCMRAM_END;
  static MemorySegment heapSegments[] = {
    { (uint8_t*)&_CCMRAM, (uint32_t)(&_CCMRAM_END - &_CCMRAM) - PROGRAMSTACK_SIZE },
#ifndef OWL_PRISM
    { (uint8_t*)&_EXTRAM, (uint32_t)(&_EXTRAM_END - &_EXTRAM) },
#endif
    // todo: add remaining program space
    { NULL, 0 }
  };
#endif
  pv->heapSegments = (MemorySegment*)heapSegments;
#ifdef USE_WM8731
  pv->audio_format = AUDIO_FORMAT_24B16;
  // pv->audio_format = AUDIO_FORMAT_24B24;
#else
  pv->audio_format = AUDIO_FORMAT_24B32;
#endif
#endif /* PROGRAM_VECTOR_V13 */
  pv->message = NULL;
}

volatile int flashSectorToWrite;
volatile void* flashAddressToWrite;
volatile uint32_t flashSizeToWrite;
void programFlashTask(void* p){
  uint8_t index = flashSectorToWrite;
  uint32_t size = flashSizeToWrite;
  uint8_t* source = (uint8_t*)flashAddressToWrite;
  if(index == 0xff && size < MAX_SYSEX_FIRMWARE_SIZE){
    // flashFirmware(source, size); TODO
    error(PROGRAM_ERROR, "Flash firmware TODO");
  }else{
    registry.store(index, source, size);
    program.loadProgram(index);
    program.resetProgram(false);
  }
  // midi.sendProgramMessage();
  // midi.sendDeviceStats();
  utilityTask = NULL;
  vTaskDelete(NULL);
}

void eraseFlashTask(void* p){
  int sector = flashSectorToWrite;
  if(sector == 0xff){
    storage.erase();
    // debugMessage("Erased flash storage");
    registry.init();
  }
  // midi.sendProgramMessage();
  // midi.sendDeviceStats();
  utilityTask = NULL;
  vTaskDelete(NULL);
}

void runAudioTask(void* p){
#ifdef USE_SCREEN
    graphics.params.reset();
#endif
    PatchDefinition* def = getPatchDefinition();
    ProgramVector* pv = def == NULL ? NULL : def->getProgramVector();
    if(pv != NULL && def->verify()){
      updateProgramVector(pv);
      programVector = pv;
      setErrorStatus(NO_ERROR);
      // setLed(GREEN);
      // codec.softMute(false);
      // codec.resume();
      def->run();
      error(PROGRAM_ERROR, "Program exited");
    }else{
      error(PROGRAM_ERROR, "Invalid program");
    }
    // setLed(RED);
    // program.exitProgram(false);
    audioTask = NULL;
    vTaskDelete(NULL);
    for(;;);
}

void runManagerTask(void* p){
  uint32_t ulNotifiedValue = 0;
  TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  for(;;){
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,          /* Don't clear any notification bits on entry. */
		    UINT32_MAX,       /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime); 
    if(ulNotifiedValue & STOP_PROGRAM_NOTIFICATION){ // stop program
      if(audioTask != NULL){
	// codec.softMute(true);
	// capture program error before pv is changed
	if(programVector != NULL){
	  staticVector.error = programVector->error;
	  staticVector.heap_bytes_used = programVector->heap_bytes_used;
	  staticVector.cycles_per_block = programVector->cycles_per_block;
	  staticVector.message = programVector->message;
	}
	programVector = &staticVector;
	// clear callbacks
#ifdef USE_SCREEN
	graphics.setCallback(NULL);
#endif /* USE_SCREEN */
#ifdef USE_MIDI_CALLBACK
	extern MidiReader mididevice;
	mididevice.setCallback(NULL);
#ifdef USE_USB_HOST
	extern MidiReader midihost;
	midihost.setCallback(NULL);
#endif /* USE_USB_HOST */
#endif /* USE_MIDI_CALLBACK */
	vTaskDelete(audioTask);
	audioTask = NULL;
      }
    }
    // allow idle task to garbage collect if necessary
    vTaskDelay(20);
    if(ulNotifiedValue & PROGRAM_FLASH_NOTIFICATION){ // program flash
      if(utilityTask != NULL)
	error(PROGRAM_ERROR, "Utility task already running");
      xTaskCreate(programFlashTask, "Flash Write", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &utilityTask);
      // bool ret = utilityTask.create(programFlashTask, "Flash Write", FLASH_TASK_PRIORITY);
      // if(!ret)
      // 	error(PROGRAM_ERROR, "Failed to start Flash Write task");
    }else if(ulNotifiedValue & ERASE_FLASH_NOTIFICATION){ // erase flash
      if(utilityTask != NULL)
	error(PROGRAM_ERROR, "Utility task already running");
      xTaskCreate(eraseFlashTask, "Flash Write", FLASH_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &utilityTask);
      // bool ret = utilityTask.create(eraseFlashTask, "Flash Erase", FLASH_TASK_PRIORITY);
      // if(!ret)
      // 	error(PROGRAM_ERROR, "Failed to start Flash Erase task");
    }
    // vTaskDelay(20);
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      PatchDefinition* def = getPatchDefinition();
      if(audioTask == NULL && def != NULL){
      	static StaticTask_t audioTaskBuffer;
#ifndef OWL_ARCH_F7
	extern char _CCMRAM, _CCMRAM_END;
	uint32_t CCMHEAP_SIZE = (uint32_t)(&_CCMRAM_END - &_CCMRAM) - PROGRAMSTACK_SIZE;
	uint8_t* CCMHEAP = (uint8_t*)&_CCMRAM;
	uint8_t* PROGRAMSTACK = CCMHEAP+CCMHEAP_SIZE;
#else
	extern char _PATCHRAM, _PATCHRAM_END;
	uint32_t PATCHRAM_SIZE = (uint32_t)(&_PATCHRAM_END - &_PATCHRAM);
	uint8_t* PROGRAMSTACK = ((uint8_t*)&_PATCHRAM )+PATCHRAM_SIZE-PROGRAMSTACK_SIZE; // put stack at end of program ram
#endif
	audioTask = xTaskCreateStatic(runAudioTask, "Audio", 
				      PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, AUDIO_TASK_PRIORITY, 
				      (StackType_t*)PROGRAMSTACK, 
				      &audioTaskBuffer);
      }
      if(audioTask == NULL)
	error(PROGRAM_ERROR, "Failed to start program task");
    }
  }
}

ProgramManager::ProgramManager(){
#ifdef DEBUG_DWT
  // DWT cycle count enable
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#ifdef OWL_ARCH_F7
  DWT->LAR = 0xC5ACCE55; // enable debug access: required on F7
#endif
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
}

void ProgramManager::startManager(){
  codec.start();
  // codec.pause();
  updateProgramVector(getProgramVector());
// #ifdef USE_SCREEN
//   xTaskCreate(runScreenTask, "Screen", SCREEN_TASK_STACK_SIZE, NULL, SCREEN_TASK_PRIORITY, &screenTask);
// #endif
  xTaskCreate(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY, &managerTask);
}

void ProgramManager::notifyManagerFromISR(uint32_t ulValue){
  BaseType_t xHigherPriorityTaskWoken = 0; 
  if(managerTask != NULL)
    xTaskNotifyFromISR(managerTask, ulValue, eSetBits, &xHigherPriorityTaskWoken );
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  // managerTask.notifyFromISR(ulValue);
}

void ProgramManager::notifyManager(uint32_t ulValue){
  if(managerTask != NULL)
    xTaskNotify(managerTask, ulValue, eSetBits );
  // managerTask.notify(ulValue);
}

void ProgramManager::startProgram(bool isr){
  if(isr)
    notifyManagerFromISR(START_PROGRAM_NOTIFICATION);
  else
    notifyManager(START_PROGRAM_NOTIFICATION);
}

void ProgramManager::exitProgram(bool isr){
  if(isr)
    notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION);
  else
    notifyManager(STOP_PROGRAM_NOTIFICATION);
}

/* exit and restart program */
void ProgramManager::resetProgram(bool isr){
  if(isr)
    notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
  else
    notifyManager(STOP_PROGRAM_NOTIFICATION|START_PROGRAM_NOTIFICATION);
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){
  patchindex = 0;
  dynamo.load(address, length);
  if(dynamo.getProgramVector() != NULL){
    patchdef = &dynamo;
    registry.setDynamicPatchDefinition(patchdef);
    // updateProgramIndex(0);
  }else{
    registry.setDynamicPatchDefinition(NULL);
  }
}

// void ProgramManager::saveProgramToFlash(uint8_t sector, void* address, uint32_t length){
// }

void ProgramManager::loadProgram(uint8_t pid){
  PatchDefinition* def = registry.getPatchDefinition(pid);
  if(def != NULL && def != patchdef && def->getProgramVector() != NULL){
    patchdef = def;
    // updateProgramIndex(pid);
    patchindex = pid;
  }
  // patchdef = &dynamo;
}

#ifdef DEBUG_STACK
uint32_t ProgramManager::getProgramStackUsed(){
  if(audioTask == NULL)
    return 0;
  uint32_t ph = uxTaskGetStackHighWaterMark(audioTask);
  return getProgramStackAllocation() - ph*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getProgramStackAllocation(){
  uint32_t ss = 0;
  if(patchdef != NULL)
    ss = patchdef->getStackSize();
  if(ss == 0)
    ss = PROGRAM_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
  return ss;
}

uint32_t ProgramManager::getManagerStackUsed(){
  if(managerTask == NULL)
    return 0;
  uint32_t mh = uxTaskGetStackHighWaterMark(managerTask);
  return getManagerStackAllocation() - mh*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getManagerStackAllocation(){
  return MANAGER_TASK_STACK_SIZE*sizeof(portSTACK_TYPE);
}
#endif /* DEBUG_STACK */

uint32_t ProgramManager::getCyclesPerBlock(){
  return getProgramVector()->cycles_per_block;  
}

uint32_t ProgramManager::getHeapMemoryUsed(){
  return getProgramVector()->heap_bytes_used;
}

uint8_t ProgramManager::getProgramIndex(){
  return patchindex;
}

extern "C" {
#if 0 // defined in Project/Src/freertos.c
  static StaticTask_t xIdleTaskTCBBuffer;// CCM;
  static StackType_t xIdleStack[IDLE_TASK_STACK_SIZE];// CCM;
  void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = IDLE_TASK_STACK_SIZE;
  }
#endif
  void vApplicationMallocFailedHook(void) {
    program.exitProgram(false);
    error(PROGRAM_ERROR, "malloc failed");
    HAL_Delay(5000);
    assert_param(0);
  }
  void vApplicationIdleHook(void) {
  }
  void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
    program.exitProgram(false);
    error(PROGRAM_ERROR, "Stack overflow");
    HAL_Delay(5000);
    assert_param(0);
  }
}

void ProgramManager::eraseFromFlash(uint8_t sector){
  flashSectorToWrite = sector;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|ERASE_FLASH_NOTIFICATION);
}

void ProgramManager::saveToFlash(uint8_t sector, void* address, uint32_t length){
  flashSectorToWrite = sector;
  flashAddressToWrite = address;
  flashSizeToWrite = length;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|PROGRAM_FLASH_NOTIFICATION);
}

uint16_t getSampleCounter(){
  // does not work: always returns values <= 5
  // return DMA_GetCurrDataCounter(DMA2_Stream0);
  return (DWT->CYCCNT)/3500;
}
