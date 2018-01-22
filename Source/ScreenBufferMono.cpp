#include "ScreenBuffer.h"
#include <string.h>
#include <stddef.h>
#include "font.c"

Colour ScreenBuffer::getPixel(unsigned int x, unsigned int y){
  if(x >= width || y >= height)
    return 0;
  uint8_t  ucByteOffset = 0;
  uint16_t usiArrayLoc = 0;
  // Determine array location
  usiArrayLoc = (y/8)+(x*8);
  // Determine byte offset
  ucByteOffset = y-((uint8_t)(y/8)*8);
  // Return bit state from buffer
  return pixels[usiArrayLoc] & (1 << ucByteOffset);
  // return pixels[y*width+x];
}

void ScreenBuffer::setPixel(unsigned int x, unsigned int y, Colour c){
  if(x < width && y < height){
    uint8_t  ucByteOffset = 0;
    uint16_t usiArrayLoc = 0;
    // Determine array location
    usiArrayLoc = (y/8)+(x*8);
    // Determine byte offset
    ucByteOffset = y-((uint8_t)(y/8)*8);		
    // Set pixel in buffer
    pixels[usiArrayLoc] |= (1 << ucByteOffset);
    // pixels[y*width+x] = c;
  }
}

void ScreenBuffer::invertPixel(unsigned int x, unsigned int y){
  if(x < width && y < height){
    uint8_t  ucByteOffset = 0;
    uint16_t usiArrayLoc = 0;
    // Determine array location
    usiArrayLoc = (y/8)+(x*8);
    // Determine byte offset
    ucByteOffset = y-((uint8_t)(y/8)*8);
    uint8_t pixel = (1 << ucByteOffset);
    // Set pixel in buffer
    if(pixels[usiArrayLoc] & pixel)
      pixels[usiArrayLoc] &= ~pixel;
    else
      pixels[usiArrayLoc] |= pixel;
  }
}

void ScreenBuffer::fade(uint16_t steps){
  // for(unsigned int i=0; i<height*width; ++i)
  //   pixels[i] = 
  //     (((pixels[i] & RED) >> steps) & RED) | 
  //     (((pixels[i] & GREEN) >> steps) & GREEN) |
  //     (((pixels[i] & BLUE) >> steps) & BLUE);
  // todo!
}

void ScreenBuffer::fill(Colour c) {
  memset(pixels, c, height*width/8); // todo: height*width/8
  // for(unsigned int i=0; i<height*width; ++i)
  //   pixels[i] = c;
}

void ScreenBuffer::write(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    if(wrap && (cursor_x > (width - textsize*6))){
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }
}
