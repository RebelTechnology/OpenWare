#include "ScreenBuffer.h"
#include <string.h>
#include <stddef.h>
#include "font.c"
#include "message.h"

#define swap(a, b) { int16_t t = a; a = b; b = t; }
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif

ScreenBuffer::ScreenBuffer(uint16_t w, uint16_t h) : 
  width(w), height(h), pixels(NULL),
  cursor_x(0), cursor_y(0), textsize(1),
  textcolor(WHITE), textbgcolor(WHITE), wrap(true) {}

void ScreenBuffer::print(const char* str) {
  unsigned int len = strnlen(str, 256);
  for(unsigned int i=0; i<len; ++i)
    write(str[i]);
}

void ScreenBuffer::print(float num) {
  print(msg_ftoa(num, 10));
}

void ScreenBuffer::print(int num) {
  print(msg_itoa(num, 10));
}

void ScreenBuffer::print(int x, int y, const char* text){
  setCursor(x, y);
  print(text);
}

void ScreenBuffer::drawVerticalLine(int x, int y,
				    int length, Colour c){
  // drawLine(x, y, x, y+length-1, c);
  length += y;
  while(y < length)
    setPixel(x, y++, c);
}

void ScreenBuffer::drawHorizontalLine(int x, int y,
				      int length, Colour c){
  // drawLine(x, y, x+length-1, y, c);
  length += x;
  while(x < length)
    setPixel(x++, y, c);
}

void ScreenBuffer::fillRectangle(int x, int y, int w, int h,
				 Colour c) {
  // for(int i=x; i<x+w; i++)
  //   drawVerticalLine(i, y, h, c);
  for(int i=y; i<y+h; i++)
    drawHorizontalLine(x, i, w, c);
}

void ScreenBuffer::drawRectangle(int x, int y, int w, int h,
				 Colour c) {
  drawHorizontalLine(x+1, y, w-2, c);
  drawHorizontalLine(x+1, y+h-1, w-2, c);
  drawVerticalLine(x, y, h, c);
  drawVerticalLine(x+w-1, y, h, c);
}

// Bresenham's algorithm - thx wikpedia
void ScreenBuffer::drawLine(int x0, int y0,
			    int x1, int y1,
			    Colour c) {
  int steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }
  int dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);
  int err = dx / 2;
  int ystep;
  if (y0 < y1)
    ystep = 1;
  else
    ystep = -1;
  for (; x0<=x1; x0++) {
    if (steep)
      setPixel(y0, x0, c);
    else
      setPixel(x0, y0, c);
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void ScreenBuffer::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void ScreenBuffer::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void ScreenBuffer::setTextColour(Colour c) {
  // For 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

void ScreenBuffer::setTextColour(Colour c, Colour b) {
  textcolor   = c;
  textbgcolor = b; 
}

void ScreenBuffer::setTextWrap(bool w) {
  wrap = w;
}

// Draw a character
void ScreenBuffer::drawChar(uint16_t x, uint16_t y, unsigned char ch,
                            Colour c, Colour bg, uint8_t size) {
  // if((x >= width)            || // Clip right
  //    (y >= height)           || // Clip bottom
  //    ((x + 6 * size - 1) < 0) || // Clip left
  //    ((y + 8 * size - 1) < 0))   // Clip top
  //   return;
  y -= 8 * size; // set origin to bottom left
  for(int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(ch*5)+i];
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          setPixel(x+i, y+j, c);
        else {  // big size
          fillRectangle(x+(i*size), y+(j*size), size, size, c);
        } 
      } else if (bg != c) {
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
void ScreenBuffer::drawRotatedChar(uint16_t x, uint16_t y, unsigned char ch,
                                   Colour c, Colour bg, uint8_t size) {
  // if((x >= width)            || // Clip right
  //    (y >= height)           || // Clip bottom
  //    ((x + 8 * size - 1) < 0) || // Clip left
  //    ((y + 6 * size - 1) < 0))   // Clip top
  //   return;
  // for (int8_t i=5; i>=0; i-- ) {
  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5)
      line = 0x0;
    else 
      line = font[(ch*5)+i];
    // for (int8_t j = 0; j<8; j++) {
    for (int8_t j = 7; j>=0; j--) {
      if (line & 0x1) {
        if (size == 1) // default size
          setPixel(y+i, x+j, c);
        else {  // big size
          // fillRectangle(x+(i*size), y+(j*size), size, size, c);
          fillRectangle(y+(j*size), x+(i*size), size, size, c);
        } 
      } else if (bg != c) {
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

void ScreenBuffer::invert(){
  invert(0, 0, width, height);
}

void ScreenBuffer::invert(int x, int y, int w, int h){
  for(int i=x; i<x+w; ++i)
    for(int j=y; j<y+h; ++j)
      invertPixel(i, j);
}
