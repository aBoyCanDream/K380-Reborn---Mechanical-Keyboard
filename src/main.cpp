#include <Arduino.h>
#include <tinyNeoPixel_Static.h>

// Needed for sleep functions:
#include <avr/sleep.h>

// Use const pin assignments to maximize efficiency per https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Ref_Digital.md. Also, some of the newer calls need these as known consts at compile time
const uint8_t OPTOSW1 = PIN_PC5;
const uint8_t OPTOSW2 = PIN_PC4;
const uint8_t OPTOSW3 = PIN_PA7;
const uint8_t OPTOSW4 = PIN_PB7;
const uint8_t OPTOSW5 = PIN_PB5;
const uint8_t OPTOSW6 = PIN_PC3;

const uint8_t ENC1BTN = PIN_PB2;
const uint8_t ENC1A = PIN_PA6;
const uint8_t ENC1B = PIN_PB3;

const uint8_t ENC2BTN = PIN_PC2;
// const uint8_t ENC2A = PIN_PB6;
const uint8_t ENC2B = PIN_PC1;

uint8_t neoPixelDataPin = PIN_PC1;

const uint8_t LED_CHK = PIN_PA3;
const uint8_t LED_EN = PIN_PA5;

const uint8_t KEY_PRESS_LENGTH = 6;  // in milliseconds. We were seeing consistent clicks at 4ms (3ms was bad). To be safe, we'll set this 6. Adjust if need be
const uint16_t INTERVAL_BETWEEN_PRESSES = 26; // in milliseconds. Consistent at 24 but adding a little margin of error

int16_t enc1IncrDecr = 0;
bool enc1ALastState = 1;
bool enc1IncrKeyDown = false;
bool enc1DecrKeyDown = false;
uint32_t enc1IncrDecrKeyDownMillis = 0;
uint32_t enc1IncrDecrKeyPressIntervalMillis = 0;

uint16_t encLongPressThr = 1000; // in millisecs
bool enc1BtnLastState = 1;
uint32_t enc1BtnKeyDownMillis = 0;
bool enc1BtnPressed = false;
bool enc1BtnKeyDown = false;
bool enc2BtnLastState = 1;
uint32_t enc2BtnKeyDownMillis = 0;
bool enc2BtnPressed = false;
bool enc2BtnKeyDown = false;

int16_t enc2IncrDecr = 0;
bool enc2ALastState = 1;
uint32_t enc1BtnDebounceMillis = 0;
uint32_t enc2BtnDebounceMillis = 0;
uint32_t triggerMillis = 0;
int btnDebounceDelay = 50;
int lastButtonState = 1;
bool triggerOpto = false;
bool holdTimeMode = true;

// changed in ISR, so must be volatile or compiler will optimize it away. Also, must only be 1 byte in length or interrupt can trigger between bytes during read
volatile byte interrupt1 = 0; 
volatile byte interrupt2 = 0;
volatile byte interrupt3 = 0;

// dev stuff
uint32_t previousMillis = 0;
byte ledState;

bool ledFlash = true;

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      2

// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, neoPixelDataPin, NEO_GRB, pixels);

int delayval = 500; // delay for half a second

void goToSleep() {
      // Go to sleep
      // Serial.flush(); // First flush Serial if using Serial

      // Turn off ADC before sleeping. It will be burning over 100uA in sleep mode otherwise.
      ADCPowerOptions(ADC_DISABLE);  // Turn off ADC
      //Then:
      //ADCPowerOptions(ADC_ENABLE);     // to turn on ADC after wake if needed

      //Dev
      digitalWriteFast(OPTOSW1, LOW); 
      digitalWriteFast(OPTOSW2, LOW); 

      // Turn all our interrupts back on
      // PORTB.PIN6CTRL  = 0b00001011; //ENC2A - PULLUPEN = 1, ISC = 2 interrupt on falling
      // PORTA.PIN6CTRL  = 0b00000011; // ENC1A
      // PORTB.PIN2CTRL  = 0b00001011; // ENC1BTN
      // PORTC.PIN2CTRL  = 0b00001011; // ENC2BTN

      //Dev
      PORTA.PIN4CTRL  = 0b00001011; //PULLUPEN = 1, ISC = 3 interrupt on falling
      PORTB.PIN1CTRL  = 0b00001010; //PULLUPEN = 1, ISC = 2 interrupt on rising
      PORTB.PIN6CTRL  = 0b00001011; //PULLUPEN = 1, ISC = 2 interrupt on falling

      // Then sleep!
      sleep_cpu(); 
}


void setup() {
      Serial.swap(1);
      // Serial.pins(PIN_PB2, PIN_PB3);
      pinMode(PIN_PA1, OUTPUT);

//   Serial.begin(115200);

  // These new APIs of the megatinycore maximize speed and efficiency. Further info @ https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Ref_Digital.md.
      pinModeFast(ENC1BTN, INPUT_PULLUP);
      pinModeFast(ENC1A, INPUT);
      pinModeFast(ENC1B, INPUT);
      pinModeFast(ENC2BTN, INPUT_PULLUP);
      // pinModeFast(ENC2A, INPUT);
      pinModeFast(ENC2B, INPUT);

      pinModeFast(LED_CHK, INPUT_PULLUP);
      pinModeFast(LED_EN, OUTPUT);

      pinModeFast(OPTOSW1, OUTPUT);
      pinModeFast(OPTOSW2, OUTPUT);
      pinModeFast(OPTOSW3, OUTPUT);
      pinModeFast(OPTOSW4, OUTPUT);
      pinModeFast(OPTOSW5, OUTPUT);
      pinModeFast(OPTOSW6, OUTPUT);

      pinModeFast(neoPixelDataPin, OUTPUT);

      // Set all the unused pins to INPUT_PULLUP to prevent them from floating (especially during sleep) and wasting energy
      // pinModeFast(PIN_PA1, INPUT_PULLUP);
      // pinModeFast(PIN_PA2, INPUT_PULLUP);
      pinModeFast(PIN_PA4, INPUT_PULLUP);
      pinModeFast(PIN_PB4, INPUT_PULLUP);
      pinModeFast(PIN_PB1, INPUT_PULLUP);
      // pinModeFast(PIN_PB0, INPUT_PULLUP);
      pinModeFast(PIN_PB0, OUTPUT);

      pinModeFast(PIN_PB6, INPUT_PULLUP);

      // Dev test
      // You need to have declared the pull-up previously in pinMode. If declaring PULLUPEN with PINxCTRL on an async pin when it is not already held high will cause the interrupt to trigger on declaration
      PORTA.PIN4CTRL  = 0b00001011; //PULLUPEN = 1, ISC = 3 interrupt on falling
      PORTB.PIN1CTRL  = 0b00001010; //PULLUPEN = 1, ISC = 2 interrupt on rising
      PORTB.PIN6CTRL  = 0b00001011; //ENC2A - PULLUPEN = 1, ISC = 2 interrupt on falling
      PORTA.PIN6CTRL  = 0b00000011; // ENC1A
      PORTB.PIN2CTRL  = 0b00001011; // ENC1BTN
      PORTC.PIN2CTRL  = 0b00001011; // ENC2BTN
      // Test to measure VDD
      // Serial.println("analogRead(ADC_VDDDIV10):");
      // Serial.println(analogRead(ADC_VDDDIV10));



      // We're not using ADC so turn off to save power
      ADCPowerOptions(ADC_DISABLE);

      // Sleep
      // To use sleep, we have to first select our mode and then enable with two calls below:
      set_sleep_mode(SLEEP_MODE_STANDBY);
      sleep_enable();

      // We can keep this in setup(). Then to start sleep, we call somewhere in loop():
      // sleep_cpu(); 
}

void loop() {

        //BlinkWithoutDelay, just so you can confirm that the sketch continues to run.
            unsigned long currentMillis = millis();        
        if (ledFlash) {

            if (currentMillis - previousMillis >= 1000) {
            previousMillis = currentMillis;
            if (ledState == LOW) {
                  ledState = HIGH;
            } else {
                  ledState = LOW;
            }
            digitalWrite(OPTOSW1, ledState);
            }            
        }

      
      bool enc1BtnState = digitalReadFast(ENC1BTN);
      if (enc1BtnState == LOW && millis() - enc1BtnDebounceMillis > btnDebounceDelay && enc1BtnPressed == false) {
            // No need to handle super fast clicks. Don't think User can do 6ms clicks. If you think otherwise, you can change code to behave like encoder. Also, our software debounce is preventing crazy fast clicks
            enc1BtnDebounceMillis = millis();

            enc1BtnKeyDownMillis = millis();
            enc1BtnPressed = true;
            // for (int i = 0; i < enc1IncrDecr; i++) {
            //       digitalWriteFast(OPTOSW1, HIGH); 
            //       delay(1000);
            //       digitalWriteFast(OPTOSW1, LOW); 
            //       delay(1000);
            // }
            // enc1IncrDecr = 0;
            
      } 

      // Handle short press and long press of middle button
      if (enc1BtnPressed == true && enc1BtnState == HIGH) {
            enc1BtnPressed = false;
            if (millis() - enc1BtnKeyDownMillis > encLongPressThr) { // Do long press

            } else {    // short press
                  enc1BtnKeyDown = true;
                  enc1BtnKeyDownMillis = millis(); // We'll re-use this millis
                  digitalWriteFast(OPTOSW1, HIGH); 
            }
      }

      bool enc2BtnState = digitalReadFast(ENC2BTN);
      if (enc2BtnState == LOW && millis() - enc2BtnDebounceMillis > btnDebounceDelay && enc1BtnPressed == false) {
            enc2BtnDebounceMillis = millis();

            enc2BtnKeyDownMillis = millis();
            enc2BtnPressed = true;
      } 

      if (enc2BtnPressed == true && enc2BtnState == HIGH) {
            enc2BtnPressed = false;
            if (millis() - enc2BtnKeyDownMillis > encLongPressThr) { // Do long press

            } else {    // short press
                  enc2BtnKeyDown = true;
                  enc2BtnKeyDownMillis = millis(); // We'll re-use this millis
                  //digitalWriteFast(?, HIGH); 
            }
      }

      //       if (digitalReadFast(PIN_PB1) == LOW && millis() - btnDebounceMillis > btnDebounceDelay) {
      //       btnDebounceMillis = millis();
      //       Serial.println("Hello 12345");
            
      // } 


      if (interrupt1){
      interrupt1 = 0;
      //     Serial.println("I1 fired");
            goToSleep();
      }

      if (interrupt2){
      interrupt2 = 0;
      //     ledFlash = !ledFlash;
            // Serial.println("I2 fired");
            digitalWriteFast(PIN_PB0, CHANGE);
      }

      if (interrupt3){
            interrupt3 = 0;
//     ledFlash = !ledFlash;
      // Serial.println("I3 fired");
      digitalWriteFast(OPTOSW2, HIGH); 
      }

      // Read Encoder 1
      bool enc1AState = digitalReadFast(ENC1A);

      if (enc1AState == LOW && enc1ALastState == HIGH) {
            // bool enc1CState = digitalRead(ENC1B);
            if (digitalRead(ENC1B)/*  == HIGH */) {
                  enc1IncrDecr++;                  
            } else {
                  enc1IncrDecr--;
            }

      }
      enc1ALastState = enc1AState;

      // Handle key presses
      if (enc1IncrDecr != 0) {
            if (enc1IncrKeyDown == false && enc1DecrKeyDown == false && millis() - enc1IncrDecrKeyPressIntervalMillis > INTERVAL_BETWEEN_PRESSES) {
                  if (enc1IncrDecr > 0) {
                        enc1IncrKeyDown = true;
                        enc1IncrDecrKeyDownMillis = millis();
                        digitalWriteFast(OPTOSW1, HIGH); 
                  } else {
                        enc1DecrKeyDown = true;
                        enc1IncrDecrKeyDownMillis = millis();
                        digitalWriteFast(OPTOSW2, HIGH); 
                  }            
            }
      }

      // Handle key release
            // Release Button
      if (enc1IncrKeyDown == true && millis() - enc1IncrDecrKeyDownMillis > KEY_PRESS_LENGTH) {
            // Start time for interval between presses
            enc1IncrDecrKeyPressIntervalMillis = millis();
            enc1IncrKeyDown = false;     

            digitalWriteFast(OPTOSW1, LOW); 
            enc1IncrDecr--;
      } else if (enc1DecrKeyDown == true && millis() - enc1IncrDecrKeyDownMillis > KEY_PRESS_LENGTH) {
            enc1IncrDecrKeyPressIntervalMillis = millis();
            enc1DecrKeyDown = false;     

            digitalWriteFast(OPTOSW2, LOW); 
            enc1IncrDecr++;
      }

      if (enc1BtnKeyDown == true && millis() - enc1BtnKeyDownMillis > KEY_PRESS_LENGTH) {
            enc1BtnKeyDown = false;     
            digitalWriteFast(OPTOSW1, LOW); 
      }

      if (enc2BtnKeyDown == true && millis() - enc2BtnKeyDownMillis > KEY_PRESS_LENGTH) {
            enc2BtnKeyDown = false;     
            //digitalWriteFast(?, LOW); 
      }


      // // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

      // for (int i = 0; i < NUMPIXELS; i++) {

      // // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      // leds.setPixelColor(i, leds.Color(0, 1, 0)); // Moderately bright green color.

      // leds.show(); // This sends the updated pixel color to the hardware.

      // delay(delayval); // Delay for a period of time (in milliseconds).
      // }
      // // with tinyNeoPixel_Static, since we have the pixel array, we can also directly manipulate it - this sacrifices the correction for the pixel order, and the clarity of setColor to save a tiny amount of flash and time.
      // for (int i = 0; i < (NUMPIXELS * 3); i++) {
      // pixels[i] = 1; // set byte i of array (this is channel (i%3) of led (i/3) (respectively, i%4 and i/4 for RGBW leds)
      // leds.show(); // show
      // delay(delayval); // delay for a period of time
      // pixels[i] = 0; // turn off the above pixel
      // // result is that each pixel will cycle through each of the primary colors (green, red, blue for most LEDs) in turn, and only one LED will be on at a time.
      // }
}

// flag bit position corresponds to pin (read from right)
// Each bit in binary is a power of 2:
// 	•	0th bit:  2^0 = 1  → 0x01     -> (Px0)
// 	•	1st bit:  2^1 = 2  → 0x02     -> (Px1)
// 	•	2nd bit:  2^2 = 4  → 0x04     -> (Px2)
// 	•	3rd bit:  2^3 = 8  → 0x08     -> (Px3)
// 	•	4th bit:  2^4 = 16  → 0x10    -> (Px4)
// 	•	5th bit:  2^5 = 32  → 0x20    -> (Px5)
// 	•	6th bit:  2^6 = 64  → 0x40    -> (Px6)
// 	•	7th bit:  2^7 = 128  → 0x80   -> (Px7)
ISR(PORTA_PORT_vect) {
      // First disable all interrupts across all ports. For more info: https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Ref_PinInterrupts.md

      // Ways to disable
      // Clear the lowest three bits of the PINnCTRL register:
      // PORTx.PINnCTRL &= ~(0b00000111);
      // PORTx.PINnCTRL &= ~0x07;
      // PORTx.PINnCTRL &= ~PORT_ISC_gm;
      // // The above three are equivalent.

      // If you know what the value of this register should be with the interrupt off (usually 0x00 if pullup is not on, 0x08 if it is)
      // you can write that directly to save a few bytes of flash, at the cost of making the code harder to read.

      // Dev
      //PORTA.PIN4CTRL = 0x08;
      PORTB.PIN1CTRL = 0x08;
      PORTB.PIN6CTRL = 0x08;

      // Reg code
      // PORTB.PIN6CTRL = 0x00; //ENC2A // If this pin does not have the pullup, and we just want to turn off the interrupt.
      // PORTA.PIN6CTRL = 0x00; // ENC1A
      // PORTB.PIN2CTRL = 0x08; // ENC1BTN // If this pin has the pullup turned on, and we just want to turn off the interrupt.
      // PORTC.PIN2CTRL = 0x08; // ENC2BTN

      interrupt1 = 1;
      byte flags = PORTA.INTFLAGS;
      // This line in the tutorial doesn't work to clear all flags:
      // PORTA.INTFLAGS = 1; 

      // Just explicitly clear as if with multiple interrupts on port:
      PORTA.INTFLAGS = flags; //clear flags
}

ISR(PORTB_PORT_vect) {
      // First disable all interrupts across all ports

      // Dev
      //PORTA.PIN4CTRL = 0x08;
      PORTB.PIN1CTRL = 0x08;
      PORTB.PIN6CTRL = 0x08;

      // Reg code
      // PORTB.PIN6CTRL = 0x00; //ENC2A // If this pin does not have the pullup, and we just want to turn off the interrupt.
      // PORTA.PIN6CTRL = 0x00; // ENC1A
      // PORTB.PIN2CTRL = 0x08; // ENC1BTN // If this pin has the pullup turned on, and we just want to turn off the interrupt.
      // PORTC.PIN2CTRL = 0x08; // ENC2BTN

      byte flags = PORTB.INTFLAGS;
      // You can set instructions for specific interrupts. But we will just wake up and let main() do its thing
      if (flags & 0x02) {     // dev
            interrupt2 = 1;
      }
      // if (flags & 0x04) {     // PB2
            
      // }   
      if (flags & 0x40) {     // PB6
            interrupt3 = 1;
      }      

      // Clear flags
      // PORTB.INTFLAGS = flags; //clear flags
      PORTB.INTFLAGS = flags; //clear flags
}

ISR(PORTC_PORT_vect) {// First disable all interrupts across all ports

      // Dev
      //PORTA.PIN4CTRL = 0x08;
      PORTB.PIN1CTRL = 0x08;
      PORTB.PIN6CTRL = 0x08;     

      // Reg code
      // PORTB.PIN6CTRL = 0x00; //ENC2A // If this pin does not have the pullup, and we just want to turn off the interrupt.
      // PORTA.PIN6CTRL = 0x00; // ENC1A
      // PORTB.PIN2CTRL = 0x08; // ENC1BTN // If this pin has the pullup turned on, and we just want to turn off the interrupt.
      // PORTC.PIN2CTRL = 0x08; // ENC2BTN

      byte flags = PORTC.INTFLAGS;
      // Clear flags
      PORTC.INTFLAGS = flags; //clear flags
}

