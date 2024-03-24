#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "Owl.h"
#include "errorhandlers.h"
#include "message.h"
#include "ProgramManager.h"
#include "OpenWareMidiControl.h"
#include "Pin.h"
#include "Storage.h"
#include "MidiMessage.h"
#include "cmsis_os.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef abs
#define abs(x) ((x) > 0 ? (x) : -(x))
#endif

#define PATCH_RESET_COUNTER (500/MAIN_LOOP_SLEEP_MS)

#define NOF_CALIBRATION_DATA 22 // 0-10V (11 values) * 2

#define INLEVELGREEN PARAMETER_AF
#define MOD PARAMETER_AG

// GPIO
#define SHIFT_BUTTON PUSHBUTTON
#define CU_DOWN GREEN_BUTTON
#define CU_UP RED_BUTTON
#define RECORD_BUTTON BUTTON_1
#define RECORD_GATE BUTTON_2
#define RANDOM_BUTTON BUTTON_3
#define RANDOM_GATE BUTTON_4
#define SYNC_GATE BUTTON_5
#define INLEVELRED BUTTON_6
#define PREPOST_SWITCH BUTTON_7
#define SSWT_SWITCH BUTTON_8
#define MOD_CV_GREEN BUTTON_9
#define MOD_CV_RED BUTTON_10
#define MOD_CV_BUTTON BUTTON_11

#define RANDOM_AMOUNT PARAMETER_AA

// ADC3
#define OSC_DETUNE_CV PARAMETER_A
#define FILTER_CUTOFF_CV PARAMETER_B
#define RESONATOR_HARMONY_CV PARAMETER_C
#define DELAY_TIME_CV PARAMETER_D
#define LOOPER_START_CV PARAMETER_E
#define LOOPER_LENGTH_CV PARAMETER_F
#define LOOPER_SPEED_CV PARAMETER_G

// Muxed
#define REVERB_TONESIZE_CV PARAMETER_AC
#define OSC_VOCT_CV PARAMETER_AD

#define LOOPER_VOL PARAMETER_BA
#define REVERB_VOL PARAMETER_BB
#define DELAY_VOL PARAMETER_BC
#define RESONATOR_VOL PARAMETER_BD
#define FILTER_VOL PARAMETER_BE
#define IN_VOL PARAMETER_BF
#define SSWT_VOL PARAMETER_BG
#define SINE_VOL PARAMETER_BH

#define LOOPER_SPEED PARAMETER_CA
#define FILTER_RESODRIVE PARAMETER_CB
#define OSC_DETUNE PARAMETER_CC
#define LOOPER_LENGTH PARAMETER_CD
#define OSC_PITCH PARAMETER_CE
#define LOOPER_START PARAMETER_CF
#define RESONATOR_HARMONY PARAMETER_CG
#define RESONATOR_DECAY PARAMETER_CH

#define REVERB_TONESIZE PARAMETER_DA
#define REVERB_DECAY PARAMETER_DB
#define MOD_LEVEL PARAMETER_DC
#define MOD_FREQ PARAMETER_DD
#define FILTER_CUTOFF PARAMETER_DE
#define DELAY_FEEDBACK PARAMETER_DF
#define DELAY_TIME PARAMETER_DG
#define RANDOM_MODE PARAMETER_DH

enum leds
{
  RECORD_LED = 1,
  RANDOM_LED,
  SYNC_LED,
  INLEVELRED_LED,
  INLEVELGREEN_LED,
  MOD_LED,
  CU_DOWN_LED,
  CU_UP_LED,
  SHIFT_LED,
  MOD_CV_GREEN_LED,
  MOD_CV_RED_LED,
};

enum ConfigMode
{
  CONFIG_MODE_NONE,
  CONFIG_MODE_CALIBRATION,
  CONFIG_MODE_OPTIONS,
};

enum CalibrationStep
{
  CALIBRATION_NONE,
  CALIBRATION_C1,
  CALIBRATION_C3,
  CALIBRATION_PARAMS,
};

struct Configuration
{
  uint32_t voct_scale;
  uint32_t voct_offset;
  bool soft_takeover;
  bool mod_attenuverters;
  bool cv_attenuverters;
  uint16_t params_min[40];
  uint16_t params_max[40];
};

Configuration configuration = {
  (uint32_t)118.581421f * UINT16_MAX,
  (uint32_t)2.18576622f * UINT16_MAX,
  true,
  false,
  false,
  {1000},
  {1000}
};

CalibrationStep calibrationStep = CALIBRATION_NONE;
float c3 = -1;
float c1 = -1;
float calibrationValue = -1;

ConfigMode configMode = CONFIG_MODE_NONE;

static bool randomButtonState = false;
static bool sswtSwitchState = false;
static bool shiftButtonState = false;
static bool modCvButtonState = false;
static uint16_t randomAmountState = 0;
static uint16_t mux_values[NOF_MUX_VALUES] DMA_RAM = {};

//Settings settings;

Pin randomGate(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin);
Pin randomButton(RANDOM_BUTTON_GPIO_Port, RANDOM_BUTTON_Pin);
Pin shiftButton(SHIFT_BUTTON_GPIO_Port, SHIFT_BUTTON_Pin);
Pin sswtSwitch(SSWT_SWITCH_GPIO_Port, SSWT_SWITCH_Pin);
Pin randomAmountSwitch1(RANDOM_AMOUNT_SWITCH1_GPIO_Port, RANDOM_AMOUNT_SWITCH1_Pin);
Pin randomAmountSwitch2(RANDOM_AMOUNT_SWITCH2_GPIO_Port, RANDOM_AMOUNT_SWITCH2_Pin);
Pin modCvButton(MOD_CV_BUTTON_GPIO_Port, MOD_CV_BUTTON_Pin);

//Pin funcLed1(FUNC_1_LED_GPIO_Port, FUNC_1_LED_Pin);
//Pin funcLed2(SHIFT_LED_GPIO_Port, SHIFT_LED_Pin);

// MUX binary counter digital output pins
Pin muxA(MUX_A_GPIO_Port, MUX_A_Pin);
Pin muxB(MUX_B_GPIO_Port, MUX_B_Pin);
Pin muxC(MUX_C_GPIO_Port, MUX_C_Pin);

void test()
{
    // Start the save process.
    MidiMessage msg = MidiMessage(USB_COMMAND_SINGLE_BYTE, START, 0, 0);
    midi_send(msg.data[0], msg.data[1], msg.data[2], msg.data[3]); // send MIDI START
    int16_t values[13];
    for (size_t i = 0; i < 13; i++)
    {
        // Convert to 14-bit signed int.
        int16_t value = (int16_t)(values[i] * 8192);
        // Send the parameter's value.
        MidiMessage msg = MidiMessage::pb(i, value);
        midi_send(msg.data[0], msg.data[1], msg.data[2], msg.data[3]); // send MIDI PITCH BEND
    }
    // Send the file index.
    int8_t fileIndex = 0; // 0: "oneiroi.cfg", 1: "oneiroi.alt", 2: "oneiroi.mod", 3: "oneiroi.cv"
    msg = MidiMessage::cp(0, fileIndex);
    midi_send(msg.data[0], msg.data[1], msg.data[2], msg.data[3]); // send MIDI CHANNEL PRESSURE

    // Finish the process.
    msg = MidiMessage(USB_COMMAND_SINGLE_BYTE, STOP, 0, 0);
    midi_send(msg.data[0], msg.data[1], msg.data[2], msg.data[3]); // send MIDI STOP
}

void saveConfiguration()
{
  debugMessage("Saving configuration");
  uint32_t headerSize = sizeof(ResourceHeader);
  uint32_t dataSize = sizeof(configuration);
  uint8_t buffer[headerSize + dataSize];
  memset(buffer, 0, headerSize);
  memcpy(buffer + headerSize, &configuration, dataSize);
  const char* filename = "oneiroi.cfg";
  taskENTER_CRITICAL();
  storage.writeResource(filename, buffer, dataSize, FLASH_DEFAULT_FLAGS);
  taskEXIT_CRITICAL();
  debugMessage(filename, (int)dataSize);
}

void loadConfiguration()
{
  debugMessage("Loading configuration");
  Resource* resource = storage.getResourceByName("oneiroi.cfg");
  if (resource)
  {
    storage.readResource(resource->getHeader(), &configuration, 0, sizeof(configuration));
  }
}

void setVoctParameterValue(int16_t value)
{
  int16_t previous = getParameterValue(OSC_VOCT_CV);
  // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
  value = (float)((previous * 3 + value) >> 2);

  setParameterValue(OSC_VOCT_CV, value);
}

void setCalibratedParameterValue(uint8_t pid, int16_t value)
{
  int16_t previous = getParameterValue(pid);
  // IIR exponential filter with lambda 0.75: y[n] = 0.75*y[n-1] + 0.25*x[n]
  float v = (float)((previous * 3 + value) >> 2) ;

  if (value < configuration.params_min[pid])
  {
    value = configuration.params_min[pid];
  }
  else if (value > configuration.params_max[pid])
  {
    value = configuration.params_max[pid];
  }
  v = (4095.f / (configuration.params_max[pid] - configuration.params_min[pid])) * (value - configuration.params_min[pid]);

  setParameterValue(pid, v);
}

void setMux(uint8_t index)
{
  muxA.set(index & 0b001);
  muxB.set(index & 0b010);
  muxC.set(index & 0b100);
}

void readMux(uint8_t index, uint16_t *mux_values)
{
  uint16_t muxA = 4095 - mux_values[MUX_A];
  uint16_t muxB = 4095 - mux_values[MUX_B];
  uint16_t muxC = 4095 - mux_values[MUX_C];
  uint16_t muxD = 4095 - mux_values[MUX_D];
  uint16_t muxE = 4095 - mux_values[MUX_E];

  setCalibratedParameterValue(REVERB_TONESIZE_CV, muxA);
  setVoctParameterValue(muxB);
  setCalibratedParameterValue(PARAMETER_BA + index, muxC);
  setCalibratedParameterValue(PARAMETER_CA + index, muxD);
  setCalibratedParameterValue(PARAMETER_DA + index, muxE);

  if (CALIBRATION_C1 == calibrationStep)
  {
    c1 = muxB / 4096.f;
  }
  else if (CALIBRATION_C3 == calibrationStep)
  {
    c3 = muxB / 4096.f;
  }
  else if (CALIBRATION_PARAMS == calibrationStep)
  {
    configuration.params_min[PARAMETER_BA + index] = min(configuration.params_min[PARAMETER_BA + index], 4095-mux_values[MUX_C]);
    configuration.params_min[PARAMETER_CA + index] = min(configuration.params_min[PARAMETER_CA + index], 4095-mux_values[MUX_D]);
    if (index < 7)
    {
      // Exclude random mode switch.
      configuration.params_min[PARAMETER_DA + index] = min(configuration.params_min[PARAMETER_DA + index], 4095-mux_values[MUX_E]);
    }
    configuration.params_max[PARAMETER_BA + index] = max(configuration.params_max[PARAMETER_BA + index], 4095-mux_values[MUX_C]);
    configuration.params_max[PARAMETER_CA + index] = max(configuration.params_max[PARAMETER_CA + index], 4095-mux_values[MUX_D]);
    if (index < 7)
    {
      // Exclude random mode switch.
      configuration.params_max[PARAMETER_DA + index] = max(configuration.params_max[PARAMETER_DA + index], 4095-mux_values[MUX_E]);
    }
  }
}

extern "C"
{
  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
  {
    extern ADC_HandleTypeDef ADC_PERIPH;
    extern ADC_HandleTypeDef MUX_PERIPH;
    extern uint16_t adc_values[NOF_ADC_VALUES];
    if (hadc == &MUX_PERIPH)
    {
      static uint8_t mux_index = 0;
      setMux(mux_index + 1);
      readMux(mux_index, mux_values);
      mux_index = (mux_index + 1) & 0b111;
    }
    else if (hadc == &ADC_PERIPH)
    {
      for (size_t i = 0; i < NOF_ADC_VALUES; i++)
      {
        uint16_t value = 4095 - adc_values[i];
        setCalibratedParameterValue(i, value);
      }
    }
  }
}

void readGpio()
{
  if (randomButtonState != !randomButton.get()) // Inverted: pressed = false
  {
    randomButtonState = !randomButton.get();
    setButtonValue(RANDOM_BUTTON, randomButtonState);
    if (CONFIG_MODE_OPTIONS == configMode && randomButtonState)
    {
      configuration.cv_attenuverters = !configuration.cv_attenuverters;
    }
  }
  if (sswtSwitchState != !sswtSwitch.get()) // Inverted: pressed = false
  {
    sswtSwitchState = !sswtSwitch.get();
    setButtonValue(SSWT_SWITCH, sswtSwitchState);
  }
  if (shiftButtonState != !shiftButton.get()) // Inverted: pressed = false
  {
    shiftButtonState = !shiftButton.get();
    setButtonValue(SHIFT_BUTTON, shiftButtonState);
    if (CONFIG_MODE_OPTIONS == configMode && shiftButtonState)
    {
      configuration.soft_takeover = !configuration.soft_takeover;
    }
  }
  if (modCvButtonState != !modCvButton.get()) // Inverted: pressed = false
  {
    modCvButtonState = !modCvButton.get();
    setButtonValue(MOD_CV_BUTTON, modCvButtonState);
  }

  setAnalogValue(MOD_LED, getParameterValue(MOD));
  setAnalogValue(INLEVELGREEN_LED, getParameterValue(INLEVELGREEN));

#ifdef DEBUG

  int16_t delayCv = getParameterValue(DELAY_TIME_CV); // Ok (-5 - 10v)
  int16_t osc2Cv = getParameterValue(OSC_DETUNE_CV); // Ok (-5 - 10v)
  int16_t filterCv = getParameterValue(FILTER_CUTOFF_CV); // Ok (-5 - 10v)
  int16_t startCv = getParameterValue(LOOPER_START_CV); // Ok (-5 - 10v)
  int16_t lengthCv = getParameterValue(LOOPER_LENGTH_CV); // Ok (-5 - 10v)
  int16_t resonatorCv = getParameterValue(RESONATOR_HARMONY_CV); // Ok (-5 - 10v)
  int16_t speedCv = getParameterValue(LOOPER_SPEED_CV); // Ok (-5 - 10v)
  int16_t reverbCv = getParameterValue(REVERB_TONESIZE_CV); // Ok (-5 - 10v)
  int16_t vOctCv = getParameterValue(OSC_VOCT_CV); // Ok (0 - 10v)

  int16_t looperVol = getParameterValue(LOOPER_VOL);
  int16_t reverbVol = getParameterValue(REVERB_VOL);

  int16_t delayVol = getParameterValue(DELAY_VOL);
  int16_t resoVol = getParameterValue(RESONATOR_VOL);
  int16_t filterVol = getParameterValue(FILTER_VOL);
  int16_t inVol = getParameterValue(IN_VOL);
  int16_t sswtVol = getParameterValue(SSWT_VOL);
  int16_t sineVol = getParameterValue(SINE_VOL);

  int16_t speed = getParameterValue(LOOPER_SPEED);
  int16_t resoD = getParameterValue(FILTER_RESODRIVE);
  int16_t detune = getParameterValue(OSC_DETUNE);
  int16_t length = getParameterValue(LOOPER_LENGTH);
  int16_t pitch = getParameterValue(OSC_PITCH);
  int16_t start = getParameterValue(LOOPER_START);
  int16_t resoHarmony = getParameterValue(RESONATOR_HARMONY);
  int16_t resoDecay = getParameterValue(RESONATOR_DECAY);
  int16_t toneSize = getParameterValue(REVERB_TONESIZE);
  int16_t decay = getParameterValue(REVERB_DECAY);
  int16_t cutoff = getParameterValue(FILTER_CUTOFF);

  int16_t delayF = getParameterValue(DELAY_FEEDBACK);
  int16_t delayA = getParameterValue(DELAY_TIME);
  int16_t randomMode = getParameterValue(RANDOM_MODE);

  int16_t modAmount = getParameterValue(MOD_LEVEL);
  int16_t modFreq = getParameterValue(MOD_FREQ);

  #endif
}

void onChangePin(uint16_t pin)
{
  switch (pin)
  {
    case SYNC_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(SYNC_GATE_GPIO_Port, SYNC_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(SYNC_GATE, state);
      break;
    }
    case RECORD_BUTTON_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_BUTTON_GPIO_Port, RECORD_BUTTON_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_BUTTON, state);
      if (CONFIG_MODE_OPTIONS == configMode && state)
      {
        configuration.mod_attenuverters = !configuration.mod_attenuverters;
      }
      break;
    }
    case RECORD_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RECORD_GATE_GPIO_Port, RECORD_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RECORD_GATE, state);
      break;
    }
    case RANDOM_GATE_Pin:
    {
      bool state = HAL_GPIO_ReadPin(RANDOM_GATE_GPIO_Port, RANDOM_GATE_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(RANDOM_GATE, state);
      break;
    }
    case PREPOST_SWITCH_Pin:
    {
      bool state = HAL_GPIO_ReadPin(PREPOST_SWITCH_GPIO_Port, PREPOST_SWITCH_Pin) == GPIO_PIN_RESET; // Inverted
      setButtonValue(PREPOST_SWITCH, state);
      break;
    }
  }
}

void setAnalogValue(uint8_t ch, int16_t value)
{
  extern DAC_HandleTypeDef DAC_HANDLE;
  switch (ch)
  {
  case INLEVELGREEN_LED:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_1, DAC_ALIGN_12B_R, __USAT(value, 12));
    break;
  case MOD_LED:
    HAL_DAC_SetValue(&DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 4095 - __USAT(value, 12)); // Inverted
    break;
  }
}

void setGateValue(uint8_t ch, int16_t value)
{
  switch (ch)
  {
  case RECORD_BUTTON:
    setLed(RECORD_LED, value);
    break;
  case RANDOM_BUTTON:
    setLed(RANDOM_LED, value);
    break;
  case SYNC_GATE:
    setLed(SYNC_LED, value);
    break;
  case INLEVELRED:
    setLed(INLEVELRED_LED, value);
    break;
  case CU_DOWN:
    setLed(CU_DOWN_LED, value);
    break;
  case CU_UP:
    setLed(CU_UP_LED, value);
    break;
  case SHIFT_BUTTON:
    setLed(SHIFT_LED, value);
    break;
  case MOD_CV_GREEN:
    setLed(MOD_CV_GREEN_LED, value);
    break;
  case MOD_CV_RED:
    setLed(MOD_CV_RED_LED, value);
    break;
  }
}

void setLed(uint8_t led, uint32_t rgb)
{
  switch (led)
  {
  case RECORD_LED:
    HAL_GPIO_WritePin(RECORD_BUTTON_LED_GPIO_Port, RECORD_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case RANDOM_LED:
    HAL_GPIO_WritePin(RANDOM_BUTTON_LED_GPIO_Port, RANDOM_BUTTON_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case SYNC_LED:
    HAL_GPIO_WritePin(SYNC_LED_GPIO_Port, SYNC_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case INLEVELRED_LED:
    HAL_GPIO_WritePin(INLEVELRED_LED_GPIO_Port, INLEVELRED_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case CU_DOWN_LED:
    HAL_GPIO_WritePin(CU_DOWN_LED_GPIO_Port, CU_DOWN_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case CU_UP_LED:
    HAL_GPIO_WritePin(CU_UP_LED_GPIO_Port, CU_UP_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_SET : GPIO_PIN_RESET); // Inverted
    break;
  case SHIFT_LED:
    HAL_GPIO_WritePin(SHIFT_LED_GPIO_Port, SHIFT_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case MOD_CV_GREEN_LED:
    HAL_GPIO_WritePin(MOD_CV_GREEN_LED_GPIO_Port, MOD_CV_GREEN_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  case MOD_CV_RED_LED:
    HAL_GPIO_WritePin(MOD_CV_RED_LED_GPIO_Port, MOD_CV_RED_LED_Pin, rgb == NO_COLOUR ? GPIO_PIN_RESET : GPIO_PIN_SET);
    break;
  }
}

void ledsOn()
{
  setLed(RECORD_LED, 1);
  setLed(RANDOM_LED, 1);
  setLed(SYNC_LED, 1);
  setLed(INLEVELRED_LED, 1);
  setLed(CU_DOWN_LED, 1);
  setLed(CU_UP_LED, 1);
  setLed(SHIFT_LED, 1);
  setLed(MOD_CV_RED_LED, 1);
  setAnalogValue(MOD_LED, 4095);
}

void ledsOff()
{
  setLed(RECORD_LED, NO_COLOUR);
  setLed(RANDOM_LED, NO_COLOUR);
  setLed(SYNC_LED, NO_COLOUR);
  setLed(INLEVELRED_LED, NO_COLOUR);
  setLed(CU_DOWN_LED, NO_COLOUR);
  setLed(CU_UP_LED, NO_COLOUR);
  setLed(SHIFT_LED, NO_COLOUR);
  setLed(MOD_CV_GREEN_LED, NO_COLOUR);
  setLed(MOD_CV_RED_LED, NO_COLOUR);
  setAnalogValue(MOD_LED, NO_COLOUR);
}

void updateParameters(int16_t* parameter_values, size_t parameter_len, uint16_t* adc_values, size_t adc_len)
{
  uint8_t value = (randomAmountSwitch2.get() << 1) | randomAmountSwitch1.get();
  parameter_values[RANDOM_AMOUNT] = 2047 * (value - 1); // Mid = 0, Low = 2047, High = 4094
}

void onSetup()
{
  extern ADC_HandleTypeDef MUX_PERIPH;
  if (HAL_ADC_Start_DMA(&MUX_PERIPH, (uint32_t *)mux_values, NOF_MUX_VALUES) != HAL_OK)
  {
    error(CONFIG_ERROR, "ADC1 Start failed");
  }

  onChangePin(SYNC_GATE_Pin);
  onChangePin(RECORD_BUTTON_Pin);
  onChangePin(RECORD_GATE_Pin);
  onChangePin(RANDOM_GATE_Pin);
  onChangePin(PREPOST_SWITCH_Pin);

  loadConfiguration();
}

void onLoop(void)
{
  static uint32_t counter = PATCH_RESET_COUNTER;

  bool shiftButtonPressed = HAL_GPIO_ReadPin(SHIFT_BUTTON_GPIO_Port, SHIFT_BUTTON_Pin) == GPIO_PIN_RESET;
  bool modCvButtonPressed = HAL_GPIO_ReadPin(MOD_CV_BUTTON_GPIO_Port, MOD_CV_BUTTON_Pin) == GPIO_PIN_RESET;
  bool recButtonPressed = HAL_GPIO_ReadPin(RECORD_BUTTON_GPIO_Port, RECORD_BUTTON_Pin) == GPIO_PIN_RESET;
  bool rndButtonPressed = HAL_GPIO_ReadPin(RANDOM_BUTTON_GPIO_Port, RANDOM_BUTTON_Pin) == GPIO_PIN_RESET;
  static bool buttonPressed = false;

  if (RUN_MODE != owl.getOperationMode())
  {
    if (shiftButtonPressed && modCvButtonPressed)
    {
      owl.setOperationMode(CONFIGURE_MODE);
      configMode = CONFIG_MODE_CALIBRATION;
      buttonPressed = true;
    }
    else if (recButtonPressed && rndButtonPressed)
    {
      owl.setOperationMode(CONFIGURE_MODE);
      configMode = CONFIG_MODE_OPTIONS;
      buttonPressed = true;
    }
  }

  switch (owl.getOperationMode())
  {
  case STARTUP_MODE:
    if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case LOAD_MODE:
    ledsOff();
    if (--counter == 0)
    {
      counter = PATCH_RESET_COUNTER;
      owl.setOperationMode(RUN_MODE);
    }
    else if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;
  case RUN_MODE:
    readGpio();
    if (getErrorStatus() != NO_ERROR)
    {
      owl.setOperationMode(ERROR_MODE);
    }
    break;

  case CONFIGURE_MODE:
    if (CONFIG_MODE_NONE != configMode)
    {
      if (buttonPressed && !recButtonPressed && !rndButtonPressed && !shiftButtonPressed && !modCvButtonPressed)
      {
        buttonPressed = false;
      }
      if (!buttonPressed)
      {
        if (CONFIG_MODE_CALIBRATION == configMode)
        {

          // Buttons have been released.
          if (CALIBRATION_NONE == calibrationStep)
          {
            // Enter C1 calibration.
            calibrationStep = CALIBRATION_C1;
            setLed(RECORD_LED, 1);
          }
          else if (CALIBRATION_C1 == calibrationStep)
          {
            if (recButtonPressed)
            {
              // Enter C3 calibration.
              calibrationStep = CALIBRATION_C3;
              setLed(RECORD_LED, 0);
              setLed(RANDOM_LED, 1);
            }
          }
          else if (CALIBRATION_C3 == calibrationStep)
          {
            if (rndButtonPressed)
            {
              // Enter params calibration.
              calibrationStep = CALIBRATION_PARAMS;
              setLed(RANDOM_LED, 0);
              setLed(MOD_CV_GREEN_LED, 1);
            }
          }
          else if (CALIBRATION_PARAMS == calibrationStep)
          {
            if (modCvButtonPressed)
            {
              setLed(MOD_CV_GREEN_LED, 0);
              // Save and exit calibration.
              float scalar = 24 / (c3 - c1);
              float offset = 12 - scalar * c1;
              configuration.voct_offset = offset * UINT16_MAX;
              configuration.voct_scale = scalar * UINT16_MAX;
              saveConfiguration();
              configMode = CONFIG_MODE_NONE;
              calibrationStep = CALIBRATION_NONE;
              owl.setOperationMode(LOAD_MODE);
            }
          }
        }
        else if (CONFIG_MODE_OPTIONS)
        {
          readGpio();
          setLed(SHIFT_LED, configuration.soft_takeover);
          setLed(RECORD_LED, configuration.mod_attenuverters);
          setLed(RANDOM_LED, configuration.cv_attenuverters);
          setLed(MOD_CV_GREEN_LED, 1);

          if (modCvButtonPressed)
          {
            ledsOff();
            // Save and exit calibration.
            saveConfiguration();
            configMode = CONFIG_MODE_NONE;
            owl.setOperationMode(LOAD_MODE);
          }
        }
      }
    }
    else
    {
      // Config button has not been pressed during startup, go to load mode.
      owl.setOperationMode(LOAD_MODE);
    }
    break;

  case ERROR_MODE:
    ledsOn();
    if (shiftButtonPressed)
    {
      if (--counter == 0)
      {
        // Reset device after error.
        ledsOff();
        setErrorStatus(NO_ERROR);
        owl.setOperationMode(STARTUP_MODE);
        program.resetProgram(false);
      }
    }
    else
    {
      const char* message = getErrorMessage();
      counter = PATCH_RESET_COUNTER;
    }
    break;
  }
}

#define MAX_PATCH_SETTINGS 16
#define PATCH_SETTINGS_NAME "oneiroi"

int16_t data[MAX_PATCH_SETTINGS] = {};

bool onMidiSend(uint8_t port, uint8_t status, uint8_t d1, uint8_t d2)
{
  MidiMessage msg(port, status, d1, d2);
  uint8_t fileIndex = 0;

  if (msg.isPitchBend())
  {
    int ch = msg.getChannel();
    if(ch < MAX_PATCH_SETTINGS)
    {
      data[ch] = msg.getPitchBend();
    }
  }
  else if (msg.data[1] == START)
  {
    // clear settings
    memset(data, 0, sizeof(data));

    return false; // suppress this message
  }
  else if (msg.isChannelPressure())
  {
    fileIndex = msg.getChannelPressure();

    return false; // suppress this message
  }
  else if (msg.data[1] == STOP)
  {
    const char* filename;
    switch (fileIndex)
    {
    case 0:
      filename = PATCH_SETTINGS_NAME ".alt";
      break;
    case 1:
      filename = PATCH_SETTINGS_NAME ".mod";
      break;
    case 2:
      filename = PATCH_SETTINGS_NAME ".cv";
      break;
    }

    uint32_t headerSize = sizeof(ResourceHeader);
    uint32_t dataSize = sizeof(data);
    uint8_t buffer[headerSize + dataSize];
    memset(buffer, 0, headerSize);
    memcpy(buffer + headerSize, data, dataSize);
    taskENTER_CRITICAL();
    storage.writeResource(filename, buffer, dataSize, FLASH_DEFAULT_FLAGS);
    taskEXIT_CRITICAL();

    return false; // suppress this message
  }

  return true;
}
