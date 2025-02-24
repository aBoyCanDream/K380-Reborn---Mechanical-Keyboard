#include "./LED_Controller/LED_Controller.h"
#include <tinyNeoPixel_Static.h>
#include <math.h>

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      89

bool preventSleep = false;   // Toggle true to prevent sleep - eg during LED routine
bool ledActive = false;
bool ledModeSelectActive = false;
uint8_t ledMode = 0;
bool ledsOn = false;

uint8_t ledIntensity = 5;

uint32_t animationMillis = 0;

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

// Create relative equal grid of pixels  (in order from right to left, bottom of keyboard to top -- encoder pixels run clockwise. Zero is absolute top right of keyboard). -1 is spaces for larger keys (missing lights)
// 17 elements per row
int8_t row6[] = { 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74,  3,  0};
int8_t row5[] = { 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, -1, 60,  2,  1};
int8_t row4[] = { 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, -1,  7,  4};
int8_t row3[] = { 45, -1, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, -1,  6,  5};
int8_t row2[] = { 32, -1, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, -1, 21, -1, 20, 19};
int8_t row1[] = { 18, 17, 16, 15, -1, -1, -1, 14, -1, -1, -1, 13, 12, 11, 10,  9,  8};

int8_t *grid[] = { row1, row2, row3, row4, row5, row6 };

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

      if (ledsOn == true) {
            if (ledMode == 1) {
                  rainbow();
            } else if (ledMode == 2) {
                  rainbowCycle();
            } else if (ledMode == 3) {
                  theaterChaseRainbow();
            } else if (ledMode == 4) {
                  breathe();
            } else if (ledMode == 5) {
                  wave();
            } else if (ledMode == 6) {
                  eqExample();
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

void ledsOnOff() {


      ledsOn = !ledsOn;
      if (ledsOn) {
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
            ledsOn = true;
            ledActive = true;
            digitalWriteFast(LED_EN, HIGH);
            singleColorWrite(0, 0, 0, 0, 1, 2, 3, -1);  
            singleColorWrite(5, 0, 0, 4, 5, 6, 7, -1);  
      } else {
            singleColorWrite(0, 0, 0, 4, 5, 6, 7, -1);      
            // Set encoder leds to single colors when in singleColor mode        
            if (ledMode == 0) {
                  singleColor();
            }
      }
}

void enc1AltToggle(int8_t direction) {
      switch(ledMode) {
            case 0: {
                  scrollSingleColor(direction);
                  singleColor();
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
            case 4: {
                  scrollSingleColor(direction);
                  breathe();
                  break;
            }
            case 5: {
                  wave();
                  break;
            }
            case 6: {
                  eqExample();
                  break;
            }
      }
}

const uint8_t numLedModes = 7;
void enc2AltToggle(int8_t direction) {
      int8_t l_ledMode;
      l_ledMode = ledMode += direction;
      if (l_ledMode < 0) ledMode = numLedModes - 1;
      if (l_ledMode >= numLedModes) ledMode = 0;
      switch(ledMode) {
            case 0: {
                  scrollSingleColor(0);
                  singleColor();
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
            case 4: {
                  breathe();
                  break;
            }
            case 5: {
                  wave();
                  break;
            }
            case 6: {
                  eqExample();
                  break;
            }
      }
}

static int8_t currentActiveSingleColor = 0;
// static uint32_t currentActiveSingleColorVal = 0;
void scrollSingleColor(int8_t direction) {
      currentActiveSingleColor = currentActiveSingleColor += direction;
      if (currentActiveSingleColor < 0) currentActiveSingleColor = 2;
      if (currentActiveSingleColor > 2) currentActiveSingleColor = 0;
}

void singleColor() {
      digitalWriteFast(LED_EN, HIGH);
      ledActive = true;
      preventSleep = false;

      uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;
      uint32_t colorVal = singleColorValue(ledIntensity);
      for (int i = startPixel; i < NUMPIXELS; i++) {
            leds.setPixelColor(i, colorVal);
      }

      leds.show();
}

uint32_t singleColorValue(uint8_t intensity) {
      uint8_t rValue = 0;
      uint8_t gValue = 0;
      uint8_t bValue = 0;
      switch(currentActiveSingleColor) {
            case 0: {
                  rValue = intensity;
                  break;
            }
            case 1: {
                  gValue = intensity;
                  break;
            }
            case 2: {
                  bValue = intensity;
                  break;
            }
      }
      return leds.Color(rValue, gValue, bValue);
}


static uint16_t breathAnimationCount = 0;
void breathe() {
      const double smoothness_pts = 500; // Needs to be float to get correct math below. larger=slower change in brightness  
      const uint16_t breathDelay = 5;

      preventSleep = true;
      ledActive = false;
      if (millis() - animationMillis > breathDelay) {
            animationMillis = millis();

            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;
            uint8_t intensityValue = gaussianWave(breathAnimationCount, smoothness_pts) * 50 / 256;
            uint32_t colorValue = singleColorValue(intensityValue);
            for (int i = startPixel; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, colorValue);
            }
            leds.show();
            if (++breathAnimationCount >= smoothness_pts) breathAnimationCount = 0;
      }
}

static uint16_t waveAnimationCount = 0;
static uint16_t waveAnimationOffset = 0;
void wave() {
      uint16_t waveAnimationLength = 51;
      uint16_t waveAnimationDelay = 100;

      preventSleep = true;
      ledActive = false;      

      if (millis() - animationMillis > waveAnimationDelay) {
            animationMillis = millis();


            uint32_t colorValue = singleColorValue(ledIntensity);

            // leds.clear();

            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;
            for (int i = startPixel; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, leds.Color(0, 5, 0));
            }

            for (int j = 0; j < 17; j++) {
                  // uint8_t offset = j < 11 ? j : j - 11;
                  uint8_t topPixel = gaussianWave(waveAnimationCount + j, waveAnimationLength, 0.105, 0.8) * 7 / 256; // return value between 0 - 6
                  for (int i = topPixel; i > 0; i--) {
                        // Only animate 1-6. 0 is all leds in column off
                        if (grid[i - 1][j] == -1) continue;
                        if (startPixel == 8 && grid[i - 1][j] < 8) continue;
                        leds.setPixelColor(grid[i - 1][j], colorValue);
                  }                
            }  

            leds.show();
            if (++waveAnimationCount >= waveAnimationLength) waveAnimationCount = 0;
      }
}

// Good explanation of the formula here: https://makersportal.com/blog/2020/3/27/simple-breathing-led-in-arduino
uint8_t gaussianWave(uint16_t frame, double numFrames, double gamma, double beta) { // .09 .8

      double wave_pt = 255.0*(exp(-(pow(((frame/numFrames)-beta)/gamma,2.0))/2.0));
      return static_cast<uint8_t>(wave_pt);      
}

const uint16_t eqDelay = 100;
// static uint16_t eqAnimationCount = 0;
void eqExample() {
      preventSleep = true;
      ledActive = false;      

      if (millis() - animationMillis > eqDelay) {
            animationMillis = millis();
            uint32_t colorValue = singleColorValue(ledIntensity);     
            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;

            leds.fill(leds.Color(0, 0, 0), startPixel);
            uint8_t topPixel = gaussianWave(waveAnimationCount, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                  for (int k = 0; k < 2; k++) {
                        if (grid[i - 1][k] == -1) continue;
                        if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                        leds.setPixelColor(grid[i - 1][k], leds.Color(5, 0, 0));                                 
                  }
                  // if (grid[i - 1][0] == -1) continue;
                  // if (startPixel == 8 && grid[i - 1][0] < 8) continue;
                  // leds.setPixelColor(grid[i - 1][0], colorValue);
            }     

            uint8_t topPixel2 = gaussianWave(waveAnimationCount+4, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel2; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 2; k < 4; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(5, 3, 0));                                 
                        }
            }  

            uint8_t topPixel3 = gaussianWave(waveAnimationCount+8, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel3; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 4; k < 6; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(3, 5, 0));                                 
                        }
            }  
            uint8_t topPixel4 = gaussianWave(waveAnimationCount+10, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel4; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 6; k < 8; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(0, 5, 0));                                 
                        }
            }  

            uint8_t topPixel5 = gaussianWave(waveAnimationCount+9, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel5; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 8; k < 10; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(0, 5, 3));                                 
                        }
            }  
            uint8_t topPixel6 = gaussianWave(waveAnimationCount+2, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel6; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 10; k < 12; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(0, 3, 5));                                 
                        }
            }  
            uint8_t topPixel7 = gaussianWave(waveAnimationCount+3, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel7; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 12; k < 14; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(2, 3, 5));                                 
                        }
            }  
            uint8_t topPixel8 = gaussianWave(waveAnimationCount+1, 17) * 7 / 256; // return value between 0 - 6
            for (int i = topPixel8; i > 0; i--) {
                  // Only animate 1-6. 0 is all leds in column off
                        for (int k = 14; k < 16; k++) {
                              if (grid[i - 1][k] == -1) continue;
                              if (startPixel == 8 && grid[i - 1][k] < 8) continue;
                              leds.setPixelColor(grid[i - 1][k], leds.Color(0, 0, 5));                                 
                        }
            }  

            // // leds.fill(leds.Color(0, 0, 0), startPixel);
            // leds.clear();
            // // leds.setPixelColor(9, colorValue);  
            
            // for (int j = 0; j < 8; j++) {
            //       uint8_t randomValue = random(0, 8);
            //       for (int i = randomValue; i > 0; i--) {
            //             for (int k = 0; k < 2; k++) {
            //                   if (grid[i - 1][(j * 2) + k] == -1) continue;
            //                   if (startPixel == 8 && grid[i - 1][(j * 2) + k] < 8) continue;
            //                   leds.setPixelColor(grid[i - 1][(j * 2) + k], colorValue);                                 
            //             }
            //       }
            // }
            leds.show();

            if (++waveAnimationCount >= 17) waveAnimationCount = 0;
      }
}

static uint8_t rainbowIterationCount = 0;
static uint16_t rainbowDelay = 100;

void rainbow() {
      preventSleep = true;
      ledActive = false;
      if (millis() - animationMillis > rainbowDelay) {
            animationMillis = millis();

            // Don't include encoder leds in ledModeSelect
            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;

            for (int i = startPixel; i < NUMPIXELS; i++) {
                  leds.setPixelColor(i, Wheel((i + rainbowIterationCount) & 255));
            }
            leds.show();
            if (++rainbowIterationCount >= 255) rainbowIterationCount = 0;
      }
}

void rainbowCycle() {
      preventSleep = true;
      ledActive = false;
      if (millis() - animationMillis > rainbowDelay) {
            animationMillis = millis();
            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;
            for (int i = startPixel; i < NUMPIXELS; i++) {
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
      ledActive = false;
      if (millis() - animationMillis > rainbowDelay) {
            animationMillis = millis();
            uint8_t startPixel = ledModeSelectActive == true ? 8 : 0;
                  if (chaseToggle) {
                        for (uint16_t i = startPixel; i < NUMPIXELS; i = i + chasePixelSeparation) {
                              leds.setPixelColor(i + chaseCount, Wheel((i + rainbowIterationCount) % 255)); // turn every third pixel on
                        }
                        leds.show();                        
                  } else {
                        for (uint16_t i = startPixel; i < NUMPIXELS; i = i + chasePixelSeparation) {
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
}

uint32_t Wheel(byte WheelPos) {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85) {
            // Convert 0 - 255 values to 0 to set ledIntensity (eg 0 - 255 -> 0 - 5 at ledIntensity 5). Basically map()
            return leds.Color((255 - WheelPos * 3) * ledIntensity / 255, 0, (WheelPos * 3) * ledIntensity / 255);
      }
      if (WheelPos < 170) {
            WheelPos -= 85;
            return leds.Color(0, (WheelPos * 3) * ledIntensity / 255, (255 - WheelPos * 3) * ledIntensity / 255);
      }
      WheelPos -= 170;
      return leds.Color((WheelPos * 3) * ledIntensity / 255, (255 - WheelPos * 3) * ledIntensity / 255, 0);
}

void sleepLed() {
      digitalWriteFast(LED_EN, HIGH);
      leds.setPixelColor(0, leds.Color(5, 0, 0)); 
      leds.show(); 
      delay(500);
      leds.setPixelColor(0, leds.Color(0, 0, 0)); 
      leds.show(); 
      if (ledActive == false && preventSleep == false) {
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
      if (ledActive == false && preventSleep == false) {
            digitalWriteFast(LED_EN, LOW);
      }
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

