typedef enum {
	YELLOW, RED, NONE
} LEDcolour;

void EffectsBox_Init(void);
void EffectsBox_Main(void);

void setLed(uint8_t led, LEDcolour col);
