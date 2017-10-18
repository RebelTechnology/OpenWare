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
  for(unsigned int i=0; i<height*width; ++i)
    pixels[i] = 
      (((pixels[i] & RED) >> steps) & RED) | 
      (((pixels[i] & GREEN) >> steps) & GREEN) |
      (((pixels[i] & BLUE) >> steps) & BLUE);
}

void ScreenBuffer::fill(Colour c) {
  for(unsigned int i=0; i<height*width; ++i)
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
    // if (wrap && (cursor_x > (width - textsize*6))) {
    //   cursor_y += textsize*8;
    //   cursor_x = 0;
    // }
  }
}

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// Draw a character
void ScreenBuffer::drawChar(uint16_t x, uint16_t y, unsigned char c,
			    uint16_t color, uint16_t bg, uint8_t size) {
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
      line = pgm_read_byte(font+(c*5)+i);
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
				   uint16_t color, uint16_t bg, uint8_t size) {
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
      line = pgm_read_byte(font+(c*5)+i);
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
