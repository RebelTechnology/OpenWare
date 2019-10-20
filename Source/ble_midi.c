#include "device.h"
#include "ble_midi.h"

#ifndef BLE_SPI
#error missing BLE_SPI
#endif

#define BLE_CS_LO()	HAL_GPIO_WritePin(BLE_NCS_GPIO_Port, BLE_NCS_Pin, GPIO_PIN_RESET);
#define BLE_CS_HI()	HAL_GPIO_WritePin(BLE_NCS_GPIO_Port, BLE_NCS_Pin, GPIO_PIN_SET);
#define BLE_RESET_LO()	HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_RESET);
#define BLE_RESET_HI()	HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_SET);

extern SPI_HandleTypeDef BLE_SPI;

void ble_init(){
  BLE_RESET_LO();
  BLE_CS_HI();
#ifdef USE_BLE_MIDI
  HAL_Delay(100);
  BLE_RESET_HI();
#endif
}

void ble_tx(uint8_t* data, size_t len){
#ifdef USE_BLE_MIDI
  BLE_CS_LO();
  uint8_t rxbuf[len];
  rxbuf[0] = 0;
  HAL_SPI_TransmitReceive(&BLE_SPI, data, rxbuf, len, 100);
  BLE_CS_HI();

  if(rxbuf[0] == 0x20){ // status connected
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
  }else{
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
  }
#endif
}
