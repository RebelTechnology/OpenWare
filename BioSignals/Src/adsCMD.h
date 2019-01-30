/*
 * adsCMD.h
 * Library header file for adsCMD library
 */
/* #include "Arduino.h" */

//For Leonardo SPI see http://openenergymonitor.blogspot.com/2012/06/arduino-leonardo-atmega32u4-and-rfm12b.html
//constants define pins on Arduino 

/* // Arduino Due */
/* const int IPIN_PWDN = 47; //not required for TI demo kit */
/* const int PIN_CLKSEL = 49;//6;//\*optional */
/* const int IPIN_RESET  = 48;//\*optional */

/* const int PIN_START = 46; */
/* const int IPIN_DRDY = 45; */
/* const int IPIN_CS = 52; */
/* //const int PIN_DOUT = 11;//SPI out */
/* //const int PIN_DIN = 12;//SPI in */
/* //const int PIN_SCLK = 13;//SPI clock */

//function prototypes
void adc_wreg(int reg, int val); //write register
void adc_send_command(int cmd); //send command
int adc_rreg(int reg); //read register
