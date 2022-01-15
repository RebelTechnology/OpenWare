#include <stdint.h>
#include "device.h"
#include "oled.h"
#include "gpio.h"
#include "message.h"

#define SEPS114A_SOFT_RESET 0x01
#define SEPS114A_DISPLAY_ON_OFF 0x02
#define SEPS114A_ANALOG_CONTROL 0x0F    //
#define SEPS114A_STANDBY_ON_OFF 0x14
#define SEPS114A_OSC_ADJUST 0x1A
#define SEPS114A_ROW_SCAN_DIRECTION 0x09
#define SEPS114A_DISPLAY_X1 0x30
#define SEPS114A_DISPLAY_X2 0x31
#define SEPS114A_DISPLAY_Y1 0x32
#define SEPS114A_DISPLAY_Y2 0x33
#define SEPS114A_DISPLAYSTART_X 0x38
#define SEPS114A_DISPLAYSTART_Y 0x39
#define SEPS114A_CPU_IF 0x0D
#define SEPS114A_MEM_X1 0x34
#define SEPS114A_MEM_X2 0x35
#define SEPS114A_MEM_Y1 0x36
#define SEPS114A_MEM_Y2 0x37
#define SEPS114A_MEMORY_WRITE_READ 0x1D
#define SEPS114A_DDRAM_DATA_ACCESS_PORT 0x08
#define SEPS114A_DISCHARGE_TIME 0x18
#define SEPS114A_PEAK_PULSE_DELAY 0x16
#define SEPS114A_PEAK_PULSE_WIDTH_R 0x3A
#define SEPS114A_PEAK_PULSE_WIDTH_G 0x3B
#define SEPS114A_PEAK_PULSE_WIDTH_B 0x3C
#define SEPS114A_PRECHARGE_CURRENT_R 0x3D
#define SEPS114A_PRECHARGE_CURRENT_G 0x3E
#define SEPS114A_PRECHARGE_CURRENT_B 0x3F
#define SEPS114A_COLUMN_CURRENT_R 0x40
#define SEPS114A_COLUMN_CURRENT_G 0x41
#define SEPS114A_COLUMN_CURRENT_B 0x42
#define SEPS114A_ROW_OVERLAP 0x48
#define SEPS114A_SCAN_OFF_LEVEL 0x49
#define SEPS114A_ROW_SCAN_ON_OFF 0x17
#define SEPS114A_ROW_SCAN_MODE 0x13
#define SEPS114A_SCREEN_SAVER_CONTROL 0xD0
#define SEPS114A_SS_SLEEP_TIMER 0xD1
#define SEPS114A_SCREEN_SAVER_MODE 0xD2
#define SEPS114A_SS_UPDATE_TIMER 0xD3
#define SEPS114A_RGB_IF 0xE0
#define SEPS114A_RGB_POL 0xE1
#define SEPS114A_DISPLAY_MODE_CONTROL 0xE5

#define setCS()    setPin(OLED_CS_GPIO_Port, OLED_CS_Pin)
#define clearCS()  clearPin(OLED_CS_GPIO_Port, OLED_CS_Pin)
#define setDC()   setPin(OLED_DC_GPIO_Port, OLED_DC_Pin)
#define clearDC() clearPin(OLED_DC_GPIO_Port, OLED_DC_Pin)
#define clearRST() clearPin(OLED_RST_GPIO_Port, OLED_RST_Pin);
#define setRST() setPin(OLED_RST_GPIO_Port, OLED_RST_Pin);

SPI_HandleTypeDef *hspi;

extern void osDelay(uint32_t millisec);
extern void vPortEnterCritical(void);

#define OLED_TIMEOUT 1000
static void spiwritesync(const uint8_t* data, size_t size){
  while(hspi->State != HAL_SPI_STATE_READY);
  /* clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin); */
  clearCS();
  HAL_StatusTypeDef ret = HAL_SPI_Transmit(hspi, (uint8_t*)data, size, OLED_TIMEOUT);
  assert_param(ret == HAL_OK);
  setCS();
}

static void writeCommand1byte(uint8_t c){
  clearDC();
  spiwritesync(&c, 1);
}

static void writeCommand(uint8_t reg, uint8_t value){
  clearDC();
  spiwritesync(&reg, 1);
  setDC();
  spiwritesync(&value, 1);
}

// Init sequence for 96x96 OLED color module
static void chipInit(){
  writeCommand(SEPS114A_SOFT_RESET,0x00);      
  writeCommand(SEPS114A_STANDBY_ON_OFF,0x01);          // Standby on
  osDelay(5);                                           // Wait for 5ms (1ms Delay Minimum)
  writeCommand(SEPS114A_STANDBY_ON_OFF,0x00);          // Standby off
  osDelay(5);                                           // 1ms Delay Minimum (1ms Delay Minimum)
  /* Display OFF */
  writeCommand(SEPS114A_DISPLAY_ON_OFF,0x00);
  /* Set Oscillator operation */
  writeCommand(SEPS114A_ANALOG_CONTROL,0x00);          // using external resistor and internal OSC
  /* Set frame rate */
  writeCommand(SEPS114A_OSC_ADJUST,0x03);              // frame rate : 95Hz
  /* Set active display area of panel */
  writeCommand(SEPS114A_DISPLAY_X1,0x00);
  writeCommand(SEPS114A_DISPLAY_X2,0x5F);
  writeCommand(SEPS114A_DISPLAY_Y1,0x00);
  writeCommand(SEPS114A_DISPLAY_Y2,0x5F);
  /* Select the RGB data format and set the initial state of RGB interface port */
  writeCommand(SEPS114A_RGB_IF,0x00);                 // RGB 8bit interface
  /* Set RGB polarity */
  writeCommand(SEPS114A_RGB_POL,0x00);
  /* Set display mode control */
  writeCommand(SEPS114A_DISPLAY_MODE_CONTROL, 0x00);  // SWAP:BGR, Reduce current : Normal, DC[1:0] : Normal
  /* Set MCU Interface */
  writeCommand(SEPS114A_CPU_IF,0x00);                 // MPU External interface mode, 8bits
  /* Set Memory Read/Write mode */
#ifdef OWL_RACK
  writeCommand(SEPS114A_MEMORY_WRITE_READ, 0x07);
#else
  writeCommand(SEPS114A_MEMORY_WRITE_READ, 0x02);     // Memory write/read direction
#endif
  /* SEPS114A_MEMORY_WRITE_READ : 0x0: mirrored 0deg,
                                  0x1: normal 0deg (upside down when folded back: Prism Rev03),
                                  0x2: normal 180deg (Prism Rev04)
                                  0x3: mirrored 180deg
                                  0x4: normal, 90deg
                                  0x5: mirrored, 90deg
                                  0x6: mirrored, 270deg
                                  0x7: normal, 270deg (OWL Rack) */  
  /* Set row scan direction */
  writeCommand(SEPS114A_ROW_SCAN_DIRECTION,0x00);     // Column : 0 --> Max, Row : 0 --> Max
  /* SEPS114A_ROW_SCAN_DIRECTION : 0x2 or 0x3: upside down and mirrored */  
  /* Set row scan mode */
  writeCommand(SEPS114A_ROW_SCAN_MODE,0x00);          // Alternate scan mode
  /* Set column current */
  writeCommand(SEPS114A_COLUMN_CURRENT_R,0x6E);
  writeCommand(SEPS114A_COLUMN_CURRENT_G,0x4F);
  writeCommand(SEPS114A_COLUMN_CURRENT_B,0x77);
  /* Set row overlap */
  writeCommand(SEPS114A_ROW_OVERLAP,0x00);            // Band gap only
  /* Set discharge time */
  writeCommand(SEPS114A_DISCHARGE_TIME,0x01);         // Discharge time : normal discharge
  /* Set peak pulse delay */
  writeCommand(SEPS114A_PEAK_PULSE_DELAY,0x00);
  /* Set peak pulse width */
  writeCommand(SEPS114A_PEAK_PULSE_WIDTH_R,0x02);
  writeCommand(SEPS114A_PEAK_PULSE_WIDTH_G,0x02);
  writeCommand(SEPS114A_PEAK_PULSE_WIDTH_B,0x02);
  /* Set precharge current */
  writeCommand(SEPS114A_PRECHARGE_CURRENT_R,0x14);
  writeCommand(SEPS114A_PRECHARGE_CURRENT_G,0x50);
  writeCommand(SEPS114A_PRECHARGE_CURRENT_B,0x19);
  /* Set row scan on/off  */
  writeCommand(SEPS114A_ROW_SCAN_ON_OFF,0x00);        // Normal row scan
  /* Set scan off level */
  writeCommand(SEPS114A_SCAN_OFF_LEVEL,0x04);         // VCC_C*0.75
  /* Set memory access point */
  writeCommand(SEPS114A_DISPLAYSTART_X,0x00);
  writeCommand(SEPS114A_DISPLAYSTART_Y,0x00);

  /* static const uint8_t initSequence[] = { */
  /*   // 0x01, 0x00, 0x14, 0x01, /\* 1ms delay *\/ */
  /*   // 0x14, 0x00, /\* 1ms delay *\/ */
  /*   0x02, 0x00, /\* display off *\/ */
  /*   // 0x0f, 0x40, */
  /*   0x0f, 0x00, // external resistor, internal osc */
  /*   // 0x1a, 0x03, */
  /*   0x1a, 0x0a, // frame rate 95Hz */
  /*   0x30, 0x00, 0x31, 0x5f, 0x32, 0x00, 0x33, 0x5f, // set display x/y 1/2 */
  /*   0xe0, 0x01, // RGB IF 16bit (doesn't matter for SPI) */
  /*   0xe1, 0x00, // RGB polarity */
  /*   0xe5, 0x80, // display mode control */
  /*   0x0d, 0x00, // CPU_IF */
  /*   0x1d, 0x00, // memory write read */
  /*   0x09, 0x00, 0x13, 0x00, */
  /*   0x40, 0x6e, 0x41, 0x4f, 0x42, 0x77, // column current */
  /*   0x48, 0x00, // row overlap */
  /*   0x18, 0x01, // discharge time */
  /*   0x16, 0x00, // peak pulse delay */
  /*   0x3a, 0x02, 0x3b, 0x02, 0x3c, 0x02, // peak pulse width */
  /*   0x3d, 0x14, 0x3e, 0x50, 0x3f, 0x19, // precharge current */
  /*   0x17, 0x00, 0x49, 0x04, /\* clear screen *\/ */
  /*   0x38, 0x00, 0x39, 0x00, /\* zero *\/ */
  /*   0x02, 0x01 /\* display on *\/ */
  /* }; */
  /* /\* static const uint8_t rst[] = {0x01, 0x00, 0x14, 0x01}; /\\* reset, standby on *\\/ *\/ */
  /* /\* writeCommands(rst, sizeof(rst)); *\/ */
  /* /\* osDelay(1); *\/ */
  /* /\* static const uint8_t stdby[] = {0x14, 0x00}; /\\* standby off *\\/ *\/ */
  /* /\* writeCommands(stdby, sizeof(stdby)); *\/ */
  /* /\* osDelay(1); *\/ */
  /* writeCommands(initSequence, sizeof(initSequence)); */
}

static void off() {
  writeCommand(SEPS114A_DISPLAY_ON_OFF, 0x00);
}

static void on() {
  writeCommand(SEPS114A_DISPLAY_ON_OFF, 0x01);
}

static void zero() {
  /* uint8_t cmd[] = {0x34, 0x00, 0x35, 0x5F, 0x36, 0x00, 0x37, 0x5F, 0x08}; */
  /* writeCommands(cmd, sizeof(cmd)); */
  writeCommand(SEPS114A_MEM_X1, 0x00);
  writeCommand(SEPS114A_MEM_X2, 0x5f);
  writeCommand(SEPS114A_MEM_Y1, 0x00);
  writeCommand(SEPS114A_MEM_Y2, 0x5f);
  writeCommand1byte(SEPS114A_DDRAM_DATA_ACCESS_PORT);
}

void oled_init(SPI_HandleTypeDef *spi){
  hspi = spi;
  /* off(); */
  // with clock speed 8MHz a screen update takes 25mS
  // 30MHz: 16mS
  setCS();
  clearRST();
  osDelay(10);
  setRST();
  osDelay(10);
  chipInit();
  osDelay(10);
  on();
  zero();
}

static void spiwrite(const uint8_t* data, size_t size){
#ifdef OLED_BITBANG
  vPortEnterCritical();
  clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
  clearCS();
  for(unsigned int i=0; i<size; ++i){
    char c = data[i];
    for(uint8_t bit = 0x80; bit; bit >>= 1){
      if(c & bit)
	setPin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin);
      else
	clearPin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin);
      setPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
      clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
    }
  }
  setCS();
  vPortExitCritical();
#elif defined OLED_DMA
  // while(!dmaready);
  while(hspi->State != HAL_SPI_STATE_READY);
  /* dmaready = false; */
  /* clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin); */
  clearCS();
  HAL_StatusTypeDef ret = HAL_SPI_Transmit_DMA(hspi, (uint8_t*)data, size);
  assert_param(ret == HAL_OK);
#elif defined OLED_IT
  while(hspi->State != HAL_SPI_STATE_READY);
  clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
  clearCS();
  HAL_StatusTypeDef ret = HAL_SPI_Transmit_IT(hspi, (uint8_t*)data, size);
  assert_param(ret == HAL_OK);
#else
  while(hspi->State != HAL_SPI_STATE_READY);
  clearPin(OLED_SCK_GPIO_Port, OLED_SCK_Pin);
  clearCS();
  HAL_StatusTypeDef ret = HAL_SPI_Transmit(hspi, (uint8_t*)data, size, OLED_TIMEOUT);
  assert_param(ret == HAL_OK);
  setCS();
#endif
}

#if defined OLED_IT || defined OLED_DMA
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
  ASSERT(0, "SPI Error");
}

/* void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){ */
/*   if(__HAL_DMA_GET_FLAG(OLED_SPIInst,  __HAL_DMA_GET_TC_FLAG_INDEX(OLED_SPIInst))){ */
/*     pCS_Set();	// CS high */
/*   } */
/* } */

/* static unsigned int missedcalls = 0; */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
  // todo: find out why this is called when tx is not complete
  // and apparently sometimes not called when it is ready
  setCS();
  /* if(__HAL_DMA_GET_FLAG(hspi,  __HAL_DMA_GET_TC_FLAG_INDEX(hspi))){ */
  /* extern DMA_HandleTypeDef hdma_spi1_tx; */
  /* if(__HAL_DMA_GET_FLAG(&hdma_spi1_tx,  DMA_FLAG_TCIF1_5)){ */
  /*   setCS(); */
  /* }else{ */
  /*   missedcalls++; */
  /* } */
}
#endif

void oled_write(const uint8_t* pixels, uint16_t size){
  setDC();
  spiwrite(pixels, size);
}
