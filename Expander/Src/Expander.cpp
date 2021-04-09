#include <stdlib.h>
#include "device.h"
#include "HAL_TLC5946.h"
#include "bus.h"
#include "gpio.h"
#include "clock.h"
#include "message.h"
#include "OpenWareMidiControl.h"
#include "basicmaths.h"
#include "SmoothValue.h"

#if defined USE_MAX || defined USE_MAX_DMA
#include "HAL_MAX11300.h"
extern SPI_HandleTypeDef MAX11300_SPI;
#endif

ApplicationSettings settings;

#if 0
#include "DigitalBusStreamReader.h"
extern DigitalBusStreamReader bus;
// c functions used in interrupts
void serial_rx_callback(uint8_t c){
  bus.read(c);
}

uint8_t serial_tx_available(){
  return bus_tx_buf.notEmpty();
}

uint8_t serial_tx_pull(){
  return bus_tx_buf.pull();
}
#endif

#include "MidiHandler.h"

extern "C"  void setParameterValue(uint8_t pid, int16_t value);

MidiHandler::MidiHandler(){
  // memset(midi_values, 0, NOF_PARAMETERS*sizeof(uint16_t));
}

void MidiHandler::handlePitchBend(uint8_t status, uint16_t value){
  // setParameter(PARAMETER_G, ((int16_t)value - 8192)>>1);
}

void MidiHandler::handleNoteOn(uint8_t status, uint8_t note, uint8_t velocity){
  // setButton(MIDI_NOTE_BUTTON+note, velocity<<5);
}

void MidiHandler::handleNoteOff(uint8_t status, uint8_t note, uint8_t velocity){
  // setButton(MIDI_NOTE_BUTTON+note, 0);
}

void MidiHandler::handleProgramChange(uint8_t status, uint8_t pid){
}

void MidiHandler::handleControlChange(uint8_t status, uint8_t cc, uint8_t value){
  if(cc >= PATCH_PARAMETER_AA && cc <= PATCH_PARAMETER_BH)
    setParameterValue(PARAMETER_AA+(cc-PATCH_PARAMETER_AA), value<<5);
}

void MidiHandler::handleSysEx(uint8_t* data, uint16_t size){}

void MidiHandler::handleSystemRealTime(uint8_t cmd){
}

void MidiHandler::handleSystemCommon(uint8_t cmd1, uint8_t cmd2){
}

void MidiHandler::handleSystemCommon(uint8_t cmd1, uint8_t cmd2, uint8_t cmd3){
}

void MidiHandler::handleChannelPressure(uint8_t status, uint8_t value){
}

void MidiHandler::handlePolyKeyPressure(uint8_t status, uint8_t note, uint8_t value){
}

/**
 * MAX channel index goes from 0 at top left, to 8 at bottom right, to 15 at top right.
 * LED channel index is the inverse of MAX channel index.
 */

extern "C" {
#ifdef USE_TLC
  extern SPI_HandleTypeDef TLC5946_SPI;
#endif
  void setup(void);
  void run(void);
}

// #define HYSTERESIS_DELTA 3
static uint16_t HYSTERESIS_DELTA = 7;

enum ChannelMode {
  ADC_5TO5,
  ADC_0TO10,
  ADC_10TO0,
  DAC_MODE,
  DAC_5TO5,
  DAC_0TO10,
  DAC_10TO0,
  CHANNEL_MODES
};

uint8_t cc_values[MAX11300_CHANNELS] = {0};
ChannelMode cfg[MAX11300_CHANNELS];
SmoothFloat dac[MAX11300_CHANNELS];
int adc[MAX11300_CHANNELS];

// #define USE_TEMP
#ifdef USE_TEMP
volatile float temp[3] = {0};
#endif

void setLed(uint8_t ch, int16_t value);
void configureChannel(uint8_t ch, ChannelMode mode);
void setDAC(uint8_t ch, int16_t value);
void setADC(uint8_t ch, int16_t value);
uint8_t getChannelIndex(uint8_t ch);

void setup(){
  MAX11300_init(&MAX11300_SPI);
  TLC5946_init(&TLC5946_SPI);

  // setPin(TLC_BLANK_GPIO_Port, TLC_BLANK_Pin); // bring BLANK high to turn LEDs off

  for(int ch=0; ch<MAX11300_CHANNELS; ++ch){
    adc[ch] = 0;
    dac[ch] = 0;
    dac[ch].lambda = 0.95;
    // cfg[ch] = ADC_0TO10;
    // cfg[ch] = DAC_0TO10;
    cfg[ch] = ADC_5TO5;
    // cfg[ch] = ch < 8 ? ADC_0TO10 : ADC_5TO5;
  }

  for(int ch=0; ch<MAX11300_CHANNELS; ++ch)
    configureChannel(ch, cfg[ch]);

  bus_setup();

  for(int ch=0; ch<TLC5940_CHANNELS; ++ch)
    TLC5946_SetOutput_DC(0, ch, 0xff);

#ifdef USE_TLC
  TLC5946_Refresh_DC();
  HAL_Delay(10);
  TLC5946_Refresh_GS(); // starts endless refresh loop
#endif

  int delayms = 1;
  // for(int i=0; i<8192; ++i){
  for(int i=0; i<=4096; ++i){
    for(int ch=0; ch<TLC5940_CHANNELS; ++ch)
      setLed(ch, i&0x0fff);
#ifndef TLC_CONTINUOUS
    TLC5946_Refresh_GS();
#endif
    HAL_Delay(delayms);
  }

#if defined USE_MAX || defined  USE_MAX_DMA
  MAX11300_setDeviceControl(DCR_DACCTL_ImmUpdate|DCR_DACREF_Int|DCR_ADCCTL_ContSweep/*|DCR_BRST_Contextual*/);
#endif
#if defined USE_MAX_DMA && defined MAX_CONTINUOUS
  MAX11300_startContinuous();
#endif
}

void configureChannel(uint8_t ch, ChannelMode mode){
  switch(mode){
#if defined USE_MAX || defined USE_MAX_DMA
  case ADC_5TO5:
    MAX11300_setPortMode(PORT_1+ch, PCR_Range_ADC_M5_P5|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
    break;
  case ADC_0TO10:
    MAX11300_setPortMode(PORT_1+ch, PCR_Range_ADC_0_P10|PCR_Mode_ADC_SgEn_PosIn|PCR_ADCSamples_16|PCR_ADCref_INT);
    break;
  case DAC_5TO5:
    MAX11300_setPortMode(PORT_1+ch, PCR_Range_DAC_M5_P5|PCR_Mode_DAC);
    break;
  case DAC_0TO10:
    MAX11300_setPortMode(PORT_1+ch, PCR_Range_DAC_0_P10|PCR_Mode_DAC);
    break;
#endif
  default:
    debug << "Invalid mode [" << mode << "]\r\n";
    return;
    break;
  }
  cfg[ch] = mode;
}

void setLed(uint8_t ch, int16_t value){
#ifdef USE_TLC
  if(ch < TLC5940_CHANNELS){
    // note that LED channel index is inverse of MAX channel index
    if(cfg[ch] == DAC_5TO5 || cfg[ch] == ADC_5TO5)
      TLC5946_SetOutput_GS(0, 15-ch, max(0, min(4095, abs(value-2048)*2)));
    else
      TLC5946_SetOutput_GS(0, 15-ch, max(0, min(4095, value)));
  }
#endif
}

void setDAC(uint8_t ch, int16_t value){
  if(ch < MAX11300_CHANNELS){
    if(cfg[ch] < DAC_MODE){
      // auto configure to output
      if(value < 0){
        configureChannel(ch, DAC_5TO5);
      }else{
        configureChannel(ch, DAC_0TO10);
      }
    }
    if(cfg[ch] == DAC_5TO5)
      value += 2048;
    value = max(0, min(4095, value));
    if(dac[ch] != value){
      dac[ch] = value;
      setLed(ch, value);
    }
  }
}

void setADC(uint8_t ch, int16_t value){
  if(ch < MAX11300_CHANNELS){
    if(adc[ch] != value){
      adc[ch] = value;
      setLed(ch, value);
    }
  }
}

uint8_t getChannelIndex(uint8_t ch){
  if(ch > 7)
    ch = 23-ch;
  return ch;
}

uint16_t getPortValue(uint8_t ch){
#if defined USE_MAX
  return MAX11300_readADC(ch);
#elif defined USE_MAX_DMA
  return MAX11300_getADCValue(ch);
#endif
}

void setPortValue(uint8_t ch, uint16_t value){
#if defined USE_MAX
  MAX11300_setDAC(ch, value);
#elif defined USE_MAX_DMA
  MAX11300_setDACValue(ch, value);
#endif
}

void loop(){
  bus_status();
#if defined USE_MAX_DMA && !defined MAX_CONTINUOUS
  MAX11300_bulksetDAC();
#endif
  for(int ch=0; ch<MAX11300_CHANNELS; ++ch){
    if(cfg[ch] < DAC_MODE){
      setADC(ch, getPortValue(ch));
      uint8_t cc = adc[ch] >> 5;
      if(abs(cc - cc_values[ch]) > HYSTERESIS_DELTA){
        bus_tx_parameter(getChannelIndex(PARAMETER_AA+ch), adc[ch]);
        cc_values[ch] = cc;
      }
    }
  }
  bus_status();
#if defined USE_MAX_DMA && !defined MAX_CONTINUOUS
  MAX11300_bulkreadADC();
#endif
  for(int ch=0; ch<MAX11300_CHANNELS; ++ch){
    if(cfg[ch] > DAC_MODE){
      setPortValue(ch, dac[ch]);
      setLed(ch, dac[ch]);
    }
  }
#if defined USE_TLC && !defined TLC_CONTINUOUS
  TLC5946_Refresh_GS();
#endif
}

void run(){
  for(;;){
    loop();
  }
}

void setParameterValue(uint8_t pid, int16_t value){
  uint8_t ch = getChannelIndex(pid-PARAMETER_AA);
  setDAC(ch, value);
}

void bus_rx_command(uint8_t cmd, int16_t data){
  if(cmd == BUS_CMD_CONFIGURE_IO){
    uint8_t ch = getChannelIndex(data>>8);
    ChannelMode mode = (ChannelMode)(data&0xff);
    // debug << "rx command [" << cmd << "][" << ch << "][" << mode << "]\r\n" ;
    if(ch < MAX11300_CHANNELS){
      configureChannel(ch, mode);
      bus_tx_message("Configured channel");
    }
  }
}

void bus_rx_button(uint8_t bid, int16_t value){
  // debug << "rx button [" << bid << "][" << value << "]\r\n" ;
}

void bus_rx_message(const char* msg){
  // debug << "rx msg [" << msg << "]\r\n" ;
}

void bus_rx_data(const uint8_t* data, uint16_t size){
  // debug << "rx data [" << size << "]\r\n" ;
}
