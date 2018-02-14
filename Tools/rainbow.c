#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* 
gcc -std=c99 rainbow.c -lm -o rainbow && ./rainbow > ../Source/rainbow.h
*/

/*
VIBGYOR
#9400D3 Violet
#4B0082 Indigo
#0000FF Blue
#00FF00 Green
#FFFF00 Yellow
#FF7F00 Orange
#FF0000 Red

to 10bit: 
p/x (0x94<<2)|0x3 : 0x253
p/x (0x7f<<2)|0x3 : 0x1fc

#800080 Purple
#FFC0CB	Pink
#FF8C00	DarkOrange
#FFD700	Gold
#FF4500	OrangeRed	
#DC143C	Crimson
#C71585 MediumVioletRed
*/

#define COLOUR_COUNT 3

static uint16_t rainbow[][3] = {
  /* {0x3ff, 0x3ff, 0x3ff}, // white */
  /* {0x253, 0x000, 0x34f}, // violet */
  /* {0x12f, 0x000, 0x20b}, // indigo */
  /* {0x000, 0x000, 0x3ff}, // blue */
  /* {0x000, 0x3ff, 0x000}, // green */
  /* {0x3ff, 0x3ff, 0x000}, // yellow */
  /* {0x3ff, 0x1fc, 0x000}, // orange */
  /* {0x3ff, 0x000, 0x000}, // red */
  /* {0x3ff, 0x3ff, 0x3ff}, // white */

  /* {0x200, 0x000, 0x200}, // Purple */
  /* {0x3ff, 0x300, 0x32f}, // Pink */
  /* {0x31f, 0x057, 0x217}, // MediumVioletRed */

  /* {0x200, 0x000, 0x200}, // Purple */
  /* {0x12f, 0x000, 0x20b}, // Indigo */
  /* {0x253, 0x000, 0x34f}, // Violet */
  /* {0x3ff, 0x300, 0x32f}, // Pink */

  /* {0x370, 0x050, 0x0f0}, // Crimson */
  /* {0x3ff, 0x117, 0x000}, // OrangeRed */
  /* {0x3ff, 0x35f, 0x000}, // Gold */
  /* {0x3ff, 0x230, 0x000}, // DarkOrange */


  // hand tuned with no DC balance (0xaa)
  {857/4, 152/4, 0}, // orange
  {4094/4, 152/4, 0}, // red
  {4094/4, 792/4, 855/4}, // pink

  /*  
  {217/4, 0, 418/4}, // purple
  {0, 0, 1238/4}, // blue
  {1624/4, 2072/4, 4094/4}, // bright cyan
  */

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
  int steps = n/(COLOUR_COUNT-1) - 1;
  int c, i;
  uint32_t colour;
  for(c=0 ; c < (COLOUR_COUNT-1) ; c++){
    const uint16_t* c1 = rainbow[c];
    const uint16_t* c2 = rainbow[c+1];
    float amt1, amt2;
    uint16_t r, g, b;
    for(i=0 ; i <= steps; i++){
      amt1 = (float)(steps-i)/steps;
      amt2 = (float)i/steps;
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
