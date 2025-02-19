#include "./LED_Controller/LED_Controller.h"
#include <tinyNeoPixel_Static.h>

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      89

bool preventSleep = false;   // Toggle true to prevent sleep - eg during LED routine
bool ledActive = false;
bool ledModeSelectActive = false;
uint8_t ledMode = 0;
bool ledOn = false;

uint8_t singleColorIntensity = 5;
bool ledFlash = true;
bool pixelTest = false;

bool awakeLED = false;
uint32_t awakeLEDMillis = 0;
uint16_t ledDuration = 1000;

int8_t zoneFade[6] = {0, 0, 0, 0, 0, 0};
uint32_t millisCounter0 = 0;
uint32_t millisCounter1 = 0;
uint32_t millisCounter2 = 0;
uint32_t millisCounter3 = 0;
uint32_t millisCounter4 = 0;
uint32_t millisCounter5 = 0;

uint32_t *millisCounters[6] = {
      &millisCounter0, 
      &millisCounter1, 
      &millisCounter2, 
      &millisCounter3, 
      &millisCounter4, 
      &millisCounter5      
};


uint16_t fadeIncr = 50;
bool zoneActive = false;


// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];



// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.

      // if (awakeLED == true && millis() - awakeLEDMillis > ledDuration) {
      //       // leds.setPixelColor(0, leds.Color(0, 0, 0)); // Moderately bright green color.
      //       // leds.show(); // This sends the updated pixel color to the hardware.
      //       // awakeLED = false;
      // }

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, neoPixelDataPin, NEO_GRB, pixels);

int delayval = 50;

void ledLoop() {
      if (zoneActive == true) {
            uint8_t arrSize = sizeof(zoneFade) / sizeof(zoneFade[0]);
            uint8_t ledTriggersActive = 6;
            for (int i = 0; i < arrSize; i++) {
                  if (zoneFade[i] >= 0) {
                        if (millis() - *millisCounters[i] > fadeIncr) {
                              *millisCounters[i] = millis();
                              switch(i) {
                                    case 0: {
                                          singleColorWrite(0, zoneFade[i], 0, 2, 3, -1);  
                                          break;                                        
                                    }
                                    case 1: {
                                          singleColorWrite(0, zoneFade[i], 0, 0, 1, -1);  
                                          break;     
                                    }
                                    case 2: {
                                          singleColorWrite(0, zoneFade[i], 0, 0, 1, 2, 3, -1);  
                                          break;                                            
                                    }
                                    case 3: {
                                          singleColorWrite(0, 0, zoneFade[i], 6, 7, -1);  
                                          break;                                        
                                    }
                                    case 4: {
                                          singleColorWrite(0, 0, zoneFade[i], 4, 5, -1);  
                                          break;     
                                    }
                                    case 5: {
                                          singleColorWrite(0, 0, zoneFade[i], 4, 5, 6, 7, -1);  
                                          break;                                            
                                    }
                              }
                              zoneFade[i]--;
                        }
                  } else {
                        ledTriggersActive--;
                  }
            }
            if (ledTriggersActive <= 0) {
                  zoneActive = false;
                  // digitalWriteFast(LED_EN, LOW);
            }
      }

      if (ledOn == true) {
            if (ledMode == 1) {
                  rainbow();
            } else if (ledMode == 2) {
                  rainbowCycle();
            } else if (ledMode == 3) {
                  theaterChaseRainbow();
            }        
      }

      static uint16_t ledCommandCount = 0;
      static uint32_t ledMillis = 0;

      if (pixelTest == true) {
            if (millis() - ledMillis > delayval) {
                  digitalWriteFast(LED_EN, HIGH);
                  ledMillis = millis();

                  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
                  leds.setPixelColor(ledCommandCount, leds.Color(0, 5, 0)); // Moderately bright green color.

                  leds.show(); // This sends the updated pixel color to the hardware.        
                  
                  if (ledCommandCount < NUMPIXELS) {
                        ledCommandCount++;
                  } else {
                        ledCommandCount = 0;
                        pixelTest = false;  
                        leds.clear();
                  }
            }


            
            // // with tinyNeoPixel_Static, since we have the pixel array, we can also directly manipulate it - this sacrifices the correction for the pixel order, and the clarity of setColor to save a tiny amount of flash and time.
            // for (int i = 0; i < (NUMPIXELS * 3); i++) {
            // pixels[i] = 5; // set byte i of array (this is channel (i%3) of led (i/3) (respectively, i%4 and i/4 for RGBW leds)
            // leds.show(); // show
            // delay(delayval); // delay for a period of time
            // pixels[i] = 0; // turn off the above pixel
            // // result is that each pixel will cycle through each of the primary colors (green, red, blue for most LEDs) in turn, and only one LED will be on at a time.
            // }        

  
      } else {
            // digitalWriteFast(LED_EN, LOW);
            // preventSleep = false;
      }
}

void ledOnOff() {


      ledOn = !ledOn;
      if (ledOn) {
            digitalWriteFast(LED_EN, HIGH);
            enc1AltToggle(0); // Send a 0 to turn on current settings on current mode
      } else {
            leds.clear();
            digitalWriteFast(LED_EN, LOW);
            ledActive = false;
            preventSleep = false;
      }
}

void ledModeSelect() {
      ledModeSelectActive = !ledModeSelectActive;

      if (ledModeSelectActive == true) {
            ledOn = true;
            ledActive = true;
            digitalWriteFast(LED_EN, HIGH);
            singleColorWrite(5, 0, 0, 4, 5, 6, 7, -1);  
      } else {
            ledActive = false;
            singleColorWrite(0, 0, 0, 4, 5, 6, 7, -1);  
      }
}

void enc1AltToggle(int8_t direction) {
      switch(ledMode) {
            case 0: {
                  scrollSingleColor(direction);
                  break;
            }
            case 1: {
                  rainbow();
                  break;
            }
            case 2: {
                  rainbowCycle();
                  break;
            }
            case 3: {
                  theaterChaseRainbow();
                  break;
            }
      }
}

const uint8_t numLedModes = 4;
void enc2AltToggle(int8_t direction) {
      int8_t l_ledMode;
      l_ledMode = ledMode += direction;
      if (l_ledMode < 0) ledMode = numLedModes - 1;
      if (l_ledMode >= numLedModes) ledMode = 0;
      switch(ledMode) {
            case 0: {
                  scrollSingleColor(0);
                  break;
            }
            case 1: {
                  rainbow();
                  break;
            }
            case 2: {
                  rainbowCycle();
                  break;
            }
            case 3: {
                  theaterChaseRainbow();
                  break;
            }
      }
}

static int8_t currentActiveSingleColor = 0;
void scrollSingleColor(int8_t direction) {
      currentActiveSingleColor = currentActiveSingleColor += direction;
      if (currentActiveSingleColor < 0) currentActiveSingleColor = 2;
      if (currentActiveSingleColor > 2) currentActiveSingleColor = 0;

      singleColor(currentActiveSingleColor);
}

void singleColor(uint8_t color) {
      if (color != 3) {
            digitalWriteFast(LED_EN, HIGH);
            ledActive = true;

            uint8_t rValue = 0;
            uint8_t gValue = 0;
            uint8_t bValue = 0;
            switch(color) {
                  case 0: {
                        rValue = singleColorIntensity;
                        break;
                  }
                  case 1: {
                        gValue = singleColorIntensity;
                        break;
                  }
                  case 2: {
                        bValue = singleColorIntensity;
                        break;
                  }
            }
            for (int i = 8; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, leds.Color(rValue, gValue, bValue));
            }

            leds.show();
      } 
}

static uint8_t rainbowIterationCount = 0;
static uint16_t rainbowDelay = 100;
static uint32_t rainbowMillis = 0;
void rainbow() {
      preventSleep = true;
      if (millis() - rainbowMillis > rainbowDelay) {
            rainbowMillis = millis();
            for (int i = 0; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, Wheel((i + rainbowIterationCount) & 255));
            }
            leds.show();
            if (++rainbowIterationCount >= 255) rainbowIterationCount = 0;
      }
}

void rainbowCycle() {
      preventSleep = true;
      if (millis() - rainbowMillis > rainbowDelay) {
            rainbowMillis = millis();
            for (int i = 0; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, Wheel(((i * 256 / NUMPIXELS) + rainbowIterationCount) & 255));
            }
            leds.show();
            if (++rainbowIterationCount >= 255) rainbowIterationCount = 0;
      }
}

// Theatre-style crawling lights with rainbow effect
static bool chaseToggle = false;
static uint8_t chaseCount = 0;
const uint8_t chasePixelSeparation = 5;
void theaterChaseRainbow() {
      preventSleep = true;
      if (millis() - rainbowMillis > rainbowDelay) {
            rainbowMillis = millis();
                  if (chaseToggle) {
                        for (uint16_t i = 0; i < NUMPIXELS; i = i + chasePixelSeparation) {
                              leds.setPixelColor(i + chaseCount, Wheel((i + rainbowIterationCount) % 255)); // turn every third pixel on
                        }
                        leds.show();                        
                  } else {
                        for (uint16_t i = 0; i < NUMPIXELS; i = i + chasePixelSeparation) {
                              leds.setPixelColor(i + chaseCount, 0);      // turn every third pixel off
                        }                      
                  }
            if (!chaseToggle) {
                  if (++chaseCount >= chasePixelSeparation) {
                        chaseCount = 0;
                        if (++rainbowIterationCount >= 255) rainbowIterationCount = 0;  
                  }                  
            }

            chaseToggle = !chaseToggle;
      }
      // for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
      //       for (int q = 0; q < 3; q++) {
      //             for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
      //                   strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
      //             }
      //             strip.show();
            
      //             delay(wait);
            
      //             for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
      //                   strip.setPixelColor(i + q, 0);      // turn every third pixel off
      //             }
      //       }
      // }
}

uint32_t Wheel(byte WheelPos) {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85) {
            return leds.Color((255 - WheelPos * 3)  * singleColorIntensity / 255, 0, (WheelPos * 3) * singleColorIntensity / 255);
      }
      if (WheelPos < 170) {
            WheelPos -= 85;
            return leds.Color(0, (WheelPos * 3) * singleColorIntensity / 255, (255 - WheelPos * 3) * singleColorIntensity / 255);
      }
      WheelPos -= 170;
      return leds.Color((WheelPos * 3) * singleColorIntensity / 255, (255 - WheelPos * 3) * singleColorIntensity / 255, 0);
}

void sleepLed() {
      digitalWriteFast(LED_EN, HIGH);
      leds.setPixelColor(0, leds.Color(5, 0, 0)); 
      leds.show(); 
      delay(500);
      leds.setPixelColor(0, leds.Color(0, 0, 0)); 
      leds.show(); 
      if (ledActive == false) {
            delay(500);
            leds.setPixelColor(0, leds.Color(5, 0, 0)); 
            leds.show(); 
            delay(500); 
            leds.setPixelColor(0, leds.Color(0, 0, 0)); 
            leds.show(); 
            digitalWriteFast(LED_EN, LOW);
      }
}

void LEDTrigger(LEDZone zoneToDisplay) {
      digitalWriteFast(LED_EN, HIGH);
      zoneActive = true;
      switch(zoneToDisplay) {
            case LEDZone::Enc1Left: {
                  zoneFade[0] = 5;    
                  break;
            }
            case LEDZone::Enc1Right: {
                  zoneFade[1] = 5;    
                  break;
            }
            case LEDZone::Enc1Middle: {
                  zoneFade[2] = 5;    
                  break;
            }
            case LEDZone::Enc2Left: {
                  zoneFade[3] = 5;    
                  break;
            }
            case LEDZone::Enc2Right: {
                  zoneFade[4] = 5;    
                  break;
            }
            case LEDZone::Enc2Middle: {
                  zoneFade[5] = 5;    
                  break;
            }
      }
      // digitalWriteFast(LED_EN, LOW);
}



void singleColorWrite(uint8_t rVal, uint8_t gVal, uint8_t bVal, ...) {  // Pass a Sentinel Value of -1 at end so we know when to stop iteration
      va_list args;     // Set the unspecified arguments to a list
      va_start (args, args); // va_start(va_list ap, last_named_argument) Tells us where in memory the variadic starts
      int value = va_arg(args, int);
      uint8_t iterations = 0; // Some safety. If User does not pass Sentinel at end, we can keep iterating endlessly
      while (value != -1 && iterations < NUMPIXELS) {
            leds.setPixelColor(value, leds.Color(rVal, gVal, bVal));
            value = va_arg(args, int);    // fetches the next argument in list
            iterations++;
      }
      va_end(args);     // cleans up va_list when we're done
      leds.show();
}

