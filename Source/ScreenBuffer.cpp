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

void ScreenBuffer::drawCircle(uint16_t x, uint16_t y, uint16_t r, Colour c){
    /*
     * Bresenhams midpoint circle algorithm AKA "Make turbo C great again!"
     * 
     * We don't use floating point or any slow maths to find circle points.
     * But since we draw it around center, circles with even radius become
     * slightly asymmetric.
     */
    int16_t tx = r;
    int16_t ty = 0;
    int16_t err = 0;
    
    while (tx >= ty) {
        setPixel(x + tx, y + ty, c); // p1
        setPixel(x + ty, y + tx, c); // p2
        setPixel(x - ty, y + tx, c); // p3
        setPixel(x - tx, y + ty, c); // p4
        setPixel(x - tx, y - ty, c); // p5
        setPixel(x - ty, y - tx, c); // p6
        setPixel(x + ty, y - tx, c); // p7
        setPixel(x + tx, y - ty, c); // p8
 
        if (err <= 0){
            ty += 1;
            err += 2 * ty + 1;
        }
 
        if (err >= 0){
            tx -= 1;
            err -= 2 * tx + 1;
        }
    }
}

void ScreenBuffer::fillCircle(uint16_t x, uint16_t y, uint16_t r, Colour c){
    /*
     * This is based of code from drawCircle, but we connect circle's points
     * with horizontal lines
     */
    int16_t tx = r;
    int16_t ty = 0;
    int16_t err = 0;
    
    while (tx >= ty) {
        drawHorizontalLine(x - tx, y + ty, tx * 2 + 1, c); // p4 -> p1
        drawHorizontalLine(x - ty, y + tx, ty * 2 + 1, c); // p3 -> p2
        drawHorizontalLine(x - tx, y - ty, tx * 2 + 1, c); // p5 -> p8
        drawHorizontalLine(x - ty, y - tx, ty * 2 + 1, c); // p6 -> p7
 
        if (err <= 0){
            ty += 1;
            err += 2 * ty + 1;
        }
 
        if (err >= 0){
            tx -= 1;
            err -= 2 * tx + 1;
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
