// Skull patterns not reactive.
// Snowflake pattern

#include <Adafruit_NeoPixel.h>
#include <cppQueue.h>

#define LED_PIN1 6
#define LED_PIN2 9
#define LED_COUNT 24 // 2 * 12
#define LED_EYE_COUNT 12
#define IMPLEMENTATION FIFO
#define SECOND 1000
#define SKULL_MM 300
#define EYE_MM 80
#define LIGHT_MM 40
#define PI 3.14159

Adafruit_NeoPixel strip1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT, LED_PIN2, NEO_GRB + NEO_KHZ800);
//LightController lc = LightController();
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// Settings
bool PLAY_LIGHT_CONTROL = false;
bool PLAY_LIGHTLOCATION = false;
bool PLAY_EARLY = false;
bool PLAY_ORIGINALS = false;


// START SHARED
#include <math.h> 

struct Loc {
  float x;
  float y;
};

struct Color {
  int r;
  int g;
  int b;
};

struct Node {
  int skull;
  int eye;
  int light;
  Color color;
};

class LightController {

  private:
    int overallCount = 0;
    Node lights[48];
    int led_eye_count = 12;
    int skull_mm = 300;
    int eye_mm = 80;
    int light_mm = 20;
    float pi = 3.14159;
    Loc source = {-50, 0};
    int delta = 10;
    char* pattern = "fade"; // fade, line, blink, fire
  
    // Blink
    int blinkVal = 200;
    int blinkDelta = -1;
    int eyeVal = 2;
  
    // Fire
    int fireI = 0;

  public:
    LightController () {
      // Set initial colors.
      int i = 0;
      for (int skull=0; skull<2; skull++) {
        for (int eye=0; eye<2; eye++) {
          for (int light=0; light<this->led_eye_count; light++) {
            Color color = {skull * 255, eye * 255, light * 20};
            Node node = {skull, eye, light, color};
            lights[i] = node;
            i++;
          }
        }
      }
    }

    int getOverallCount() { return overallCount; }
    
    Node getLight(int i) { return lights[i]; }
    
    Loc getLightLoc(int skull, int eye, int light) {
      float x = skull * skull_mm + eye * eye_mm + cos(float(light) / led_eye_count * 2 * pi) * light_mm;
      float y = sin(float(light) / led_eye_count * 2 * pi) * light_mm;
      return {x, y};
    }

    float getDistLoc(Loc source, Loc target) {
      return pow(pow(source.x - target.x, 2) + pow(source.y - target.y, 2), 0.5);
    }

    void next() {

      overallCount++;
      
      // Fade pattern
      if (this->pattern=="fade") {
        int i = 0;
        for (int skull=0; skull<2; skull++) {
          for (int eye=0; eye<2; eye++) {
            for (int light=0; light<led_eye_count; light++) {
              Loc target = this->getLightLoc(skull, eye, light);
              float dist = getDistLoc(this->source, target);
              Color color = {int(dist / 600 * 255), 255 - int(dist / 600 * 255), 0};
              Node node = {skull, eye, light, color};
              this->lights[i] = node;
              i++;
            }
          }
        }
        float x = this->source.x + this->delta;
        float y = this->source.y;
        if (x>400) { delta = -10; }
        if (x<0) { delta = 10; }
        this->source = {x, y};
      }

      // Line moving sideways
      if (this->pattern=="line") {
        delta = 5;
        int i = 0;
        for (int skull=0; skull<2; skull++) {
          for (int eye=0; eye<2; eye++) {
            for (int light=0; light<led_eye_count; light++) {
              Loc target = this->getLightLoc(skull, eye, light);
              // float dist = getDistLoc(this->source, target);
              float dist = abs(source.x - target.x);
              Color color = {255 - int(dist) * 15, 255 - int(dist) * 15, 255 - int(dist) * 15};
              Node node = {skull, eye, light, color};
              this->lights[i] = node;
              i++;
            }
          }
        }
        float x = this->source.x + this->delta;
        float y = this->source.y;
        if (x>800) { x = -50; }
  //      if (x<0) { delta = 10; }
        this->source = {x, y};
      }

      // Blink
      if (this->pattern=="blink") {
        int i = 0;
        for (int skull=0; skull<2; skull++) {
          for (int eye=0; eye<2; eye++) {
            for (int light=0; light<led_eye_count; light++) {
              Loc target = this->getLightLoc(skull, eye, light);
              Color color;
              color = {255, 255, 255};
              if (skull*2 + eye==eyeVal) {
                if (abs(target.y) > blinkVal) {
                  color = {0, 0, 0};
                }
              }
              Node node = {skull, eye, light, color};
              this->lights[i] = node;
              i++;
            }
          }
        }
        
        blinkVal += blinkDelta;
        if (blinkVal<-10) { blinkDelta = 1; }
        if (blinkVal>200) {
          blinkDelta = -1; 
          eyeVal = eyeVal + 1;
          if (eyeVal > 3) {
            eyeVal = 0;
          }
        }
      }

      // Fire
      if (this->pattern=="fire") {
        int i = 0;
        float val = iToVal(fireI) * 20;
        for (int skull=0; skull<2; skull++) {
          for (int eye=0; eye<2; eye++) {
            for (int light=0; light<led_eye_count; light++) {
              Loc target = this->getLightLoc(skull, eye, light);
              int strandNr = skull * 4 + eye * 2;
              if (cos(float(light) / led_eye_count * 2 * pi) > 0) {
                strandNr +=1;         
              }
              switch(strandNr) {
                case 0:
                  val = iToVal(fireI) * 20;
                  break;
                case 1:
                  val = iToVal(fireI + 3113, 200, 750, 810) * 20;
                  break;
                case 2:
                  val = iToVal(fireI + 5335, 580, 930, 150) * 20;
                  break;
                case 3:
                  val = iToVal(fireI + 4383, 240, 290, 150) * 20;
                  break;
                case 4:
                  val = iToVal(fireI + 1453, 230, 703, 203) * 20;
                  break;
                case 5:
                  val = iToVal(fireI + 2385, 230, 2240, 260) * 20;
                  break;
                case 6:
                  val = iToVal(fireI + 9482, 590, 200, 520) * 20;
                  break;
                case 7:
                  val = iToVal(fireI + 2952, 290, 740, 920) * 20;
                  break;
              }
              
              Color color;
              if (target.y > val) {
                float y = target.y * -1 + 20;
                float demon = val * -1 + 20;
                float ratio = y / demon;
                color = {255, int(ratio * 255), 0};
              } else {
                color = {0, 0, 0};
              }
              Node node = {skull, eye, light, color};
              this->lights[i] = node;
              i++;
            }
          }
        }
        fireI = fireI + 5;
      }
  
    }

    float iToVal(int i, int a=50, int b=70, int c=110) {
      return sin(i/a) * 1/1 + sin(i/b) * 1/2 + sin(i/c) * 1/3;
    }
    
};
// END SHARED

// Need to go after shared
extern void setPixel(int skull, int eye, int light, Color color);
extern Loc getLightLoc(int skull, int eye, int light);
extern float getDistLoc(Loc source, Loc target);
//extern void locationColor(LightLocation& ll);

LightController lc;

class Lagger {
  private:
    int size;
    cppQueue q = cppQueue(sizeof(Node)); // What if I don't want to initiate this here?
  public:
    Lagger(int size) {
      this->size = size;
      this->q = cppQueue(sizeof(Node), size, IMPLEMENTATION);
    }
    
    lagPixel(Node node) {
      this->q.push(&node);
      for (int i=0; i < q.getCount(); i++) {
          Node n;
          q.peekIdx(&n, i);
          setPixel(n.skull, n.eye, n.light, n.color);
      }
      Node w;
      if (q.getCount() >= this->size) {
        this->q.pop(&w);
      }
      Color BLACK = {0, 0, 0};
      setPixel(w.skull, w.eye, w.light, BLACK);
    }
};

class LightLocation {
  private:
    Loc source = {-50, 0};
  public:
    Color getColor(int skull, int eye, int light);
};

Color LightLocation::getColor(int skull, int eye, int light) {
  Loc target = getLightLoc(skull, eye, light);
  float dist = getDistLoc(this->source, target);
  float x = source.x + 1;
  if (x>400) { x = -100; }
  source = {x, 0};
  Serial.println(dist);
  return {int(dist / 600 * 255), 255 - int(dist / 600 * 255), 0};
}

float getDistLoc(Loc source, Loc target) {
  return pow(pow(source.x - target.x, 2) + pow(source.y - target.y, 2), 0.5);
}

Loc getLightLoc(int skull, int eye, int light) {
  float x = skull * SKULL_MM + eye * EYE_MM + sin(float(light) / LED_EYE_COUNT * 2 * PI) * LIGHT_MM;
  float y = skull * SKULL_MM + eye * EYE_MM + cos(float(light) / LED_EYE_COUNT * 2 * PI) * LIGHT_MM;
  return {x, y};
}

void setPixel(int skull, int eye, int light, Color color) {
  if (skull==1) {
    if (eye==0) { light = (light + 3) % LED_EYE_COUNT; }
    if (eye==1) { light = (light + 9) % LED_EYE_COUNT; }
    light = light + eye * LED_EYE_COUNT;
    strip1.setPixelColor(light, strip1.Color(color.r, color.g, color.b));
    strip1.show();
  } else {
    if (eye==0) { light = (light + 8) % LED_EYE_COUNT; }
    if (eye==1) { light = (light + 8) % LED_EYE_COUNT; }
    light = light + (1 - eye) * LED_EYE_COUNT;
    strip2.setPixelColor(light, strip2  .Color(color.r, color.g, color.b));
    strip2.show();
  }
}

Color BLACK = {0, 0, 0};

void locationColor(LightLocation& ll) {
  for (int skull=0; skull<2; skull++) {
    for (int eye=0; eye<2; eye++) {
      for (int light=0; light<LED_EYE_COUNT; light++) {
        Color color = ll.getColor(skull, eye, light);
        setPixel(skull, eye, light, color);
      }
    }
  }
}

void lightWalk(int wait) {
  for (int skull=0; skull<2; skull++) {
    for (int eye=0; eye<2; eye++) {
      for (int light=0; light<LED_EYE_COUNT; light++) {
        Color red = {255, 0, 0};
        setPixel(skull, eye, light, red);
        delay(wait);
      }
    }
  }
}

void rotate(int skull, int eye, Color color, int wait, bool clean, int start, int dir, int dur, Lagger& lagger) {
  for (int light=0; light<dur; light++) {
    int actLight = (start + LED_EYE_COUNT + light * dir) % LED_EYE_COUNT;
    setPixel(skull, eye, actLight, color);
    Node n = {skull, eye, actLight, color};
    lagger.lagPixel(n);
    delay(wait);
  }
}

void rotateEyesPing(int wait) {
  Lagger lagger = Lagger(5);
  const Color colorArray [] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255}
   };
  for (Color color : colorArray) {
    rotate(0, 0, color, wait, true, 6, 1, 6, lagger);
    rotate(0, 1, color, wait, true, 6, -1, 6, lagger);
    rotate(1, 0, color, wait, true, 6, 1, 6, lagger);
    rotate(1, 1, color, wait, true, 6, -1, 12, lagger);
    rotate(1, 0, color, wait, true, 0, 1, 6, lagger);
    rotate(0, 1, color, wait, true, 0, -1, 6, lagger);
    rotate(0, 0, color, wait, true, 0, 1, 6, lagger);
  }
}


void rotateEyes(int wait) {
  Lagger lagger = Lagger(1);
  const Color colorArray [] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255}
   };
  for (Color color : colorArray) { // for each element in the array
    for (int skull=0; skull<2; skull++) {
      for (int eye=0; eye<2; eye++) {
        rotate(skull, eye, color, wait, true, 0, 1, 12, lagger);
      }
    }
  }
}


void rotateEyesAlt(int wait) {
  Lagger lagger = Lagger(1);
  const Color colorArray [] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255}
   };
  for (Color color : colorArray) { // for each element in the array
    for (int skull=0; skull<2; skull++) {
      for (int eye=0; eye<2; eye++) {
        rotate(skull, eye, color, wait, true, 0, 1 - 2 * eye, 12, lagger);
      }
    }
  }
}

void wipe() {
  for (int skull=0; skull<2; skull++) {
    for (int eye=0; eye<2; eye++) {
      for (int light=0; light<LED_EYE_COUNT; light++) {
        setPixel(skull, eye, light, BLACK);
      }
    }
  }
}

void firsts() {
  wipe();
  setPixel(0, 0, 0, {255, 0, 0});
  delay(500);
  setPixel(0, 1, 0, {0, 255, 0});
  delay(500);
  setPixel(1, 0, 0, {0, 0, 255});
  delay(500);
  setPixel(1, 1, 0, {0, 255, 255});
  delay(500);

}

// Some functions of our own for creating animated effects -----------------

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip1.numPixels(); i++) { // For each pixel in strip...
    strip1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip1.show();                          //  Update strip to match

    strip2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip2.show();                          //  Update strip to match

    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip1.clear();         //   Set all pixels in RAM to 0 (off)
      strip2.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip1.numPixels(); c += 3) {
        strip1.setPixelColor(c, color); // Set pixel 'c' to value 'color'
        strip2.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip1.show(); // Update strip with new contents
      strip2.show(); // Update strip with new contents
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
    strip1.rainbow(firstPixelHue);
    strip2.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip1.show(); // Update strip with new contents
    strip2.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip1.clear();         //   Set all pixels in RAM to 0 (off)
      strip2.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip1.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip1.numPixels();
        uint32_t color = strip1.gamma32(strip1.ColorHSV(hue)); // hue -> RGB
        strip1.setPixelColor(c, color); // Set pixel 'c' to value 'color'
        strip2.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip1.show();                // Update strip with new contents
      strip2.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

void setup() {
  strip1.begin();
  strip2.begin();
  strip1.show();  // Turn OFF all pixels ASAP
  strip2.show();
  strip1.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip2.setBrightness(50);
}

void loop() {

  if (PLAY_LIGHT_CONTROL) {
    while(lc.getOverallCount() < 10000) {
      for (int i = 0; i < 48; i++) {
        Node current = lc.getLight(i);
        setPixel(current.skull, current.eye, current.light, current.color);
      }
     lc.next();
    }
  }

  if (PLAY_LIGHTLOCATION) {    
    LightLocation ll = LightLocation();
    for (int i=0; i < 20; i++) {
      locationColor(ll);
      delay(100);
    }
  }

  if (PLAY_EARLY) {
    //firsts();
    lightWalk(50);
    rotateEyes(100);
    rotateEyesAlt(100);
    rotateEyesPing(50);
  }

  if (PLAY_ORIGINALS) {
    // Fill along the length of the strip in various colors...
    colorWipe(strip1.Color(255,   0,   0), 50); // Red
    colorWipe(strip1.Color(  0, 255,   0), 50); // Green
    colorWipe(strip1.Color(  0,   0, 255), 50); // Blue

    colorWipe(strip2.Color(255,   0,   0), 50); // Red
    colorWipe(strip2.Color(  0, 255,   0), 50); // Green
    colorWipe(strip2.Color(  0,   0, 255), 50); // Blue

    // Do a theater marquee effect in various colors...
    theaterChase(strip1.Color(127, 127, 127), 50); // White, half brightness
    theaterChase(strip1.Color(127,   0,   0), 50); // Red, half brightness
    theaterChase(strip1.Color(  0,   0, 127), 50); // Blue, half brightness

    theaterChase(strip2.Color(127, 127, 127), 50); // White, half brightness
    theaterChase(strip2.Color(127,   0,   0), 50); // Red, half brightness
    theaterChase(strip2.Color(  0,   0, 127), 50); // Blue, half brightness

    rainbow(10);             // Flowing rainbow cycle along the whole strip
    theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
  }
  
}

//void loop() {
//
//  colorWipe(strip1.Color(255,   0,   0), 50); // Red
//  delay(1 * SECOND);
//  colorWipe(strip1.Color(  0, 255,   0), 50); // Green
//  delay(1 * SECOND);
//  colorWipe(strip1.Color(  0,   0, 255), 50); // Blue
//  delay(1 * SECOND);
//
//}
//
//void colorWipe(uint32_t color, int wait) {
//  for(int i=0; i<strip1.numPixels(); i++) {
//    strip1.setPixelColor(i, color); // Set pixel's color (in RAM)
//    strip2.setPixelColor(i, color);
//    strip1.show(); // Update strip to match
//    strip2.show();
//    delay(wait); // Pause for a moment
//  }
//}
//
//void full(Adafruit_NeoPixel strip, int eyeNr, int g, int r, int b) {
//
//}
//
//int reMap(int loc, int offset) {
//  return (loc + offset)%LED_EYE_COUNT;
//}
//
//void blink(int wait) {
//
//
//  // Params
//  float width = 3;
//  float fadeWidth = 1.5;
//  int fadeTime = 300; // ms
//  uint32_t startColor = strip1.Color(0,  0,   0);
//  uint32_t endColor = strip1.Color(255,  255,   255);
//
//  // Calculated values
//  float totalWidth = width + fadeWidth;
//  int nrCycles = fadeTime / wait;
//  float cycleMove = totalWidth / nrCycles;
//
//  // Clear colors
//  colorWipe(strip1.Color(0,   0,   0), wait);
//
//  // Loop through fade
//  for(int i=0; i<=nrCycles; i++) {
//    float startPos = 0 + i * cycleMove;
//    float endPos = startPos + fadeWidth;
//    for (int j; j<width; i++) {
//      float prop = 0.1; // TODO set this
////      uint32_t color = mixColor(startColor, endColor, prop) // Write mixColor function
////      strip1.setPixelColor(j, color); //
//    }
//  }
//
////  // Blink
////  for(int i=0; i<3; i++) {
////    strip1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
////    strip1.show();                          //  Update strip to match
////
////    strip2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
////    strip2.show();                          //  Update strip to match
////
////    delay(wait);                           //  Pause for a moment
////  }
//}
//
//
//
//
//
