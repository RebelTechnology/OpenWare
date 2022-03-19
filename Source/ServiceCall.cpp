#include <inttypes.h>
#include "arm_math.h"
#include "device.h"
#include "ServiceCall.h"
#include "ApplicationSettings.h"
#include "OpenWareMidiControl.h"
#include "Storage.h"
#include "Owl.h"

#include "cmsis_os.h"

#ifdef USE_FFT_TABLES
#include "arm_const_structs.h"
#endif /* USE_FFT_TABLES */
#ifdef USE_FAST_POW
#ifdef USE_FAST_POW_RESOURCES
extern uint32_t fast_log_table_size;
extern uint32_t fast_pow_table_size;
extern float* fast_log_table;
extern float* fast_pow_table;
#else
#include "FastLogTable.h"
#include "FastPowTable.h"
#endif
#endif /* USE_FAST_POW */
#ifdef USE_SCREEN
#include "Graphics.h"
#endif
#ifdef USE_MIDI_CALLBACK
#include "MidiReceiver.h"
#endif /* USE_MIDI_CALLBACK */
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifdef USE_FFT_TABLES
int SERVICE_ARM_CFFT_INIT_F32(arm_cfft_instance_f32* instance, int len){
  switch(len) { 
  case 16:
    *instance = arm_cfft_sR_f32_len16;
    break;
  case 32:
    *instance = arm_cfft_sR_f32_len32;
    break;
  case 64:
    *instance = arm_cfft_sR_f32_len64;
    break;
  case 128:
    *instance = arm_cfft_sR_f32_len128;
    break;
  case 256:
    *instance = arm_cfft_sR_f32_len256;
    break;
  case 512:
    *instance = arm_cfft_sR_f32_len512;
    break;
  case 1024:
    *instance = arm_cfft_sR_f32_len1024;
    break;      
  case 2048:
    *instance = arm_cfft_sR_f32_len2048;
    break;
  case 4096:
    *instance = arm_cfft_sR_f32_len4096;
    break;
  default:
    return OWL_SERVICE_INVALID_ARGS;
  }
  return OWL_SERVICE_OK;
}
#endif /* USE_FFT_TABLES */
#if 0
  case 32U:
    #define ARMBITREVINDEXTABLE_16_TABLE_LENGTH ((uint16_t)20)
    extern const uint16_t armBitRevIndexTable16[ARMBITREVINDEXTABLE_16_TABLE_LENGTH];
    extern const float32_t twiddleCoef_16[32];
    extern const float32_t twiddleCoef_rfft_32[32];
// size: 32*4 + 32*4 + 20*2 bytes == 16*2 words + 20 halfwords

  case 64U:
    #define ARMBITREVINDEXTABLE_32_TABLE_LENGTH ((uint16_t)48)
    extern const uint16_t armBitRevIndexTable32[ARMBITREVINDEXTABLE_32_TABLE_LENGTH];
    extern const float32_t twiddleCoef_32[64];
    extern const float32_t twiddleCoef_rfft_64[64];
// size: 64*4 + 64*4 + 48*2 bytes == 32*2 words + 48 halfwords

  case 128U:
    #define ARMBITREVINDEXTABLE_64_TABLE_LENGTH ((uint16_t)56)
    extern const uint16_t armBitRevIndexTable64[ARMBITREVINDEXTABLE_64_TABLE_LENGTH];

  case 256U:
    #define ARMBITREVINDEXTABLE_128_TABLE_LENGTH ((uint16_t)208)
    extern const uint16_t armBitRevIndexTable128[ARMBITREVINDEXTABLE_128_TABLE_LENGTH];

template<size_t fftlen, size_t bitrevlen>
struct arm_rfft_tables {
  const float32_t twiddle[fftlen];
  const float32_t twiddle_rfft[fftlen];  
  const uint16_t bit_rev[bitrevlen];
  size_t getTotalSize(){
    return fftlen*2*sizeof(float32_t) + bitrevlen*sizeof(uint16_t);
  }
  void arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32* S){
    arm_cfft_instance_f32* Sint = &(S->Sint);
    Sint->fftLen = fftlen/2;
    S->fftLenRFFT = fftlen;
    Sint->bitRevLength = bitrevlen;
    Sint->pBitRevTable = bit_rev;
    Sint->pTwiddle     = twiddle;
    S->pTwiddleRFFT    = twiddle_rfft;
  }
};

arm_rfft_tables<32, ARMBITREVINDEXTABLE_16_TABLE_LENGTH> rfft_32 = {
  twiddleCoef_16,
  twiddleCoef_rfft_32,
  armBitRevIndexTable16
};

arm_status arm_rfft_fast_init_f32( arm_rfft_fast_instance_f32 * S, size_t fftlen) {
  arm_cfft_instance_f32* Sint = &(S->Sint);
  switch(fftlen){
  case 32U:
    Sint->fftLen = 16U;
    S->fftLenRFFT = 32U;
    Sint->bitRevLength = ARMBITREVINDEXTABLE_16_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)armBitRevIndexTable16;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_16;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_32;

  case 1024:
    Sint->fftLen = 512U;
    S->fftLenRFFT = 1024U;
    Sint->bitRevLength = ARMBITREVINDEXTABLE_512_TABLE_LENGTH;
    Sint->pBitRevTable = (uint16_t *)armBitRevIndexTable512;
    Sint->pTwiddle     = (float32_t *) twiddleCoef_512;
    S->pTwiddleRFFT    = (float32_t *) twiddleCoef_rfft_1024;
  }
  return ARM_MATH_SUCCESS;
}
#endif

static int handleVersion(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  if(len > 0){
    int* value = (int*)params[0];
    *value = OWL_SERVICE_VERSION_V1;
    ret = OWL_SERVICE_OK;
  }
  return ret;
}
#ifdef USE_FFT_TABLES
static int handleRFFT(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  if(len == 2){
    arm_rfft_fast_instance_f32* instance = (arm_rfft_fast_instance_f32*)params[0];
    int fftlen = *(int*)params[1];
    arm_rfft_fast_init_f32(instance, fftlen);
    ret = OWL_SERVICE_OK;
  }
  return ret;
}

static int handleCFFT(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
    if(len == 2){
      arm_cfft_instance_f32* instance = (arm_cfft_instance_f32*)params[0];
      int fftlen = *(int*)params[1];
      ret = SERVICE_ARM_CFFT_INIT_F32(instance, fftlen);
    }
  return ret;
}
#endif

static int handleGetParameters(void** params, int len){
  int ret = OWL_SERVICE_OK;
  int index = 0;
  while(len >= index+2){
    char* p = (char*)params[index++];
    int32_t* value = (int32_t*)params[index++];
    if(strncmp(SYSEX_CONFIGURATION_INPUT_OFFSET, p, 2) == 0){
      *value = settings.input_offset;
    }else if(strncmp(SYSEX_CONFIGURATION_INPUT_SCALAR, p, 2) == 0){
      *value = settings.input_scalar;
    }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_OFFSET, p, 2) == 0){
      *value = settings.output_offset;
    }else if(strncmp(SYSEX_CONFIGURATION_OUTPUT_SCALAR, p, 2) == 0){
      *value = settings.output_scalar;
    }else{
      ret = OWL_SERVICE_INVALID_ARGS;
    }
  }
  return ret;
}

/*
 * Copy resource contents to preallocated buffer in memory
 * 
 * 4 parameters are expected:
 *  - resource name
 *  - buffer address
 *  - offset in bytes
 *  - max_length in bytes
 * 
 * When buffer address is not given, we will update max_length based on resource size. If resource
 * is using memory mapped storage, buffer address will be set to resource start + offset. If storage
 * is not memory mapped, we can't set this pointer.
 * 
 * When address is given, we're assuming max_length to be its size. So resource contents up to that
 * length in bytes would be copied, starting after offset specified.
 */
static int handleLoadResource(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  if(len == 4){
    const char* name = (const char*)params[0];
    uint8_t** buffer = (uint8_t**)params[1];
    uint32_t offset = *(uint32_t*)params[2];
    uint32_t* max_size = (uint32_t*)params[3];
    Resource* res = storage.getResourceByName(name);
    // We require offset to be aligned to 4 bytes
    if (res != NULL && !(offset & 0b11)) {
      if (*buffer == NULL) {
        // Buffer pointer not given, so we will update value referenced by max_size with
        // actual resource size here
        *max_size = res->getDataSize() - offset;
	if(res->isMemoryMapped())
	  *buffer = (uint8_t*)res->getData() + offset;
      }else{
	uint32_t copy_size = min(*max_size, res->getDataSize() - offset);
        // Buffer pointer is given. We'll copy no more than max_size data into it.
	storage.readResource(res->getHeader(), *buffer, offset, copy_size);
	*max_size = copy_size; // update max_size parameter with amount of data actually copied
      }
      ret = OWL_SERVICE_OK;
    }
  }
  return ret;
}

static int handleGetArray(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  // get array and array size
  // expects three parameters: name, &array and &size
#ifdef USE_FAST_POW
  if(len >= 3){
    char* p = (char*)params[0];
    void** array = (void**)params[1];
    int* size = (int*)params[2];
    if(strncmp(SYSTEM_TABLE_LOG, p, 3) == 0){
      *array = (void*)fast_log_table;
      *size = fast_log_table_size;
    }else if(strncmp(SYSTEM_TABLE_POW, p, 3) == 0){
      *array = (void*)fast_pow_table;
      *size = fast_pow_table_size;
      ret = OWL_SERVICE_OK;
    }else{
      *array = NULL;
      *size = 0;
    }
  }
// #else
//   // look for it in resources
//   if(len >= 3){
//     char* p = (char*)params[0];
//     void** array = (void**)params[1];
//     int* size = (int*)params[2];
//     Resource* res = NULL;
//     if(strncmp(SYSTEM_TABLE_LOG, p, 3) == 0){
//       res = storage.getResourceByName(SYSTEM_TABLE_LOG ".bin");
//     }else if(strncmp(SYSTEM_TABLE_POW, p, 3) == 0){
//       res = storage.getResourceByName(SYSTEM_TABLE_POW ".bin");
//     }
//     if(res && res->isValid()){
//       static float fast_pow_table[fast_pow_table_size] __attribute__ ((section (".d2data")));
//       static float fast_log_table[fast_log_table_size] __attribute__ ((section (".d2data")));
//       *array = res->getData();
//       *size = res->getDataSize();
//       ret = OWL_SERVICE_OK;
//     }else{
//       *array = NULL;
//       *size = 0;
//     }
#endif /* USE_FAST_POW */
  return ret;
}

static int handleRequestCallback(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  int index = 0;
  if(len >= index+2){
    char* name = (char*)params[index++];
    void** callback = (void**)params[index++];
#ifdef USE_MIDI_CALLBACK
    if(strncmp(SYSTEM_FUNCTION_MIDI, name, 3) == 0){
      *callback = (void*)midi_send;
      ret = OWL_SERVICE_OK;
    }
#endif /* USE_MIDI_CALLBACK */
  }
  return ret;
}

static int handleRegisterCallback(void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  int index = 0;
  if(len >= index+2){
    char* name = (char*)params[index++];
    void* callback = (void*)params[index++];
#ifdef USE_SCREEN
    if(strncmp(SYSTEM_FUNCTION_DRAW, name, 3) == 0){
      // void (*drawCallback)(uint8_t*, uint16_t, uint16_t);
      graphics.setCallback(callback);
      ret = OWL_SERVICE_OK;
    }
#endif /* USE_SCREEN */
#ifdef USE_MIDI_CALLBACK
    if(strncmp(SYSTEM_FUNCTION_MIDI, name, 3) == 0){
      // void (*midiCallback)(uint8_t port, uint8_t status, uint8_t, uint8_t);
      midi_rx.setCallback(callback);
      ret = OWL_SERVICE_OK;
    }
#endif /* USE_MIDI_CALLBACK */
#ifdef USE_MESSAGE_CALLBACK
    if(strncmp(SYSTEM_FUNCTION_MESSAGE, name, 3) == 0){
      // void (*messageCallback)(const char* msg, size_t len);
      owl.setMessageCallback(callback);
      ret = OWL_SERVICE_OK;
    }
#endif /* USE_MESSAGE_CALLBACK */
  }
  return ret;
}

int serviceCall(int service, void** params, int len){
  int ret = OWL_SERVICE_INVALID_ARGS;
  switch(service){
  case OWL_SERVICE_VERSION:
    ret = handleVersion(params, len);
    break;
#ifdef USE_FFT_TABLES
  case OWL_SERVICE_ARM_RFFT_FAST_INIT_F32:
    ret = handleRFFT(params, len);    
    break;
  case OWL_SERVICE_ARM_CFFT_INIT_F32:
    ret = handleCFFT(params, len);
    break;
#endif /* USE_FFT_TABLES */
  case OWL_SERVICE_GET_PARAMETERS:
    ret = handleGetParameters(params, len);
    break;
  case OWL_SERVICE_GET_ARRAY:
    ret = handleGetArray(params, len);
    break;
  case OWL_SERVICE_LOAD_RESOURCE:
    ret = handleLoadResource(params, len);
    break;
  case OWL_SERVICE_REQUEST_CALLBACK:
    ret = handleRequestCallback(params, len);
    break;
  case OWL_SERVICE_REGISTER_CALLBACK:
    ret = handleRegisterCallback(params, len);
    break;
  }
  return ret;
}
