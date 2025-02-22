#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
extern uint8_t neoPixelDataPin;
extern const uint8_t LED_EN;
extern bool pixelTest;
extern bool preventSleep;     // This prevents sleep for an animation
extern bool ledActive;        // This allows sleep for a stationary color - The color will stay on even when asleep because all pixel values have already been written and the led P mosfet will be held active
extern bool ledModeSelectActive;

enum class LEDZone {
      Enc1Left = 0,
      Enc1Right,
      Enc1Middle,
      Enc2Left,
      Enc2Right,
      Enc2Middle
};

void ledLoop();

void ledModeSelect();
void ledsOnOff();
void enc1AltToggle(int8_t direction);
void enc2AltToggle(int8_t direction);
void scrollSingleColor(int8_t direction);
void singleColor();
uint32_t singleColorValue(uint8_t intensity);
void breathe();
uint8_t gaussianWave(uint16_t frame, double numFrames);
void rainbow();
void rainbowCycle();
void theaterChaseRainbow();
uint32_t Wheel(byte WheelPos);

void singleColorWrite(uint8_t rVal, uint8_t gVal, uint8_t bVal, ...);
void sleepLed();
void LEDTrigger(LEDZone);


#endif