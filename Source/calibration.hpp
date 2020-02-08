#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include "ApplicationSettings.h"
#include "ProgramVector.h"

#define VOLTS_LO 1.0f
#define VOLTS_HI 3.0f
#define SAMPLE_LO -0.5f
#define SAMPLE_HI 0.5f


#define MIN_BUFFER_WRITES 8192
// Amount of bytes we want to send before reading calibration
// We don't just count buffer in case if buffer size would change


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
    // Returns input sample converted to float
    ProgramVector* pv = getProgramVector();
    return (float)(int32_t)((pv->audio_input[0])<<8) / 2147483648.0f;
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
  // Note: calibration tested with ER-101, results are spot on.
  void calibrate(){
    calculateScalarAndOffset(samples[0], samples[1], VOLTS_LO, VOLTS_HI);
  }

  void reset(){
    BaseCalibration::reset();
    results = CAL_SAVE;
    scalar = settings.input_scalar;
    offset = settings.input_offset;
  }
  
  bool readSample(){
    ProgramVector* pv = getProgramVector();
    // TODO: use average over multiple values for better precision?
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
    if (data_written >= MIN_BUFFER_WRITES) {
      // Ready to read results
      samples[(state == CAL_LO)?0:1] = getInput();
      return true;
    }
    else {
      ProgramVector* pv = getProgramVector();      
      for (uint16_t i = 0; i < pv->audio_blocksize; i++) {
	pv->audio_output[i * 2] = (int32_t)(current_sample * 2147483648.0f) >> 8;
	data_written++;
      }
      return false;
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
