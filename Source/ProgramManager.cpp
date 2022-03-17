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
#ifdef DEBUG
  printf("start tx %u %u %u\n", rate, channels, usbd_tx->getSize());
#endif
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

void usbd_rx_convert(int32_t* dst, size_t len){
  CircularBuffer<audio_t>* rx = usbd_rx;
  if(rx){
    usbd_audio_rx_count += len;
    size_t cap = rx->getReadCapacity();
    if(cap < len){
      // rx buffer underflow
      memset(dst+cap, 0, (len - cap)*sizeof(int32_t));
      len = cap;
#ifdef DEBUG_USBD_AUDIO
      debugMessage("rx unf", (int)(len - cap));
#endif
    }
#if USBD_AUDIO_RX_CHANNELS == AUDIO_CHANNELS
#if AUDIO_BITS_PER_SAMPLE == 32
    rx->read(dst, len);
#else
    while(len--)
      *dst++ = AUDIO_SAMPLE_TO_INT32(rx->read());
#endif
#else /* USBD_AUDIO_RX_CHANNELS != AUDIO_CHANNELS */
    len /= AUDIO_CHANNELS;
    while(len--){
#if AUDIO_BITS_PER_SAMPLE == 32
      rx->read(dst, USBD_AUDIO_RX_CHANNELS);
      dst += AUDIO_CHANNELS;
#else
      audio_t* src = rx->getReadHead();
      for(int ch=0; ch<USBD_AUDIO_RX_CHANNELS; ch++)
	*dst++ = AUDIO_SAMPLE_TO_INT32(*src++);
      rx->moveReadHead(USBD_AUDIO_RX_CHANNELS);
      dst += AUDIO_CHANNELS - USBD_AUDIO_RX_CHANNELS;
#endif
    }
#endif
  }
}

void usbd_tx_convert(int32_t* src, size_t len){
  CircularBuffer<audio_t>* tx = usbd_tx;
  if(tx){
    size_t cap = tx->getWriteCapacity() - USBD_AUDIO_TX_CHANNELS;
    // leave a bit of space to prevent wrapping read/write pointers
    if(cap < len){
      // tx buffer overflow
      len = cap;
#ifdef DEBUG_USBD_AUDIO
      debugMessage("tx ovf", (int)(len - cap));
#endif
    }
#if USBD_AUDIO_TX_CHANNELS == AUDIO_CHANNELS
#if false // AUDIO_BITS_PER_SAMPLE == 32
    tx->write(src, len);
#else
    while(len--)
      tx->write(AUDIO_INT32_TO_SAMPLE(*src++));
#endif
#else /*  USBD_AUDIO_TX_CHANNELS != AUDIO_CHANNELS */
    len /= AUDIO_CHANNELS;
    while(len--){
#if false // AUDIO_BITS_PER_SAMPLE == 32
      tx->write(src, USBD_AUDIO_TX_CHANNELS);
      src += AUDIO_CHANNELS;
#else
      audio_t* dst = tx->getWriteHead();
      for(int ch=0; ch<USBD_AUDIO_TX_CHANNELS; ch++)
	*dst++ = AUDIO_INT32_TO_SAMPLE(*src++);
      tx->moveWriteHead(USBD_AUDIO_TX_CHANNELS);
      src += AUDIO_CHANNELS - USBD_AUDIO_TX_CHANNELS;
#endif
    }
#endif
  }
}

#endif /* USE_USBD_AUDIO */

// FreeRTOS low priority numbers denote low priority tasks. 
// The idle task has priority zero (tskIDLE_PRIORITY).
// audio and manager task priority must be the same so that the program can stop itself in case of errors
#define FLASH_TASK_PRIORITY   1 // allow default task to run when FLASH task yields
#define SCREEN_TASK_PRIORITY  3 // less than AUDIO_TASK_PRIORITY, more than osPriorityNormal (which is probably 1)
#define AUDIO_TASK_PRIORITY   4
#define MANAGER_TASK_PRIORITY ((AUDIO_TASK_PRIORITY + 1) | portPRIVILEGE_BIT)

#define PROGRAMSTACK_SIZE (PROGRAM_TASK_STACK_SIZE*sizeof(portSTACK_TYPE)) // size in bytes

#define START_PROGRAM_NOTIFICATION  0x01
#define STOP_PROGRAM_NOTIFICATION   0x02
#define PROGRAM_FLASH_NOTIFICATION  0x04
#define ERASE_FLASH_NOTIFICATION    0x08
#define SEND_RESOURCE_NOTIFICATION  0x10

ProgramManager program;
PatchRegistry registry;
ProgramVector staticVector;
ProgramVector* volatile programVector = &staticVector;
BootloaderStorage bootloader;
static volatile TaskHandle_t audioTask = NULL;
static TaskHandle_t managerTask = NULL;
static TaskHandle_t utilityTask = NULL;
static StaticTask_t audioTaskBuffer;
static uint8_t PROGRAMSTACK[PROGRAMSTACK_SIZE] CCM_RAM; // use CCM if available
#ifdef USE_SCREEN
static TaskHandle_t screenTask = NULL;
#endif

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
    return graphics.params->getValue(pid);
#else
    return parameter_values[pid];
#endif
  return 0;
}

// called from program, MIDI, or (potentially) digital bus
__weak void setParameterValue(uint8_t pid, int16_t value){
  if(pid < NOF_PARAMETERS)
#ifdef USE_SCREEN
    graphics.params->setValue(pid, value);
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

#if 0 // pre / post fx
#ifdef USE_USBD_AUDIO_TX
#define USE_USBD_AUDIO_TX_PRE_FX
#endif
#ifdef USE_USBD_AUDIO_RX
#define USE_USBD_AUDIO_RX_PRE_FX
#endif
#else
#ifdef USE_USBD_AUDIO_TX
#define USE_USBD_AUDIO_TX_POST_FX
#endif
#ifdef USE_USBD_AUDIO_RX
#define USE_USBD_AUDIO_RX_POST_FX
#endif
#endif

/* called by the program when a block has been processed */
void onProgramReady(){
  ProgramVector* pv = getProgramVector();
#ifdef USE_USBD_AUDIO_TX_POST_FX
  // after patch runs: convert patch output to USBD audio tx
  usbd_tx_convert(pv->audio_output, pv->audio_blocksize*AUDIO_CHANNELS);
#endif
#ifdef USE_USBD_AUDIO_RX_POST_FX
  // after patch runs: convert USBD audio rx to DAC (overwriting patch output)
  usbd_rx_convert(pv->audio_output, pv->audio_blocksize*AUDIO_CHANNELS);
#endif  
#ifdef DEBUG_DWT
  pv->cycles_per_block = DWT->CYCCNT;
#endif
  /* Block indefinitely (released by audioCallback) */
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#ifdef DEBUG_DWT
  DWT->CYCCNT = 0;
#endif
  // push queued up MIDI messages through to patch
  midi_tx.transmit();
  midi_rx.receive();
#ifdef USE_ADC
#ifdef USE_SCREEN
  updateParameters(graphics.params->getParameters(), NOF_PARAMETERS, adc_values, NOF_ADC_VALUES);
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
#ifdef USE_USBD_AUDIO_TX_PRE_FX
  // before patch runs: convert audio input to USBD audio tx
  usbd_tx_convert(pv->audio_input, pv->audio_blocksize*AUDIO_CHANNELS);
#endif
#ifdef USE_USBD_AUDIO_RX_PRE_FX
  // before patch runs: convert USBD audio rx to patch audio input
  usbd_rx_convert(pv->audio_input, pv->audio_blocksize*AUDIO_CHANNELS);
#endif  
}

// called from program
void onSetPatchParameter(uint8_t pid, int16_t value){
// #ifdef USE_SCREEN
//   graphics.params->setDynamicValue(ch, value);
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
  graphics.params->setName(id, name);
#endif /* USE_SCREEN */
  midi_tx.sendPatchParameterName((PatchParameterId)id, name);
}

// called from program
void onRegisterPatch(const char* name, uint8_t inputChannels, uint8_t outputChannels){
#if defined USE_SCREEN
  graphics.params->setTitle(name);
#endif /* OWL_MAGUS */
}

void updateProgramVector(ProgramVector* pv, PatchDefinition* def){
  pv->hardware_version = HARDWARE_ID;
  pv->checksum = PROGRAM_VECTOR_CHECKSUM;
#ifdef USE_SCREEN
  pv->parameters_size = graphics.params->getSize();
  pv->parameters = graphics.params->getParameters();
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
  pv->error = 0;
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
  extern uint8_t _PATCHRAM, _PATCHRAM_END, _PATCHRAM_SIZE;
  uint8_t* end = (uint8_t*)def->getStackBase(); // program end
  uint32_t remain = &_PATCHRAM_END - end; // space left (don't use patch declared stack size)
  if(end < &_PATCHRAM || end+remain > &_PATCHRAM_END) // sanity check
    remain = 0; // prevent errors if program stack is not linked to PATCHRAM
#ifdef USE_PLUS_RAM
  extern uint8_t _PLUSRAM, _PLUSRAM_END, _PLUSRAM_SIZE;
  uint8_t* plusend = (uint8_t*)&_PLUSRAM;
  uint32_t plusremain = (uint32_t)&_PLUSRAM_SIZE;
  if(def->getLinkAddress() == (uint32_t*)&_PLUSRAM){
    end = (uint8_t*)&_PATCHRAM;
    remain = (uint32_t)&_PATCHRAM_SIZE; // use all of PATCHRAM for heap
    plusend = (uint8_t*)def->getStackBase();
    plusremain = &_PLUSRAM_END - plusend;
  }
#endif
  static MemorySegment heapSegments[5] = {};
  size_t segments = 0;
#ifdef USE_CCM_RAM
  extern char _CCMRAM, _CCMRAM_SIZE;
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
  extern char _EXTRAM, _EXTRAM_SIZE;
  heapSegments[segments++] = 
    { (uint8_t*)&_EXTRAM, (uint32_t)(&_EXTRAM_SIZE) };
#endif
  heapSegments[segments] = { NULL, 0 };
  // zero-fill heap memory
  for(size_t i=0; i<segments; ++i)
    memset(heapSegments[i].location, 0, heapSegments[i].size);
  pv->heapSegments = (MemorySegment*)heapSegments;
#ifdef USE_WM8731
  pv->audio_format = AUDIO_FORMAT_24B16_2X;
#elif AUDIO_CHANNELS == 2
  pv->audio_format = AUDIO_FORMAT_24B32;
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
      program.resetProgramIndex();
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
  graphics.reset();
#else
  memset(parameter_values, 0, sizeof(parameter_values));
#endif
}

#ifdef USE_SCREEN
void runScreenTask(void* p){
  TickType_t xLastWakeTime;
  TickType_t xFrequency;
  xFrequency = SCREEN_LOOP_SLEEP_MS / portTICK_PERIOD_MS;
  xLastWakeTime = xTaskGetTickCount();
  for(;;){
    vTaskDelayUntil(&xLastWakeTime, xFrequency);    
    onScreenDraw();
    graphics.draw();
    graphics.display();
  }
}
#endif	// USE_SCREEN

void runAudioTask(void* p){
  taskENTER_CRITICAL();
  PatchDefinition* def = getPatchDefinition();
  if(def->isValid() && def->copy()){
    ProgramVector* pv = def->getProgramVector();
    updateProgramVector(pv, def);
    programVector = pv;
    setErrorStatus(NO_ERROR);
    owl.setOperationMode(RUN_MODE);
    onStartProgram();
#ifdef USE_CODEC
    codec.clear();
#endif
  }else{
    def = NULL;
  }
  taskEXIT_CRITICAL();  
  if(def != NULL)
    def->run();  // run program (should not return)
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
  }else if(registry.hasPatch(settings.program_index)){
    program.loadProgram(settings.program_index);
    program.startProgram(false);
  }else{
    owl.setOperationMode(CONFIGURE_MODE);
  }

#ifdef USE_CODEC
  codec.start();
  // codec.pause();
#endif
}

void runManagerTask(void* p){
  bootstrap();  
  uint32_t ulNotifiedValue = 0;
  const TickType_t xMaxBlockTime = portMAX_DELAY;  /* Block indefinitely. */
  for(;;){
    /* Block indefinitely (without a timeout, so no need to check the function's
       return value) to wait for a notification.
       Bits in this RTOS task's notification value are set by the notifying
       tasks and interrupts to indicate which events have occurred. */
    xTaskNotifyWait(pdFALSE,          /* Don't clear any notification bits on entry. */
		    UINT32_MAX,       /* Reset the notification value to 0 on exit. */
		    &ulNotifiedValue, /* Notified value pass out in ulNotifiedValue. */
		    xMaxBlockTime); 
    taskENTER_CRITICAL();
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
	owl.setMessageCallback(NULL);
	vTaskDelete(audioTask);
	audioTask = NULL;
#ifdef USE_CODEC
	codec.clear();
#endif
      }
    }
    taskEXIT_CRITICAL();
    vTaskDelay(20); // allow idle task to garbage collect if necessary
    taskENTER_CRITICAL();
    if(ulNotifiedValue & PROGRAM_FLASH_NOTIFICATION){ // program flash
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      else
	xTaskCreate(programFlashTask, "Flash Write", UTILITY_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &utilityTask);
    }else if(ulNotifiedValue & ERASE_FLASH_NOTIFICATION){ // erase flash
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      else
	xTaskCreate(eraseFlashTask, "Flash Erase", UTILITY_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &utilityTask);
;
    }else if(ulNotifiedValue & SEND_RESOURCE_NOTIFICATION){
      if(utilityTask != NULL)
        error(PROGRAM_ERROR, "Utility task already running");
      else
	xTaskCreate(sendResourceTask, "Send Resource", UTILITY_TASK_STACK_SIZE, NULL, FLASH_TASK_PRIORITY, &utilityTask);
    }
    if(ulNotifiedValue & START_PROGRAM_NOTIFICATION){ // start
      if(audioTask == NULL && getPatchDefinition()->isValid()){
	audioTask = xTaskCreateStatic(runAudioTask, "Audio", PROGRAMSTACK_SIZE/sizeof(portSTACK_TYPE),
				      NULL, AUDIO_TASK_PRIORITY, (StackType_t*)PROGRAMSTACK, &audioTaskBuffer);
      }
      if(audioTask == NULL && registry.hasPatches())
	error(PROGRAM_ERROR, "Failed to start program task");
    }
    taskEXIT_CRITICAL();
  }
}

ProgramManager::ProgramManager(){
#ifdef DEBUG_DWT
  // DWT cycle count enable
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#ifdef STM32H7xx
  DWT->LAR = 0xC5ACCE55; // enable debug access: required on M7
#endif
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
}

void ProgramManager::startManager(){
  xTaskCreate(runManagerTask, "Manager", MANAGER_TASK_STACK_SIZE, NULL, MANAGER_TASK_PRIORITY, &managerTask);
#ifdef USE_SCREEN
  xTaskCreate(runScreenTask, "Screen", SCREEN_TASK_STACK_SIZE, NULL, SCREEN_TASK_PRIORITY, &screenTask);
#endif
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

void ProgramManager::loadDynamicProgram(ResourceHeader* resource){
  PatchDefinition* def = getPatchDefinition();
  if(def->load(resource) && def->isValid())
  // if(def->load(resource.getData(), resource.getDataSize()) && def->isValid())
    updateProgramIndex(0);
  else
    error(PROGRAM_ERROR, "Load failed");
}

// void ProgramManager::loadDynamicProgram(void* address, uint32_t length){  
//   // if(registry.loadProgram(address, length))
//   PatchDefinition* def = getPatchDefinition();
//   if(def->load(address, length) && def->isValid())
//     updateProgramIndex(0);
//   else
//     error(PROGRAM_ERROR, "Load failed");
// }

void ProgramManager::loadProgram(uint8_t pid){
  if(patchindex != pid && registry.hasPatch(pid)){
    if(registry.loadProgram(pid))
      updateProgramIndex(pid);
    else
      error(PROGRAM_ERROR, "Load failed");
  }
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

bool ProgramManager::isProgramRunning(){
  return audioTask != NULL;
}

extern "C" {
  void vApplicationMallocFailedHook(void) {
    error(PROGRAM_ERROR, "malloc failed");
    program.exitProgram(false);
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
