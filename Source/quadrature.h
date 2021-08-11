/* https://electronics.stackexchange.com/questions/99915/stm32-rotary-encoder-with-hardware-interrupts */

int16_t encoder_data[ENCODER_COUNT];
static uint8_t encoder_state[ENCODER_COUNT];

/** 
 * Initialise encoder with the given state and value
 */
void encoder_init(uint8_t eid, uint8_t state, int16_t value);

/** 
 * Interrupt handler for rising and falling edges on both pins.
 * @param state encoder bitfield with pin status A and B in bits 1 and 0.
 */
void encoder_interrupt(uint8_t eid, uint8_t state);

#ifdef ENCODER_ERROR_DETECTION
// increments and decrements shifted <<1, error in LSB
// illegal transitions are those where both pins change at once, e.g. from 0b10 to 0b01
static const int8_t transitions[16] = {0, -2, 2, 1, 2, 0, 1, -2, -2, 1, 0, 2, 1, 2, -2, 0};
size_t encoder_errors = 0;
void encoder_interrupt(uint8_t eid, uint8_t state) {
  uint8_t previous = encoder_state[eid];
  state |= (previous<<2)&0x0f;
  int8_t delta = transitions[state];
  encoder_data[eid] += delta >> 1;
  encoder_errors += delta & 0x01;
  encoder_state[eid] = state;
}
#else
// increments and decrements for all possible (and impossible) transitions
static const int8_t transitions[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

void encoder_interrupt(uint8_t eid, uint8_t state) {
 /* https://electronics.stackexchange.com/questions/99915/stm32-rotary-encoder-with-hardware-interrupts */
  uint8_t previous = encoder_state[eid];
  state |= (previous<<2)&0x0f;
  encoder_data[eid] += transitions[state];
  encoder_state[eid] = state;
}
#endif

void encoder_init(uint8_t eid, uint8_t state, int16_t value){
  encoder_data[eid] = value;
  encoder_state[eid] = (state << 2) | state;
}  
