#include <stdint.h>

template<size_t SIZE, typename value_t>
class TakeoverControls {
private:
  value_t values[SIZE];
  bool takeover[SIZE];
public:
  TakeoverControls(){
    reset(true);
  }
  value_t get(uint8_t index){
    return values[index];
  }
  void set(uint8_t index, value_t value){
    values[index] = value;
  }
  void update(uint8_t index, value_t value, value_t threshold){
    if(takeover[index]){
      values[index] = value;
    }else if(abs(values[index] - value) < threshold){
      takeover[index] = true;
      values[index] = value;
    }
  }
  bool taken(uint8_t index){
    return takeover[index];
  }
  /**
   * If @param state is true, then the control is taken.
   * If the control is taken it reflects the current knob setting.
   */
  void reset(uint8_t index, bool state){
    takeover[index] = state;
  }
  void reset(bool state){
    for(size_t i=0; i<SIZE; ++i)
      takeover[i] = state;
  }
};
