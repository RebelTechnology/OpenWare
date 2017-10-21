#include "ScreenBuffer.h"
#include <string.h>
#include <stddef.h>
#include "font.c"

Colour ScreenBuffer::getPixel(unsigned int x, unsigned int y){
  if(x >= width || y >= height)
    return 0;
  return pixels[y*width+x];
}

void ScreenBuffer::setPixel(unsigned int x, unsigned int y, Colour c){
  if(x < width && y < height)
    pixels[y*width+x] = c;
}

void ScreenBuffer::fade(uint16_t steps){
  for(int i=0; i<height*width; ++i)
    pixels[i] = 
      (((pixels[i] & RED) >> steps) & RED) | 
      (((pixels[i] & GREEN) >> steps) & GREEN) |
      (((pixels[i] & BLUE) >> steps) & BLUE);
}

void ScreenBuffer::fill(Colour c) {
  for(int i=0; i<height*width; ++i)
    pixels[i] = c;
}

void ScreenBuffer::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawRotatedChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    if(wrap && (cursor_x > (width - textsize*6))){
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }
}
