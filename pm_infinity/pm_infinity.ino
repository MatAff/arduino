#include <Adafruit_NeoPixel.h>

#define MIC A0 // Microphone is connected at pin A)
#define LED_PIN 6 //11 // LED are connected to D11
#define N_PIXELS 40 // Number of LED 
#define N 100 //N 100 // Number of samples 
#define fadeDelay 2 // 10 // fade amount
#define noiseLevel 3 // 15 // Amount of noice we want to chop off 

float runningSlopeDiscount = 0.7;
bool resetNonFirecount = false;
int nonFireCountThreshold = 6000;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_RGBW + NEO_KHZ800);

int samples[N]; // storage for a sample 
int periodFactor = 0; // For period calculation
int t1 = -1; 
int T; 
int slope; 
byte periodChanged = 0;

float runningSlope = 1;
int nonFireCount = 0;
int backUpCount = 0;

//extern uint8_t getGreenValueFromColor(uint32_t c);
//extern uint8_t getRedValueFromColor(uint32_t c);
//extern uint8_t getBlueValueFromColor(uint32_t c);

int brightness = 25;

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif  
  // Serial.begin(9600);
  strip.begin();
  ledsOff();
  delay(500);
  displayColor(Wheel(100));
  strip.show();
  delay(500);
}


void loop() {
  if (nonFireCount > nonFireCountThreshold) {
    backUpLoop();
    backUpCount += 1;
    if (backUpCount > 3) {
      nonFireCount = 0;
      backUpCount = 0;
    }
  } else {
    Samples();
  }
}

void Samples() {
  for(int i=0; i<N; i++) {
    samples[i] = analogRead(0);
    Serial.print("Analog read: "); Serial.println(samples[i]);
    if(i>0) {
      slope = samples[i] - samples[i-1];
    }
    else {
      slope = samples[i] - samples[N-1];
    }

    // Update running slope
    if (slope > 0) {
      runningSlope = runningSlope * runningSlopeDiscount + abs(slope) * (1 - runningSlopeDiscount);
    }
    
    Serial.print("slope: "); Serial.println(slope);
    Serial.print("running slope: "); Serial.println(runningSlope);
    // if(abs(slope) > noiseLevel) {/
    if(abs(slope) > (runningSlope * 1.1)) {
      if (resetNonFirecount) { nonFireCount = 0; }
      if(slope < 0) {
      // if(slope > 0) {
        calculatePeriod(i);
        if(periodChanged == 1) {
          Serial.println("Display color !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
          auto color = getColor(T);
          // displayColor(color);
          uint32_t rgb;
          rgb = grb_to_rgb(color);
          displayColor(rgb);
        }
      }
    }
    else {
      nonFireCount += 1;
      Serial.println("Turning off");
      ledsOff();
    }
    periodFactor += 1;
    delay(1);
  }
}

uint32_t grb_to_rgb(uint32_t grb) {
  uint8_t g = getGreenValueFromColor(grb);
  uint8_t r = getRedValueFromColor(grb);
  uint8_t b = getBlueValueFromColor(grb);
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

uint8_t getGreenValueFromColor(uint32_t c) {
    return c >> 16;
}

uint8_t getRedValueFromColor(uint32_t c) {
    return c >> 8;
}

uint8_t getBlueValueFromColor(uint32_t c) {
   return c;
}


void calculatePeriod(int i) {
  if(t1 == -1) {
    t1 = i;
  }
  else {
    int period = periodFactor*(i - t1);
    periodChanged = T==period ? 0 : 1;
    T = period;  
    t1 = i;
    periodFactor = 0;
  }
}

uint32_t getColor(int period) {
  if(period == -1)
    return Wheel(0);
    // return WheelRGB(0);
  else if(period > 400)
    return Wheel(5);
    // return WheelRGB(5);
  else
    return Wheel(map(-1*period, -400, -1, 50, 255));
    // return WheelRGB(map(-1*period, -400, -1, 50, 255));
}

void fadeOut()
{
  for(int i=0; i<5; i++) {
    // strip.setBrightness(110 - i*20);
    strip.setBrightness(brightness - i*brightness/5);
    strip.show(); // Update strip
    delay(fadeDelay);
    periodFactor +=fadeDelay;
  }
}

void fadeIn() {
  // strip.setBrightness(100);
  strip.setBrightness(brightness);
  strip.show();

  for(int i=0; i<5; i++) {
    //strip.setBrightness(20*i + 30);
    //strip.show();
    delay(fadeDelay);
    periodFactor+=fadeDelay;
  }
}

void ledsOff() {
  fadeOut();
  for(int i=0; i<N_PIXELS; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
}

void displayColor(uint32_t color) {
  for(int i=0; i<N_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  fadeIn();
}

uint32_t WheelRGB(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, 255 - WheelPos * 3, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 0, 255 - WheelPos * 3);
  }
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

//#include <Adafruit_NeoPixel.h>
//#ifdef __AVR__
// #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
//#endif
//
//// Which pin on the Arduino is connected to the NeoPixels?
//// On a Trinket or Gemma we suggest changing this to 1:
//#define LED_PIN    6
//
//// How many NeoPixels are attached to the Arduino?
//#define LED_COUNT 60
//
//// Declare our NeoPixel strip object:
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW + NEO_KHZ800);
//// Argument 1 = Number of pixels in NeoPixel strip
//// Argument 2 = Arduino pin number (most are valid)
//// Argument 3 = Pixel type flags, add together as needed:
////   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
////   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
////   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
////   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
////   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// setup() function -- runs once at startup --------------------------------

//void setup() {
//  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
//  // Any other board, you can remove this part (but no harm leaving it):
//#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
//  clock_prescale_set(clock_div_1);
//#endif
//  // END of Trinket-specific code.
//
//  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
//  strip.show();            // Turn OFF all pixels ASAP
//  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
//}


// loop() function -- runs repeatedly as long as board is on ---------------

void backUpLoop() {
  // Fill along the length of the strip in various colors...
  colorWipe(strip.Color(127,   0,   0), 50); // Red
  colorWipe(strip.Color(  0, 127,   0), 50); // Green
  colorWipe(strip.Color(  0,   0, 127), 50); // Blue

  // Do a theater marquee effect in various colors...
  theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness

  rainbow(10);             // Flowing rainbow cycle along the whole strip
  theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
}


// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.setBrightness(brightness);
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
