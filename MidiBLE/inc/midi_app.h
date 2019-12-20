#ifndef __MIDI_APP_H
#define __MIDI_APP_H

#include "bluenrg1_stack.h"

typedef enum 
{
  MIDI_APP_SUCCESS = 0x00,  	/*!< Success.*/
  MIDI_APP_ERROR = 0x10  	/*!< Error.*/
} MIDI_APP_Status;

extern volatile uint16_t ClassificationHandle;
extern volatile uint16_t ModeHandle;

MIDI_APP_Status CLASSIFICATION_APP_Init(void);

MIDI_APP_Status CLASSIFICATION_APP_add_char(uint16_t service_handle);
MIDI_APP_Status MODE_APP_add_char(uint16_t service_handle);
MIDI_APP_Status MIDI_APP_add_char(uint16_t service_handle);

MIDI_APP_Status CLASSIFICATION_APP_DataUpdate(uint16_t service_handle, uint8_t value);
uint8_t MODE_APP_DataRead(uint16_t service_handle);
MIDI_APP_Status MIDI_APP_DataUpdate(uint16_t service_handle, uint8_t channel, uint8_t note, uint8_t vector);
MIDI_APP_Status MIDI_APP_Passthrough(uint16_t service_handle, uint8_t*);

#endif /* __MIDI_APP_H */
