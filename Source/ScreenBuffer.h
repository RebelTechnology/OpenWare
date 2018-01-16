#ifndef __ScreenBuffer_h__
#define __ScreenBuffer_h__

#include <stdint.h>
#include "device.h"

#if defined SSD1309
typedef uint8_t Colour;
#elif defined SSD1331 || defined SEPS114A
typedef uint16_t Colour;
#else
#error "Invalid configuration"
#endif

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

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
  void setBuffer(uint8_t* buffer){
    pixels = (Colour*)buffer;
  }
  Colour* getBuffer(){
    return pixels;
  }
  Colour getPixel(unsigned int x, unsigned int y);
  void setPixel(unsigned int x, unsigned int y, Colour c);
  void drawLine(int fromX, int fromY, int toX, int toY, Colour c);
  void drawVerticalLine(int x, int y, int length, Colour c);
  void drawHorizontalLine(int x, int y, int length, Colour c);
  void drawRectangle(int x, int y, int width, int height, Colour c);
  void fillRectangle(int x, int y, int width, int height, Colour c);
  // void setOrigin(int absoluteX, int absoluteY);
  // void setFont(int font, int size);
  void print(int x, int y, const char* text);
  void fill(Colour c);
  void invert();
  void fade(uint16_t steps);
  void clear(){
    cursor_x = cursor_y = 0;
    fill(BLACK);
  }
  void clear(int x, int y, int width, int height){
    fillRectangle(x, y, width, height, BLACK);
  }
  inline int getWidth(){
    return width;
  }
  inline int getHeight(){
    return height;
  }
  void draw(int x, int y, ScreenBuffer& pixels);
  static ScreenBuffer* create(uint16_t width, uint16_t height);

  void drawChar(uint16_t x, uint16_t y, unsigned char c, Colour fg, Colour bg, uint8_t size);
  void drawRotatedChar(uint16_t x, uint16_t y, unsigned char c, Colour fg, Colour bg, uint8_t size);
  void setCursor(uint16_t x, uint16_t y);
  void setTextColour(Colour c);
  void setTextColour(Colour fg, Colour bg);
  void setTextSize(uint8_t s);
  void setTextWrap(bool w);

  void write(uint8_t c);
  void print(const char* str);
  void print(int num);
  void print(float num);

protected:
  void drawFastVLine(int x, int y, int h, Colour color);
  void drawFastHLine(int x, int y, int h, Colour color);
};

/* class VideoPatch : public Patch { */
/* public: */
/*   virtual void processVideo(ScreenBuffer& video, AudioBuffer& audio) = 0; */
/* }; */

#endif // __ScreenBuffer_h__
