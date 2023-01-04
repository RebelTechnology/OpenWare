#ifndef _MidiStatus_h_
#define _MidiStatus_h_

enum MidiStatus {
  STATUS_BYTE			= 0x80,
  NOTE_OFF			= 0x80,
  NOTE_ON			= 0x90,
  POLY_KEY_PRESSURE		= 0xA0,
  CONTROL_CHANGE		= 0xB0,
  PROGRAM_CHANGE		= 0xC0,
  CHANNEL_PRESSURE		= 0xD0,
  PITCH_BEND_CHANGE		= 0xE0,
  SYSTEM_COMMON			= 0xF0,
  SYSEX				= 0xF0,
  TIME_CODE_QUARTER_FRAME       = 0xF1,
  SONG_POSITION_PTR             = 0xF2,
  SONG_SELECT                   = 0xF3,
  RESERVED_F4                   = 0xF4,
  RESERVED_F5                   = 0xF5,
  TUNE_REQUEST                  = 0xF6,
  SYSEX_EOX                     = 0xF7,
  SYSTEM_REAL_TIME		= 0xF8,
  TIMING_CLOCK		        = 0xF8,
  RESERVED_F9                   = 0xF9,
  START                         = 0xFA,
  CONTINUE                      = 0xFB,
  STOP                          = 0xFC,
  RESERVED_FD                   = 0xFD,
  ACTIVE_SENSING                = 0xFE,
  SYSTEM_RESET                  = 0xFF,
  MIDI_CHANNEL_MASK		= 0x0F,
  MIDI_STATUS_MASK		= 0xF0
};

enum MidiControlChange {
  MIDI_CC_MODULATION    = 1,
  MIDI_CC_BREATH        = 2,
  MIDI_CC_DATAENTRY_MSB = 6,
  MIDI_CC_VOLUME        = 7,
  MIDI_CC_BALANCE       = 8,
  MIDI_CC_PAN           = 10,
  MIDI_CC_EXPRESSION    = 11,
  MIDI_CC_EFFECT_CTRL_1 = 12,
  MIDI_CC_EFFECT_CTRL_2 = 13,
  MIDI_CC_DATAENTRY_LSB = 38,
  MIDI_CC_SUSTAIN       = 64,
  MIDI_CC_PORTAMENTO    = 65,
  MIDI_CC_SOSTENUTO     = 66,
  MIDI_CC_FREQ_CUTOFF   = 74,
  MIDI_CC_DATA_INCR     = 96,
  MIDI_CC_DATA_DECR     = 97,
  MIDI_CC_RPN_LSB       = 100,
  MIDI_CC_RPN_MSB       = 101,
  MIDI_ALL_SOUND_OFF    = 120,
  MIDI_RESET_ALL_CTRLS  = 121,
  MIDI_LOCAL_CONTROL    = 122,
  MIDI_ALL_NOTES_OFF    = 123,
  MIDI_OMNI_MODE_OFF    = 124,
  MIDI_OMNI_MODE_ON     = 125,
  MIDI_MONO_MODE_ON     = 126,
  MIDI_POLY_MODE_ON     = 127
};

enum UsbMidi {
  USB_COMMAND_MISC                = 0x00,	/* reserved */
  USB_COMMAND_CABLE_EVENT         = 0x01,	/* reserved */
  USB_COMMAND_2BYTE_SYSTEM_COMMON = 0x02,	/* e.g. MTC, SongSelect */
  USB_COMMAND_3BYTE_SYSTEM_COMMON = 0x03,	/* e.g. Song Position Pointer SPP */
  USB_COMMAND_SYSEX               = 0x04,
  USB_COMMAND_SYSEX_EOX1          = 0x05,
  USB_COMMAND_SYSEX_EOX2          = 0x06,
  USB_COMMAND_SYSEX_EOX3          = 0x07,
  USB_COMMAND_NOTE_OFF            = 0x08,
  USB_COMMAND_NOTE_ON             = 0x09,
  USB_COMMAND_POLY_KEY_PRESSURE   = 0x0A,
  USB_COMMAND_CONTROL_CHANGE	  = 0x0B,
  USB_COMMAND_PROGRAM_CHANGE	  = 0x0C,
  USB_COMMAND_CHANNEL_PRESSURE	  = 0x0D,
  USB_COMMAND_PITCH_BEND_CHANGE	  = 0x0E,
  USB_COMMAND_SINGLE_BYTE	  = 0x0F
};

enum OwlProtocol {
  OWL_COMMAND_DISCOVER            = 0xa0,
  OWL_COMMAND_PARAMETER           = 0xb0,
  OWL_COMMAND_COMMAND             = 0xc0,
  OWL_COMMAND_MESSAGE             = 0xd0,
  OWL_COMMAND_DATA                = 0xe0,
  OWL_COMMAND_RESET               = 0xf0,
};

enum MidiRPN {
  MIDI_RPN_PITCH_BEND_RANGE       = 0x0000, // GM1
  MIDI_RPN_FINE_TUNING            = 0x0001, // GM1
  MIDI_RPN_COARSE_TUNING          = 0x0002, // GM1
  MIDI_RPN_MODULATION_DEPTH_RANGE = 0x0005, // GM2
  MIDI_RPN_MPE_CONFIGURATION      = 0x0006, // MPE
  MIDI_RPN_RESET                  = 0x3fff,
};

#endif /* _MidiStatus_h_ */
