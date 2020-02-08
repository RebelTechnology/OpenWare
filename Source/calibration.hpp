#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include "ApplicationSettings.h"
#include "ProgramVector.h"

#define VOLTS_LO 1.0f
#define VOLTS_HI 3.0f
#define SAMPLE_LO -0.5f
#define SAMPLE_HI 0.5f
#define MIN_BUFFER_WRITES 8192
// Amount of bytes we want to send before reading calibration.
// We don't count buffers to avoid measuring input too early due to short buffer size.


class BaseCalibration {
protected:
  int32_t scalar;
  int32_t offset;

  void calculateScalarAndOffset(float sample1, float sample2, float volts1, float volts2){
    /*----------------------------------------------------------
    This is based on V/oct conversion code:

    Volts / multiplier + offset = sample

    We can derive from this the following:

    multiplier = (volts1 - volts2) / (sample1 - sample2)

    To get scalar variable, we divide multiplier by UINT16_MAX
    ----------------------------------------------------------*/
    float f_scalar = ((volts1 - volts2) / (sample1 - sample2) * UINT16_MAX);
    scalar = f_scalar;
    
    /*----------------------------------------------------------
    Now we use this formula:

    offset = sample - volts / multiplier

    This also has to be multiplied by UINT16_MAX to get integer
    value.
    ----------------------------------------------------------*/
    offset = ((sample1 - volts1 * UINT16_MAX / f_scalar) * UINT16_MAX);
  }
public:
  enum CalibrationMode {
    CAL_INPUT, CAL_OUTPUT
  };
  enum CalibrationState {
    CAL_LO, CAL_HI, CAL_DONE
  };
  enum CalibrationResults {
    CAL_SAVE, CAL_DISCARD
  };
  CalibrationResults results;
 
  float samples[2];
  CalibrationState state;

  BaseCalibration() {
    reset();
  }
  int32_t getScalar(){
    return scalar;
  }
  int32_t getOffset(){
    return offset;
  }
  virtual void calibrate() = 0;
  virtual bool readSample() = 0;  
  // readSample is boolean as we'll be writing data first and
  // reading it later for output calibration.
  virtual void storeResults() = 0;
  
  void reset() {
    samples[0] = 0.0f;
    samples[1] = 0.0f;
    state = CAL_LO;
  }

  bool isDone(){
    return state == CAL_DONE;
  }

  float getInput() {
    /*--------------------------------------------------------------
      We want to use whole buffer for measurement, but we only have
      8 bits left in ADC input (assuming 24 bit sample data stored as
      32 bit value. So we'll use minimum of 256 samples or buffer size.
      --------------------------------------------------------------*/
    ProgramVector* pv = getProgramVector();
    int32_t tmp_data = 0;
    uint8_t shift = 0;
    uint16_t num_samples = 1;
    while (num_samples < 256 && num_samples < settings.audio_blocksize) {
      num_samples = num_samples << 1;
      shift++;
    }
    uint16_t block_step = max(1, settings.audio_blocksize / num_samples);
    // Block step is used to skip some samples when audio buffer size > 256

    for (int i = 0; i < settings.audio_blocksize; i = i + block_step)
      // We read only left channel (* 2) and we do it every block_step samples.
      tmp_data += ((int32_t)((pv->audio_input[i * 2]) << 8)) >> shift;
    return (float)tmp_data / 2147483648.0f;
  }
  
  virtual void nextState(){
    switch (state){
    case CAL_LO:
      state = CAL_HI;
      break;
    case CAL_HI:
      state = CAL_DONE;
      break;
    case CAL_DONE:
      break;
    }
  }
};


class InputCalibration : public BaseCalibration {
 public:
  void calibrate(){
    // Calibrate based on known voltage and measured samples
    calculateScalarAndOffset(samples[0], samples[1], VOLTS_LO, VOLTS_HI);
  }

  void reset(){
    BaseCalibration::reset();
    results = CAL_SAVE;
    scalar = settings.input_scalar;
    offset = settings.input_offset;
  }
  
  bool readSample(){
    samples[(state == CAL_LO)?0:1] = getInput();
    return true;
  }

  void storeResults() {
    settings.input_scalar = scalar;
    settings.input_offset = offset;
    settings.saveToFlash();
  }
};

class OutputCalibration : public BaseCalibration {
  uint32_t data_written;
  float current_sample;
public:
  void calibrate(){
    // First measure voltage from calibrated input, then use it to determine scaling
    float input_multiplier = (float)(int32_t)settings.input_scalar / UINT16_MAX;
    float input_offset = (float)(int32_t)settings.input_offset / UINT16_MAX;

    /*----------------------------------------------------------
    We use input offset/scalar for measuring voltage here

    volts = (sample - offset) * multiplier

    Note that we're using samples from input, not output
    ----------------------------------------------------------*/
    float volts1 = (samples[0] - input_offset) * input_multiplier;
    float volts2 = (samples[1] - input_offset) * input_multiplier;

    // Samples from output are converted to normalized float values
    calculateScalarAndOffset(SAMPLE_LO, SAMPLE_HI, volts1, volts2);
  };
  
  void reset(){
    BaseCalibration::reset();
    scalar = settings.output_scalar;
    offset = settings.output_offset;
    data_written = 0;
    current_sample = SAMPLE_LO;
  };

  bool readSample(){
    if (data_written < MIN_BUFFER_WRITES) {
      // First we must write a certain of data to output buffer
      ProgramVector* pv = getProgramVector();      
      for (uint16_t i = 0; i < pv->audio_blocksize; i++) {
	pv->audio_output[i * 2] = (int32_t)(current_sample * 2147483648.0f) >> 8;
	data_written++;
      }
      return false;
    }
    else {
      // Ready to read results previously written
      samples[(state == CAL_LO)?0:1] = getInput();
      return true;      
    }    
  }

  void storeResults() {
    settings.output_scalar = scalar;
    settings.output_offset = offset;
    settings.saveToFlash();
  }

  void nextState(){
    BaseCalibration::nextState();
    current_sample = SAMPLE_HI;
    data_written = 0;
  }
};
#endif
