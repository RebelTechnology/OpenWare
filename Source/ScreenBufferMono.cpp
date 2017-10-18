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

void ScreenBuffer::fade(uint16_t steps){
  // for(unsigned int i=0; i<height*width; ++i)
  //   pixels[i] = 
  //     (((pixels[i] & RED) >> steps) & RED) | 
  //     (((pixels[i] & GREEN) >> steps) & GREEN) |
  //     (((pixels[i] & BLUE) >> steps) & BLUE);
  // todo!
}

void ScreenBuffer::fill(Colour c) {
  memset(pixels, c, 1024); // todo: height*width/8
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
    // if (wrap && (cursor_x > (width - textsize*6))) {
    //   cursor_y += textsize*8;
    //   cursor_x = 0;
    // }
  }
}

// Draw a character
void ScreenBuffer::drawChar(uint16_t x, uint16_t y, unsigned char c,
                            Colour color, Colour bg, uint8_t size) {
  if((x >= width)            || // Clip right
     (y >= height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;
  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          setPixel(x+i, y+j, color);
        else {  // big size
          fillRectangle(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          setPixel(x+i, y+j, bg);
        else {  // big size
          fillRectangle(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

// Draw a character rotated 90 degrees
void ScreenBuffer::drawRotatedChar(uint16_t x, uint16_t y, unsigned char c,
                                   Colour color, Colour bg, uint8_t size) {
  if((x >= width)            || // Clip right
     (y >= height)           || // Clip bottom
     ((x + 8 * size - 1) < 0) || // Clip left
     ((y + 6 * size - 1) < 0))   // Clip top
    return;
  // for (int8_t i=5; i>=0; i-- ) {
  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    // for (int8_t j = 0; j<8; j++) {
    for (int8_t j = 7; j>=0; j--) {
      if (line & 0x1) {
        if (size == 1) // default size
          setPixel(y+i, x+j, color);
        else {  // big size
          // fillRectangle(x+(i*size), y+(j*size), size, size, color);
          fillRectangle(y+(j*size), x+(i*size), size, size, color);
        } 
      } else if (bg != color) {
        if (size == 1) // default size
          setPixel(y+i, x+j, bg);
        else {  // big size
          // fillRectangle(x+i*size, y+j*size, size, size, bg);
          fillRectangle(y+j*size, x+i*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

