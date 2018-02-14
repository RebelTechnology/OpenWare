#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* gcc -std=c99 TestTLC.c -o TestTLC && ./TestTLC */
  
uint8_t rgGSbuf[3][24];
uint8_t rgDCbuf[3][12];

void SetOutput_GS(uint8_t IC, uint8_t LED_ID, uint16_t value){
#if 1
  uint8_t temp;
  uint8_t ucBuffLoc = LED_ID + (LED_ID>>1); // (uint8_t)(LED_ID*1.5);
  if(LED_ID & 0x01)	// bbbbaaaa aaaaaaaa
    {
      temp			= rgGSbuf[IC][ucBuffLoc];
      rgGSbuf[IC][ucBuffLoc] 	= (value&0xF00)>>8;
      rgGSbuf[IC][ucBuffLoc]   |= (temp&0xF0);
      rgGSbuf[IC][ucBuffLoc+1]  = (value&0x0FF);
    }
  else            	// aaaaaaaa aaaabbbb
    {
      rgGSbuf[IC][ucBuffLoc] 	= (value&0xFF0)>>4;
      temp 			= rgGSbuf[IC][ucBuffLoc+1];
      rgGSbuf[IC][ucBuffLoc+1]  = (value&0x00F)<<4;
      rgGSbuf[IC][ucBuffLoc+1] |= (temp&0x0F);
    }
#elif 1
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/8;
  uint32_t bit = bitshift % 8;
  uint8_t* data = (uint8_t*)(rgGSbuf[IC]);
  data[word] = (data[word] & ~(0xfffu >> (8-bit))) | (value & 0xfffu) >> (8-bit);
  data[word+1] = (data[word+1] & ~(0xfffu << bit)) | ((value & 0xfffu) << bit);
#else
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/32;
  uint32_t bit = bitshift % 32;
  uint32_t* data = (uint32_t*)(rgGSbuf[IC]);
  data[word] = (data[word] & ~(0xfffu << bit)) | ((value & 0xfffu) << bit);
  if(bit > 20)
    data[word+1] = (data[word+1] & ~(0xfffu >> (32-bit))) | (value & 0xfffu) >> (32-bit);
#endif
}

uint16_t GetOutput_GS(uint8_t IC, uint8_t LED_ID){
#if 0
  uint8_t ucBuffLoc = LED_ID + (LED_ID>>1); // (uint8_t)(LED_ID*1.5);
  uint16_t val;	
  if(LED_ID & 0x01)	// bbbbaaaa aaaaaaaa
    {
      val = (rgGSbuf[IC][ucBuffLoc]<<8) & 0xf00;
      val |= rgGSbuf[IC][ucBuffLoc+1] & 0x0ff;
    }
  else            	// aaaaaaaa aaaabbbb
    {
      val = rgGSbuf[IC][ucBuffLoc]<<4 & 0xff0;
      val |= (rgGSbuf[IC][ucBuffLoc+1]>>4) & 0x00f;
    }
  return val;
#elif 1
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/8;
  uint32_t bit = bitshift % 8;
  uint8_t* data = (uint8_t*)(rgGSbuf[IC]);
  uint16_t val = (data[word+1] >> bit) & (0xfffu >> bit);  // should be & 0x0ff or & 0x00f
  val |= (data[word] << (8-bit)) & ((0xfffu >> (8-bit))<<(8-bit)); // should be & 0xf00 or & 0xff0
  return val;
#else
  uint32_t bitshift = LED_ID*12;
  uint32_t word = bitshift/32;
  uint32_t bit = bitshift % 32;
  uint32_t* data = (uint32_t*)(rgGSbuf[IC]);
  uint16_t val = (data[word] >> bit) & 0xfffu;
  if(bit > 20)
    val |= (data[word+1] << (32-bit)) & 0xfffu;
  return val;
#endif
}

int main(int argc, char** argv) {
    int tx, rx;
    printf("tx \t\t rx \t\t delta\n");
    for(int i=0; i<16; ++i){
      tx = 4095-i*255;
      SetOutput_GS(0, i, tx);
      rx = GetOutput_GS(0, i);
      printf("%d \t\t %d \t\t %d\n", tx, rx, tx-rx);
    }
    for(int i=0; i<16; ++i){
      tx = 4095-i*255;
      rx = GetOutput_GS(0, i);
      printf("%d \t\t %d \t\t %d\n", tx, rx, tx-rx);
    }    
}
