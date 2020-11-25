#include "stm32f1xx_hal.h"
#include "bus.h"
#include "message.h"
#include "serial.h"
#include "clock.h"
#include "DigitalBusStreamReader.h"
#include "mxconstants.h"
#include "SerialBuffer.hpp"

static DigitalBusStreamReader bus;
static SerialBuffer<512> bus_tx_buf;
bool DIGITAL_BUS_PROPAGATE_MIDI = 0;
bool DIGITAL_BUS_ENABLE_BUS = 1;

extern "C" {
  void serial_rx_callback(uint8_t c);
  uint8_t serial_tx_available();
  uint8_t serial_tx_pull();

#define NO_ERROR            0x00
#define HARDFAULT_ERROR     0x10
#define BUS_ERROR           0x20
#define MEM_ERROR           0x30
#define NMI_ERROR           0x40
#define USAGE_ERROR         0x50
#define PROGRAM_ERROR       0x60
#define CONFIG_ERROR        0x70
#define UART_ERROR          0x80

  void setErrorMessage(int8_t err, const char* msg){
    debug << "Error " << (int)err << ": " << msg << ".";
  }
}

uint8_t* bus_deviceid(){
  return ((uint8_t*)0x1ffff7e8); /* STM32F1 */
  // return ((uint8_t *)0x1FFF7A10); /* STM32F4, STM32F0 */ 
}

void bus_setup(){
  // debug << "bus_setup\r\n";
  extern UART_HandleTypeDef huart1;
  UART_HandleTypeDef *huart = &huart1;
  /* Enable the UART Parity Error Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
  /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
  __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
  /* Enable the UART Data Register not empty Interrupt */
  __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
}

#define BUS_IDLE_INTERVAL 2300

int bus_status(){
  bus.process();
  static uint32_t lastpolled = 0;
  if(getSysTicks() > lastpolled + BUS_IDLE_INTERVAL){
    bus.connected();
    lastpolled = getSysTicks();
  }
  return bus.getStatus();
}

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

void serial_write(uint8_t* data, uint16_t size){
  extern UART_HandleTypeDef huart1;
  UART_HandleTypeDef *huart = &huart1;
  bus_tx_buf.push(data, size);
  __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
}


void bus_tx_parameter(uint8_t pid, int16_t value){
  // debug << "tx parameter [" << pid << "][" << value << "]" ;
  bus.sendParameterChange(pid, value);
}

void bus_tx_command(uint8_t cmd, int16_t data){
  // debug << "tx command [" << cmd << "][" << data << "]" ;
  bus.sendCommand(cmd, data);
}

void bus_tx_message(const char* msg){
  // debug << "tx msg [" << msg << "]" ;
  bus.sendMessage(msg);
}

void bus_tx_error(const char* reason){
  debug << "Digital bus send error: " << reason << ".";
}

void bus_rx_error(const char* reason){
  debug << "Digital bus receive error: " << reason << ".";
  bus.reset();
  bus.sendReset();
}
