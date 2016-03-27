//include libraries
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <Adafruit_NeoPixel.h>
#include "DigiKeyboard.h" //DigiKeyboard.println();
#include <OneWire.h>
#include <DallasTemperature.h>

//board setup
#define PIN 12
#define PHOTOTRANSISTOR A9
#define BUTTON A5
#define PULLUP 5
#define FET 1

//Temperature sensor setup
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/////////////////////////////////////////////////////////////
//mode for our switch case
uint8_t mode;

//sleep and button setup
byte btnCount = 0; //number of times button has been pressed
long sleep = 0L; //Sleep time in mills 
long onTime = millis(); //How long has the device been on?

//led setup
uint8_t maxBrightness = 200; //maximium brightness
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, PIN, NEO_GRB + NEO_KHZ800);

/////////////////////////////////////////////////////////////

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, HIGH); //setup FET
  pinMode(PULLUP, INPUT_PULLUP); //add pullup for phototransitor/button

  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW); //setup LED signal bus

  randomSeed(analogRead(11)); //random starting number

  sensors.begin(); //start the tempture sensor
  
  strip.begin();
  strip.setBrightness(maxBrightness);

  ledFlash(1); //provided mode feedback 

}
/////////////////////////////////////////////////////////////


/* Counts the number of button presses and sets sleep mode
   Parms None
   Modified from TinyGlo2 Demo.ino
*/
byte checkButton( void ) {
  //onTime = millis();
  pinMode( BUTTON, HIGH );
  delay( 5 );
  if ( analogRead( BUTTON ) < 50 ) {
    btnCount++;
    if ( btnCount > 3 ) {
      btnCount = 0;
      ledFlash(1);
      sleep = 0;
    }
    if ( btnCount == 1 ) {
      ledFlash(2);
      sleep = ( 15L * 60 * 1000 ) + onTime;
    }
    else if ( btnCount == 2 ) {
      ledFlash(3);
      sleep = ( 30L * 60 * 1000 ) + onTime;
    }
    else if ( btnCount == 3 ) {
      ledFlash(4);
      sleep = ( 60L * 60 * 1000 ) + onTime;
    }
    while ( analogRead( BUTTON ) < 50 ) {}
  }
  pinMode( BUTTON, LOW );
}

/* Flashes white number of times
   Parms times, number of times to flash
*/
void ledFlash(uint8_t times) {
  for (uint8_t n = 0; n < times; n++) { //select all pins
    strip.setPixelColor(0, 255, 255, 255); //set each pin to on
    strip.setPixelColor(1, 0); //set each pin to off
    strip.setPixelColor(2, 0); //set each pin to off
    strip.show();
    delay(250);
    strip.setPixelColor(0, 0); //set each pin to off
    strip.show();
    delay(250);
  }
}

/* Gets the tempture from the sensor
   Parms F or C for Temperature scale
*/
float getTemp(char tScale) {
  sensors.requestTemperatures(); // Send the command to get temperatures
  //the sensor returns the temp in celsius 83.30
  float celsius = (sensors.getTempCByIndex(0));
  float fahrenheit = celsius * 1.8 + 32.0;
  if (tScale == 'C') {
    return celsius;
  }
  else if (tScale == 'F') {
    return fahrenheit;
  }
}

/* Sets the mode variable for our switch case
   Parms null
*/
void getMode(void) {
  uint16_t lightLevel = analogRead( PHOTOTRANSISTOR ) / 4;
  onTime = millis();
  checkButton();
  if ( sleep == 0 || onTime < sleep) {
    if ( lightLevel > 150 ) { //too bright turn off
      mode = 3;
    }
    else if ( getTemp ('F' ) <= 60 ) { //set cool mode
      mode = 1;
    }
    else { //set warm mode
      mode = 2;
    }
  }
  else{
  mode = 3;
  }
}
/* Turns off all lights
   Parms null
*/
void allOff(void) {
  for (int i = 0; i < strip.numPixels(); i++) { //select all pins
    strip.setPixelColor(i, 0); //set each pin to off
    strip.show();
  }
}
/* Fades up the LED lights
   Parm wait: how long the transition should take in mills
   Parm gMin: The mininum value of the gradient
   Parm gMax: The maximium value of the gradient
*/
void colorFade(uint8_t wait, uint16_t gMin, uint16_t gMax ) {
  //parms, wait, start value, end value,
  uint16_t i, j, start, end;

  for ( i = 0; i < strip.numPixels(); i++ ) {

    //get random values for start and end
    start = random( gMin, gMax );
    end = random( start, gMax );

    //going up
    for (j = start; j < end; j++) {
      checkButton();
      strip.setPixelColor( i, Wheel( ( i + j ) & 255 ) );
      delay(wait);
      strip.show();
    }
  }
}

/* Gets the tempture from the sensor
   Parms Input a value 0 to 255 to get a color value.
   The colours are a transition r - g - b - back to r.
*/
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


/////////////////////////////////////////////////////////////
void loop() {
  //DigiKeyboard.println(btnCount);

  /*
     ledFlash(10);
    DigiKeyboard.println(sleep);
    DigiKeyboard.println(onTime);*/
  switch (mode) { //Measure ambient light level and temp
    case 1:    // Light is low and temp below 60F
      getMode();
      colorFade(200, 110, 255); //cool
      break;
    case 2:  // light is low temp above 60F
      getMode();
      colorFade(200, 30, 115); //warm
      break;
    case 3:  // light level low, turn off
      getMode();
      allOff();
      break;
    default:
      getMode();
      allOff();
  }
}
/////////////////////////////////////////////////////////////

