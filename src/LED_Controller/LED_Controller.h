#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
extern uint8_t neoPixelDataPin;
extern const uint8_t LED_EN;
extern bool pixelTest;
extern bool preventSleep;     // This prevents sleep for an animation
extern bool ledActive;        // This allows sleep for a stationary color
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
void ledOnOff();
void enc1AltToggle(int8_t direction);
void enc2AltToggle(int8_t direction);
void scrollSingleColor(int8_t direction);
void singleColor(uint8_t color);
void rainbow();
void rainbowCycle();
void theaterChaseRainbow();
uint32_t Wheel(byte WheelPos);

void singleColorWrite(uint8_t rVal, uint8_t gVal, uint8_t bVal, ...);
void sleepLed();
void LEDTrigger(LEDZone);


#endif