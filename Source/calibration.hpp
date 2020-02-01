#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include "ApplicationSettings.h"
#include "ProgramVector.h"

#define VOLTS_LO 1.0f
#define VOLTS_HI 3.0f

class BaseCalibration {
protected:
  int32_t scalar;
  int32_t offset;
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
  }

  bool isDone(){
    return state == CAL_DONE;
  }

  float getInput() {
    ProgramVector* pv = getProgramVector();
    return (float)(int32_t)((pv->audio_input[0])<<8) / 2147483648.0f;
  }
  
  virtual void nextState() = 0;
};


class InputCalibration : public BaseCalibration {
 public:
  // Note: calibration tested with ER-101, results are spot on.
  void calibrate(){
    /*----------------------------------------------------------
    This is based on V/oct conversion code:

    Volts / multiplier + offset = sample

    We can derive from this the following:

    multiplier = (volts1 - volts2) / (sample1 - sample2)

    To get scalar variable, we multipy multiplier by UINT16_MAX
    ----------------------------------------------------------*/
    float f_scalar = (int32_t)((VOLTS_LO - VOLTS_HI) / (samples[0] - samples[1]) * UINT16_MAX);
    scalar = (int32_t)f_scalar;
    
    /*----------------------------------------------------------
    Now we use this formula:

    offset = sample  - volts / multiplier

    This also has to be multiplied by UINT16_MAX to get integer
    value.
    ----------------------------------------------------------*/
    offset = (int32_t)((samples[0] - VOLTS_LO * UINT16_MAX / f_scalar) * UINT16_MAX);
  }

  void reset(){
    BaseCalibration::reset();
    state = CAL_LO;
    results = CAL_SAVE;
    scalar = settings.input_scalar;
    offset = settings.input_offset;
  }
  
  void nextState(){
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
  
  bool readSample(){
    ProgramVector* pv = getProgramVector();
    // TODO: use average over multiple values for better precision?
    samples[(state == CAL_LO)?0:1] = (int32_t)(pv->audio_input[0] << 8) / 2147483648.0;
    return true;
  }

  void storeResults() {
    settings.input_scalar = scalar;
    settings.input_offset = offset;
    settings.saveToFlash();
  }
};

class OutputCalibration : public BaseCalibration {
 public:
  void calibrate(){
    // TBD
  };
  
  void reset(){
    BaseCalibration::reset();
    scalar = settings.output_scalar;
    offset = settings.output_offset;    
  };
  void nextState(){};  

  bool readSample(){
    // We'll write samples to output for a few cycles, then read it from input
    return false;
  }

  void storeResults() {
    settings.output_scalar = scalar;
    settings.output_offset = offset;
    settings.saveToFlash();
  }
    
};
#endif
