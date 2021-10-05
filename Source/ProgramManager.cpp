#include <string.h>
#include "device.h"
#include "cmsis_os.h"
#include "Owl.h"
#include "PatchRegistry.h"
#include "ProgramManager.h"
#include "ProgramVector.h"
#include "PatchDefinition.hpp"
#include "ApplicationSettings.h"
#include "errorhandlers.h"
#include "BootloaderStorage.h"
#include "VersionToken.h"
#include "message.h"
#ifdef USE_CODEC
#include "Codec.h"
#endif
#include "ServiceCall.h"
#include "Storage.h"
#include "BitState.hpp"
#include "MidiReceiver.h"
#include "MidiController.h"
#ifdef USE_DIGITALBUS
#include "bus.h"
#endif
#ifdef USE_USBD_AUDIO
#include "usbd_audio.h"
#include "CircularBuffer.h"
#endif
#ifdef USE_SCREEN
#include "Graphics.h"
#endif

#ifdef USE_USBD_AUDIO
CircularBuffer<audio_t>* volatile usbd_rx = NULL;
CircularBuffer<audio_t>* volatile usbd_tx = NULL;
static uint32_t usbd_audio_rx_count = 0;

/* Get number of samples transmitted since previous request */
uint32_t usbd_audio_get_rx_count(){
  // return 0;
  uint32_t pos = usbd_audio_rx_count + codec.getSampleCounter();
  usbd_audio_rx_count = 0;
  return pos;
}

void usbd_audio_tx_start_callback(size_t rate, uint8_t channels, void* cb){
  usbd_tx = (CircularBuffer<audio_t>*)cb;
  // usbd_tx->reset();
  // usbd_tx->clear();
  // usbd_tx->moveWriteHead(usbd_tx->getSize()/2);
}

void usbd_audio_tx_stop_callback(){
  usbd_tx = NULL;
#ifdef DEBUG
  printf("stop tx\n");
#endif
}

void usbd_audio_rx_start_callback(size_t rate, uint8_t channels, void* cb){
  usbd_rx = (CircularBuffer<audio_t>*)cb;
  // usbd_rx->reset();
  // usbd_rx->clear();
  // usbd_rx->moveReadHead(usbd_rx->getSize()/2);
  usbd_audio_rx_count = 0;
#ifdef DEBUG
  printf("start rx %u %u %u\n", rate, channels, usbd_rx->getSize());
#endif
}

void usbd_audio_rx_stop_callback(){
  usbd_rx = NULL;
#ifdef DEBUG
  printf("stop rx\n");
#endif
}

// void usbd_rx_convert(int32_t* dst, size_t len){
//   usbd_audio_rx_count += len;
//   while(len--)
//     *dst++ = AUDIO_SAMPLE_TO_INT32(usbd_rx->read());
// }

void usbd_rx_convert_add(int32_t* dst, size_t len){
  usbd_audio_rx_count += len;
  size_t cap = usbd_rx->getReadCapacity();
  if(cap < len){
    // rx buffer underflow
    memset(dst+cap, 0, (len - cap)*sizeof(int32_t));
    len = cap;
#ifdef DEBUG_USBD_AUDIO
    debugMessage("rx unf", (int)(len - cap));
#endif
  }
  while(len--)
    *dst++ = __SSAT(*dst + AUDIO_SAMPLE_TO_INT32(usbd_rx->read()), 24);
}

#if USBD_AUDIO_RX_CHANNELS != AUDIO_CHANNELS
#error "todo: support for USBD_AUDIO_RX_CHANNELS != AUDIO_CHANNELS"
#endif

void usbd_tx_convert(int32_t* src, size_t len){
  size_t cap = usbd_tx->getWriteCapacity() - USBD_AUDIO_TX_CHANNELS;
  // leave a bit of space to prevent wrapping read/write pointers
  if(cap < len){
    // tx buffer overflow
    len = cap;
#ifdef DEBUG_USBD_AUDIO
    debugMessage("tx ovf", (int)(len - cap));
#endif
  }
  while(len--)
    // macro handles shift, round, dither, clip, truncate, bitswap
    usbd_tx->write(AUDIO_INT32_TO_SAMPLE(*src++));
}

// void usbd_tx_convert_add(int32_t* src, size_t len){
//   while(len--)
//     usbd_tx->overdub(AUDIO_INT32_TO_SAMPLE(*src++));
// }

#if USBD_AUDIO_TX_CHANNELS != AUDIO_CHANNELS
#error "todo: support for USBD_AUDIO_TX_CHANNELS != AUDIO_CHANNELS"
#endif

#endif

// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).
// #define SCREEN_TASK_STACK_SIZE (2*1024/sizeof(portSTACK_TYPE))
#define AUDIO_TASK_PRIORITY  4

// #define MANAGER_TASK_STACK_SIZE  (2*1024/sizeof(portSTACK_TYPE))
#define MANAGER_TASK_PRIORITY  (AUDIO_TASK_PRIORITY | portPRIVILEGE_BIT)
// audio and manager task priority must be the same so that the program can stop itself in case of errors
#define FLASH_TASK_PRIORITY 1 // allow default task to run when FLASH task yields

#define PROGRAMSTACK_SIZE (PROGRAM_TASK_STACK_SIZE*sizeof(portSTACK_TYPE)) // size in bytes

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define PROGRAM_FLASH_NOTIFICATION  0x04
#define ERASE_FLASH_NOTIFICATION    0x08
#define SEND_RESOURCE_NOTIFICATION  0x10

ProgramManager program;
PatchRegistry registry;
ProgramVector staticVector;
ProgramVector* programVector = &staticVector;
BootloaderStorage bootloader;
static volatile TaskHandle_t audioTask = NULL;
static TaskHandle_t managerTask = NULL;
static TaskHandle_t utilityTask = NULL;
static StaticTask_t audioTaskBuffer;
static uint8_t PROGRAMSTACK[PROGRAMSTACK_SIZE] CCM_RAM; // use CCM if available

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

PatchDefinition* getPatchDefinition(){
  return registry.getPatchDefinition();
}

void audioCallback(int32_t* rx, int32_t* tx, uint16_t size){
  ProgramVector* pv = getProgramVector();  
  pv->audio_input = rx;
  pv->audio_output = tx;
  pv->audio_blocksize = size;
#ifdef FASCINATION_MACHINE
  extern uint32_t ledstatus;
  static float audio_envelope_lambda = 0.999995f;
  static float audio_envelope = 0.0;
  audio_envelope = audio_envelope*audio_envelope_lambda + (1.0f-audio_envelope_lambda)*abs(pv->audio_output[0])*(1.0f/INT16_MAX);
#endif
  if(audioTask != NULL){
    BaseType_t yield;
    // wake up audio task
    vTaskNotifyGiveFromISR(audioTask, &yield);
    portYIELD_FROM_ISR(yield);
  }
}

/* called by the program when an error or anomaly has occured */
void onProgramStatus(ProgramVectorAudioStatus status){
  program.exitProgram(false);
  char msg[] = "Err xx";
  msg[4] = '0'+(status/10);
  msg[5] = '0'+(status%10);
  error(PROGRAM_ERROR, msg);
  for(;;);
}

__weak int16_t getParameterValue(uint8_t pid){
  if(pid < NOF_PARAMETERS)
#ifdef USE_SCREEN
    return graphics.params.parameters[pid];
#else
    return parameter_values[pid];
#endif
  return 0;
}

// called from program, MIDI, or (potentially) digital bus
__weak void setParameterValue(uint8_t pid, int16_t value){
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

/* called by the program when a block has been processed */
void onProgramReady(){
  ProgramVector* pv = getProgramVector();
#ifdef USE_USBD_AUDIO_TX
  if(usbd_tx) // after patch runs: convert wet output to USBD audio tx
    usbd_tx_convert(pv->audio_output, pv->audio_blocksize*AUDIO_CHANNELS);
#endif
#ifdef USE_USBD_AUDIO_RX
  if(usbd_rx) // after patch runs: convert USBD audio rx to DAC (summing patch output)
    usbd_rx_convert_add(pv->audio_output, pv->audio_blocksize*AUDIO_CHANNELS);
#endif  
#ifdef DEBUG_DWT
  pv->cycles_per_block = DWT->CYCCNT;
#endif
  /* Block indefinitely (released by audioCallback) */
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#ifdef USE_USBD_AUDIO_RX
  // if(usbd_rx) // before patch runs: convert USBD audio rx to input (overwriting ADC)
  //   usbd_rx_convert(pv->audio_input, pv->audio_blocksize*AUDIO_CHANNELS);
  // if(usbd_rx) // before patch runs: convert USBD audio rx to input (summing ADC)
  //   usbd_rx_convert_add(pv->audio_input, pv->audio_blocksize*AUDIO_CHANNELS);
#endif
#ifdef DEBUG_DWT
  DWT->CYCCNT = 0;
#endif
#ifdef USE_ADC
#ifdef USE_SCREEN
  updateParameters(graphics.params.parameters, NOF_PARAMETERS, adc_values, NOF_ADC_VALUES);
#else
  updateParameters(parameter_values, NOF_PARAMETERS, adc_values, NOF_ADC_VALUES);
#endif
#endif
  pv->buttons = button_values;
  if(pv->buttonChangedCallback != NULL && stateChanged.getState()){
    int bid = stateChanged.getFirstSetIndex();
    do{
      pv->buttonChangedCallback(bid, getButtonValue(bid)?4095:0, timestamps[bid]);
      timestamps[bid] = 0;
      stateChanged.clear(bid);
      bid = stateChanged.getFirstSetIndex();
    }while(bid > 0); // bid 0 is bypass button which we ignore
  }
}

// called from program
void onSetPatchParameter(uint8_t pid, int16_t value){
// #ifdef USE_SCREEN
//   graphics.params.setDynamicValue(ch, value);
// #else
//   parameter_values[ch] = value;
// #endif
  setParameterValue(pid, value);
  setAnalogValue(pid, value);
#ifdef USE_DIGITALBUS
  if(settings.bus_enabled){
    bus_tx_parameter(pid, value);
  }
#endif
}

// called from program
void onSetButton(uint8_t bid, uint16_t state, uint16_t samples){
  // setButtonValue(bid, state); // Patch should update program vector. This may cause feedback loop?
  setGateValue(bid, state);
}

// called from program
void onRegisterPatchParameter(uint8_t id, const char* name){
#ifdef USE_SCREEN 
  graphics.params.setName(id, name);
#endif /* USE_SCREEN */
  midi_tx.sendPatchParameterName((PatchParameterId)id, name);
}

// called from program
void onRegisterPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
#if defined USE_SCREEN
  graphics.params.setTitle(name);
#endif /* OWL_MAGUS */
}

void updateProgramVector(ProgramVector* pv, PatchDefinition* def){
  pv->hardware_version = HARDWARE_ID;
  pv->checksum = PROGRAM_VECTOR_CHECKSUM;
#ifdef USE_SCREEN
  pv->parameters_size = graphics.params.getSize();
  pv->parameters = graphics.params.parameters;
#else
  pv->parameters_size = NOF_PARAMETERS;
  pv->parameters = parameter_values;
#endif
  pv->audio_samplingrate = AUDIO_SAMPLINGRATE;
#ifdef USE_CODEC
  pv->audio_blocksize = codec.getBlockSize();
#else
  pv->audio_blocksize = AUDIO_BLOCK_SIZE;
#endif
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
  extern char _PATCHRAM_END;
  uint8_t* end = (uint8_t*)def->getStackBase(); // program end
  uint32_t remain = (uint32_t)&_PATCHRAM_END - (uint32_t)def->getStackBase(); // space left
#ifdef USE_CCM_RAM
  extern char _CCMRAM, _CCMRAM_SIZE;
#endif
#ifdef USE_PLUS_RAM
  extern char _PLUSRAM, _PLUSRAM_END, _PLUSRAM_SIZE;
  extern char _PATCHRAM, _PATCHRAM_SIZE;
  uint8_t* plusend = (uint8_t*)&_PLUSRAM;
  uint32_t plusremain = (uint32_t)&_PLUSRAM_SIZE;
  if(def->getLinkAddress() == (uint32_t*)&_PLUSRAM){
    end = (uint8_t*)&_PATCHRAM;
    remain = (uint32_t)&_PATCHRAM_SIZE;
    plusend = (uint8_t*)def->getStackBase();
    plusremain = (uint32_t)&_PLUSRAM_END - (uint32_t)def->getStackBase();
  }
#endif
#ifdef USE_EXTERNAL_RAM
  extern char _EXTRAM, _EXTRAM_SIZE;
#endif
  static MemorySegment heapSegments[5] = {};
  size_t segments = 0;
#ifdef USE_CCM_RAM
  heapSegments[segments++] = 
    { (uint8_t*)&_CCMRAM, (uint32_t)(&_CCMRAM_SIZE) };
#endif
  if(remain >= 32) // minimum heap segment size
    heapSegments[segments++] = { end, remain };
#ifdef USE_PLUS_RAM
  if(plusremain >= 32)
    heapSegments[segments++] = { plusend, plusremain };
#endif
#ifdef USE_EXTERNAL_RAM
  heapSegments[segments++] = 
    { (uint8_t*)&_EXTRAM, (uint32_t)(&_EXTRAM_SIZE) };
#endif
  heapSegments[segments++] = { NULL, 0 };
  pv->heapSegments = (MemorySegment*)heapSegments;
#ifdef USE_WM8731
  pv->audio_format = AUDIO_FORMAT_24B16_2X;
#else
  pv->audio_format = AUDIO_FORMAT_24B32 | AUDIO_CHANNELS;
#endif
#endif /* PROGRAM_VECTOR_V13 */
  pv->message = NULL;
}

uint8_t flashSectorToWrite;
void* flashAddressToWrite;
uint32_t flashSizeToWrite;
Resource* flashResourceToSend = NULL;

void programFlashTask(void* p){
  uint8_t index = flashSectorToWrite;
  uint32_t size = flashSizeToWrite;
  uint8_t* source = (uint8_t*)flashAddressToWrite;
  owl.setOperationMode(LOAD_MODE);
  if(index == 0xff){
    error(PROGRAM_ERROR, "Enter bootloader to flash firmware");
  }else if(index == 0xfe){
    if(size <= MAX_SYSEX_BOOTLOADER_SIZE){
      taskENTER_CRITICAL();
      bootloader.erase();
      extern char _BOOTLOADER, _BOOTLOADER_END;
      if(*(uint32_t*)&_BOOTLOADER != 0xFFFFFFFF ||
	 *(uint32_t*)((uint32_t)&_BOOTLOADER_END - sizeof(VersionToken)) != 0xFFFFFFFF){
	error(PROGRAM_ERROR, "Bootloader not erased");
      }else{
	if(!bootloader.store((void*)source, size))
	  error(PROGRAM_ERROR, "Bootloader write error");
      }
      taskEXIT_CRITICAL();
    }else{
      error(PROGRAM_ERROR, "Bootloader too big");
    }
  }else{
    ResourceHeader* header = (ResourceHeader*)flashAddressToWrite;
    storage.writeResource(header);
    registry.init();
    if(index == 0){
      onResourceUpdate();
    }else{
      program.loadProgram(index);
    }
  }
  program.resetProgram(false);
  owl.setOperationMode(RUN_MODE); // in case no program available
  utilityTask = NULL;
  vTaskDelete(NULL);
}

void eraseFlashTask(void* p){
  uint8_t slot = flashSectorToWrite;
  owl.setOperationMode(LOAD_MODE);
  if(slot == 0xff){
    storage.erase();
    debugMessage("Erased flash storage");
  }else if(slot-1 < MAX_NUMBER_OF_PATCHES-1){
    Resource* resource = registry.getPatch(slot-1);
    if(resource != NULL){
      storage.eraseResource(resource);
      debugMessage("Erased patch");
    }
  }else if(slot-MAX_NUMBER_OF_PATCHES < MAX_NUMBER_OF_RESOURCES){
    Resource* resource = registry.getResource(slot-MAX_NUMBER_OF_PATCHES);
    if(resource != NULL){
      storage.eraseResource(resource);
      onResourceUpdate();
      debugMessage("Erased resource");
    }
  }
  registry.init();
  settings.init();
  program.resetProgram(false);
  owl.setOperationMode(RUN_MODE);
  utilityTask = NULL;
  vTaskDelete(NULL);
}

void sendResourceTask(void* p){
  Resource* resource = flashResourceToSend;
  flashResourceToSend = NULL;
  if(resource != NULL){
    owl.setOperationMode(LOAD_MODE);
    midi_tx.sendResource(resource);
  }
  utilityTask = NULL;
  vTaskDelete(NULL);
}

__weak void onStartProgram(){
#ifdef USE_SCREEN
  graphics.params.reset();
#endif
#ifndef USE_SCREEN
  memset(parameter_values, 0, sizeof(parameter_values));
#endif
}

void runAudioTask(void* p){
  PatchDefinition* def = getPatchDefinition();
  if(def->isValid()){
    def->copy();
    ProgramVector* pv = def->getProgramVector();
    updateProgramVector(pv, def);
    onStartProgram();
    programVector = pv;
    setErrorStatus(NO_ERROR);
    owl.setOperationMode(RUN_MODE);
#ifdef USE_CODEC
    codec.clear();
#endif
    // zero-fill heap memory
    for(size_t i=0; i<5 && pv->heapSegments[i].location != NULL; ++i)
      memset(pv->heapSegments[i].location, 0, pv->heapSegments[i].size);
    // run program
    def->run();
  }
  error(PROGRAM_ERROR, "Program error");
  audioTask = NULL;
  vTaskDelete(NULL);
}

void bootstrap(){
#ifdef USE_BKPSRAM
  extern RTC_HandleTypeDef hrtc;
  uint8_t lastprogram = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
#else    
  uint8_t lastprogram = 0;
#endif
  if(lastprogram != 0 && lastprogram == settings.program_index){
    error(CONFIG_ERROR, "Preventing reset program from starting");
#ifdef USE_BKPSRAM
    // reset for next time
    extern RTC_HandleTypeDef hrtc;
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
#endif
  }else{
    program.loadProgram(settings.program_index);
    program.startProgram(false);
  }

#ifdef USE_CODEC
  codec.start();
  // codec.pause();
#endif
}

void runManagerTask(void* p){
  bootstrap();
  
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
	midi_rx.setCallback(NULL);
	vTaskDelete(audioTask);
	audioTask = NULL;
#ifdef USE_CODEC
	codec.clear();
#endif
      }
    }
    // allow idle task to garbage collect if necessary
    vTaskDelay(20);
    if(ulNotifiedValue & PROGRAM_FLASH_NOTIFICATION){ // program flash
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      utilityTask = xTaskCreateStatic(programFlashTask, "Flash Write",
				      PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, FLASH_TASK_PRIORITY, (StackType_t*)PROGRAMSTACK, &audioTaskBuffer);
    }else if(ulNotifiedValue & ERASE_FLASH_NOTIFICATION){ // erase flash
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      utilityTask = xTaskCreateStatic(eraseFlashTask, "Flash Erase", 
				      PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, FLASH_TASK_PRIORITY, (StackType_t*)PROGRAMSTACK, &audioTaskBuffer);
    }else if(ulNotifiedValue & SEND_RESOURCE_NOTIFICATION){
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      utilityTask = xTaskCreateStatic(sendResourceTask, "Send Resource", 
				      PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, FLASH_TASK_PRIORITY, (StackType_t*)PROGRAMSTACK, &audioTaskBuffer);
    }
    // vTaskDelay(20);
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      if(audioTask == NULL && getPatchDefinition()->isValid()){
#ifdef USE_ICACHE
	SCB_InvalidateICache();
#endif
#ifdef USE_DCACHE
	SCB_CleanInvalidateDCache();
#endif
	audioTask = xTaskCreateStatic(runAudioTask, "Audio", 
				      PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, AUDIO_TASK_PRIORITY, (StackType_t*)PROGRAMSTACK, &audioTaskBuffer);
      }
      if(audioTask == NULL && registry.hasPatches())
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
  // updateProgramVector(getProgramVector(), NULL);
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

void ProgramManager::updateProgramIndex(uint8_t index){
  owl.setOperationMode(LOAD_MODE);
  patchindex = index;
  midi_tx.sendPc(index);
  midi_tx.sendPatchName(index);
  if(index != 0){
    settings.program_index = index;
#ifdef USE_BKPSRAM
    extern RTC_HandleTypeDef hrtc;
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, index);
#endif
  }
}

void ProgramManager::loadDynamicProgram(void* address, uint32_t length){  
  if(registry.loadProgram(address, length))
    updateProgramIndex(0);
}

void ProgramManager::loadProgram(uint8_t pid){
  if(patchindex != pid && registry.loadProgram(pid))
    updateProgramIndex(pid);
}

#ifdef DEBUG_STACK
uint32_t ProgramManager::getProgramStackUsed(){
  if(audioTask == NULL)
    return 0;
  uint32_t ph = uxTaskGetStackHighWaterMark(audioTask);
  return getProgramStackAllocation() - ph*sizeof(portSTACK_TYPE);
}

uint32_t ProgramManager::getProgramStackAllocation(){
  // uint32_t ss = 0;
  // if(patchdef != NULL)
  //   ss = patchdef->getStackSize();
  // if(ss == 0)
  //   ss = PROGRAMSTACK_SIZE;
  // return ss;
  return PROGRAMSTACK_SIZE;
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
  void vApplicationMallocFailedHook(void) {
    program.exitProgram(false);
    error(PROGRAM_ERROR, "malloc failed");
  }
  void vApplicationIdleHook(void) {
  }
  void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called from PendSV ISR if a stack overflow is detected. */
    // vTaskDelete(pxTask); // seems to just get stuck
    if(pxTask == utilityTask)
      utilityTask = NULL;
    if(pxTask == audioTask)
      audioTask = NULL;
    error(PROGRAM_ERROR, "stack overflow");
    if(pxTask == managerTask)
      program.startManager();
  }
}

void ProgramManager::sendResource(Resource* resource){
  flashResourceToSend = resource;
  notifyManagerFromISR(STOP_PROGRAM_NOTIFICATION|SEND_RESOURCE_NOTIFICATION);
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
  // return (DWT->CYCCNT)/ARM_CYCLES_PER_SAMPLE;
  return codec.getSampleCounter() / AUDIO_CHANNELS;
}
