#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* 
gcc -std=c99 rainbow.c -lm -o rainbow && ./rainbow > ../Source/rainbow.h
*/

/*
VIBGYOR
#9400D3
#4B0082
#0000FF
#00FF00
#FFFF00
#FF7F00
#FF0000

to 10bit: 
p/x (0x94<<2)|0x3 : 0x253
p/x (0x7f<<2)|0x3 : 0x1fc
*/
#define COLOUR_COUNT 8

static uint16_t rainbow[9][3] = {
  {0x3ff, 0x3ff, 0x3ff}, // white
  {0x253, 0x000, 0x34f}, // violet
  {0x12f, 0x000, 0x20b}, // indigo
  {0x000, 0x000, 0x3ff}, // blue
  {0x000, 0x3ff, 0x000}, // green
  {0x3ff, 0x3ff, 0x000}, // yellow
  {0x3ff, 0x1fc, 0x000}, // orange
  {0x3ff, 0x000, 0x000}, // red
  {0x3ff, 0x3ff, 0x3ff}, // white
};

uint32_t colourcode(uint32_t r, uint32_t g, uint32_t b){
  return ((r&0x3ff)<<20)|((g&0x3ff)<<10)|(b&0x3ff);
}

uint32_t red(uint32_t colourcode){
  return (colourcode>>20)&0x3ff;
}

uint32_t green(uint32_t colourcode){
  return (colourcode>>10)&0x3ff;
}

uint32_t blue(uint32_t colourcode){
  return (colourcode>>00)&0x3ff;
}

int main(int argc, char** argv) {
  int n = 1024;
  if(argc > 1)
    n = atol(argv[1]);
  printf("/* rainbow table %d values */\n", n );
  printf("const uint32_t rainbow[%d] = { ", n);
  int steps = n/COLOUR_COUNT;
  int c, i;
  uint32_t colour;
  for(c=0 ; c < COLOUR_COUNT ; c++){
    uint16_t* c1 = rainbow[c];
    uint16_t* c2 = rainbow[c+1];
    float amt1, amt2;
    uint16_t r, g, b;
    for(i=0 ; i < steps; i++){
      amt1 = (float)(steps-i-1)/(steps-1);
      amt2 = (float)i/(steps-1);
      r = c1[0]*amt1 + c2[0]*amt2;
      g = c1[1]*amt1 + c2[1]*amt2;
      b = c1[2]*amt1 + c2[2]*amt2;
      colour = colourcode(r, g, b);
      printf("%#08x, ", colour);
    }
  }
  for(i=c*i; i < n; ++i)
    printf("%#08x, ", colour);
  printf(" };\n");
  return 0;

}
