#ifndef __ParameterController_hpp__
#define __ParameterController_hpp__

#include <string.h>
#include "errorhandlers.h"
#include "ProgramVector.h"

class ParameterController {
protected:
  char title[11];
  int16_t parameters[NOF_PARAMETERS];
  char names[NOF_PARAMETERS][12];
public:
  virtual void reset(){
    for(size_t i=0; i<NOF_PARAMETERS; ++i){
      size_t c = 0;
      if(i >= 8)
	names[i][c++] = '@'+(i/8);
      names[i][c++] = 'A'+(i%8);
      names[i][c] = '\0';
      parameters[i] = 0;
    }
  }
  virtual void draw(ScreenBuffer& screen) = 0;
  /* Update parameters with encoders */
  virtual void updateEncoders(int16_t* data, uint8_t size) = 0;
  /* Update parameters with ADC values */
  virtual void updateValues(int16_t* values, size_t len){};
  virtual void setValue(uint8_t pid, int16_t value){    
    parameters[pid] = value;
  }
  const char* getName(uint8_t pid){
    if(pid < NOF_PARAMETERS)
      return names[pid];
    return "";
  }
  // bool isInput(uint8_t pid)
  // bool isBipolar(uint8_t pid)
  virtual void setName(uint8_t pid, const char* name){
    // todo : set direction (input/output), type (parameter/button)
    if(pid < NOF_PARAMETERS)
      strncpy(names[pid], name, 11);
  }
  int16_t getValue(uint8_t pid){
    return parameters[pid];
  }
  int16_t* getParameters(){
    return parameters;
  }
  size_t getSize(){
    return NOF_PARAMETERS;
  }
  void setTitle(const char* str){
    strncpy(title, str, sizeof(title)-1);    
  }
  const char* getTitle(){
    return title;
  }
};  

#endif // __ParameterController_hpp__
