#include "ScreenBuffer.h"
#include <string.h>
#include <stddef.h>
#include "font.c"

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

  // ScreenBuffer(int w, int h, Colour* buffer) : width(w), height(h), pixels(buffer){}
ScreenBuffer::ScreenBuffer(int w, int h) : width(w), height(h), pixels(NULL){
  cursor_y  = cursor_x    = 0;
  textsize  = 1;
  textcolor = textbgcolor = 0xFFFF;
}

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

void ScreenBuffer::print(const char* str) {
  unsigned int len = strnlen(str, 256);
  for(unsigned int i=0; i<len; ++i)
    write(str[i]);
}

const char hexnumerals[] = "0123456789abcdef";

char* itoa(int val, int base){
  static char buf[13] = {0};
  int i = 11;
  unsigned int part = abs(val);
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(part && i);
  if(val < 0)
    buf[i--] = '-';
  return &buf[i+1];
}

char* ftoa(float val, int base){
  static char buf[16] = {0};
  int i = 14;
  // print 4 decimal points
  unsigned int part = abs((int)((val-int(val))*10000));
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(i>10);
  buf[i--] = '.';
  part = abs(int(val));
  do{
    buf[i--] = hexnumerals[part % base];
    part /= base;
  }while(part && i);
  if(val < 0.0f)
    buf[i--] = '-';
  return &buf[i+1];
}

void ScreenBuffer::print(float num) {
  print(ftoa(num, 10));
}

void ScreenBuffer::print(int num) {
  print(itoa(num, 10));
}

void ScreenBuffer::print(int x, int y, const char* text){
  setCursor(x, y);
  print(text);
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

// void ScreenBuffer::setTextWrap(bool w) {
//   wrap = w;
// }


// Draw a rectangle
void ScreenBuffer::drawRectangle(int x, int y,
				 int w, int h,
				 Colour color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void ScreenBuffer::drawFastVLine(int x, int y,
				 int h, Colour color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void ScreenBuffer::drawFastHLine(int x, int y,
				 int w, Colour color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

void ScreenBuffer::fillRectangle(int x, int y, int w, int h,
			    Colour color) {
  // Update in subclasses if desired!
  for (int i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

// Bresenham's algorithm - thx wikpedia
void ScreenBuffer::drawLine(int x0, int y0,
			    int x1, int y1,
			    Colour color) {
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
      setPixel(y0, x0, color);
    else
      setPixel(x0, y0, color);
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}
