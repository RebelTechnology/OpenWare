#ifndef __ScreenBuffer_h__
#define __ScreenBuffer_h__

#include <stdint.h>
#include "device.h"

#if defined SSD1309
typedef uint8_t Colour;
// Color definitions mono
#define	BLACK           0x00
#define WHITE           0x01
#elif defined SSD1331 || defined SEPS114A
typedef uint16_t Colour;
// Color definitions
/* #define BLACK           0x0000 */
/* #define BLUE            0x001F */
/* #define RED             0xF800 */
/* #define GREEN           0x07E0 */
/* #define CYAN            0x07FF */
/* #define MAGENTA         0xF81F */
/* #define YELLOW          0xFFE0   */
/* #define WHITE           0xFFFF */
  
#define	BLACK           0x0000
#define	RED             0xF800
#define	GREEN           0x001F
#define	BLUE            0x07E0
#define YELLOW          0xF81F  // puke
#define CYAN            0x07FF
#define MAGENTA         0xFFE0
#define WHITE           0xFFFF
#elif defined USE_SCREEN
#error "Invalid screen configuration"
#endif

class ScreenBuffer {
private:
  const uint16_t width;
  const uint16_t height;
  Colour* pixels;
  uint16_t cursor_x;
  uint16_t cursor_y;  
  uint8_t textsize;
  uint16_t textcolor;
  uint16_t textbgcolor;
  bool wrap;
public:
  ScreenBuffer(uint16_t w, uint16_t h);
  inline int getWidth(){
    return width;
  }
  inline int getHeight(){
    return height;
  }
  void setBuffer(uint8_t* buffer){
    pixels = (Colour*)buffer;
  }
  uint8_t* getBuffer(){
    return (uint8_t*)pixels;
  }
  Colour getPixel(unsigned int x, unsigned int y);
  void setPixel(unsigned int x, unsigned int y, Colour c);
  void invertPixel(unsigned int x, unsigned int y);
  void fill(Colour c);
  void fade(uint16_t steps);
  void clear(){
    cursor_x = cursor_y = 0;
    fill(BLACK);
  }
  void clear(int x, int y, int width, int height){
    fillRectangle(x, y, width, height, BLACK);
  }
  void invert();
  void invert(int x, int y, int width, int height);
  void draw(int x, int y, ScreenBuffer& pixels);

  // lines and rectangles
  void drawLine(int fromX, int fromY, int toX, int toY, Colour c);
  void drawVerticalLine(int x, int y, int length, Colour c);
  void drawHorizontalLine(int x, int y, int length, Colour c);
  void drawRectangle(int x, int y, int width, int height, Colour c);
  void fillRectangle(int x, int y, int width, int height, Colour c);
  
  // circles - maybe add arch, ellipse here as well
  void drawCircle(uint16_t x, uint16_t y, uint16_t r, Colour c);
  void fillCircle(uint16_t x, uint16_t y, uint16_t r, Colour c);

  // text
  void print(int x, int y, const char* text);
  void drawChar(uint16_t x, uint16_t y, unsigned char c, Colour fg, Colour bg, uint8_t size);
  void drawRotatedChar(uint16_t x, uint16_t y, unsigned char c, Colour fg, Colour bg, uint8_t size);
  void setCursor(uint16_t x, uint16_t y);
  void setTextColour(Colour c);
  void setTextColour(Colour fg, Colour bg);
  void setTextSize(uint8_t s);
  void setTextWrap(bool w);
  // void setFont(int font, int size);
  void write(uint8_t c);
  void print(const char* str);
  void print(int num);
  void print(float num);
  static ScreenBuffer* create(uint16_t width, uint16_t height);
};

#endif // __ScreenBuffer_h__
