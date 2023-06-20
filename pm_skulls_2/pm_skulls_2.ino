
#include <Adafruit_NeoPixel.h>

#define LED_PIN1 6
#define LED_PIN2 9
#define LED_COUNT 24 // 2 * 12

//#define IMPLEMENTATION FIFO
//#define SECOND 1000

Adafruit_NeoPixel strip1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT, LED_PIN2, NEO_GRB + NEO_KHZ800);

// START SHARED
#include <math.h>

// Custom return struct, update as needed
struct Element {
  int r;
  int g;
  int b;
  int x;
  int y;
};

class LightLocator {
  // Calculates x, y position of led in space
  public:
    int skull_mm = 300;
    int eye_mm = 80;
    int light_mm = 20;
    int led_eye_count = 12;
    float pi = 3.14159;
    float x[48];
    float y[48];
    LightLocator () {
      for (int i = 0; i < 48; i++) {
        x[i] = getX(i);  
        y[i] = getY(i);  
      }
    }
    float getX(int light) {
        int skull =  int(light / 24);
        int eye = int((light - skull * 24) / 12);
        light = light - skull * 24 - eye * 12;
        return skull * skull_mm + eye * eye_mm + cos(float(light) / led_eye_count * 2 * pi) * light_mm + 50;
    }
    float getY(int light) {
        int skull =  int(light / 24);
        int eye = int((light - skull * 24) / 12);
        light = light - skull * 24 - eye * 12;
        return sin(float(light) / led_eye_count * 2 * pi) * light_mm + 50;
    }  
};

class LightCorrector {
  // Corrects light position for misalighment of soldering positions
  public:
    int corrected[48];
    LightCorrector() {
      for (int i = 0; i < 48; i++) {
        corrected[i] = correctLight(i);
      }
    }
    int correctLight(int light) {
      int skull =  int(light / 24);
      int eye = int((light - skull * 24) / 12);
      light = light - skull * 24 - eye * 12;
      if (skull == 0) {
        if (eye==0) { return (light + 8) % 12 + skull * 24 + eye * 12; }
        if (eye==1) { return (light + 8) % 12 + skull * 24 + eye * 12; }
      } else {
        if (eye==0) { return (light + 3) % 12 + skull * 24 + eye * 12; }
        if (eye==1) { return (light + 9) % 12 + skull * 24 + eye * 12; }
      }
      return -1;
    }
    // void check() { for (int i = 0; i < 48; i++) { Serial.print(i); Serial.print(" "); Serial.println(corrected[i]); }}
};


class Controller {
  public:
    int overallCount = 0;
    int hueStart = 0;
    LightLocator lightLocator;
    Controller() {
       lightLocator= LightLocator();
    }
    void next() { overallCount++; }
    
    void getElements(Element * elements, int size) {
      // red(elements, size);
      // rainbow(elements, size);
      rainbowChase(elements, size);
    }
    
    void red(Element * elements, int size) {
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        elements[i] = Element{255, 0, 0, int(x), int(y)};
      }
    }

    void rainbowChase(Element * elements, int size) {
      hueStart += 1;
      rainbow(elements, size);
    }

    void rainbow(Element * elements, int size) {  
      int hue = hueStart % 255;    
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        Element rgbElement = hsvToRgb(hue, 255, 127);
        elements[i] = { rgbElement.r, rgbElement.g, rgbElement.b, int(x), int(y) };
        hue = (hue + (255 / 48)) % 255;
      }
    }
    
    Element hsvToRgb(int h, int s, int v) {
      int region = h / 43;
      int remainder = (h - region * 43) * 6;
      int p = (v * (255 - s)) >> 8;
      int q = (v * (255 - ((s * remainder) >> 8))) >> 8;
      int t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
      int r; int g; int b;
      switch(region) {
          case 0: r = v; g = t; b = p; break;
          case 1: r = q; g = v; b = p; break;
          case 2: r = p; g = v; b = t; break;
          case 3: r = p; g = q; b = v; break;
          case 4: r = t; g = p; b = v; break;
          default: r = v; g = p; b = q; break;
        }
        return { r, g, b, 0, 0 };
    }   
};
// END SHARED

LightCorrector lightCorrector = LightCorrector();
Controller controller = Controller();

void setup() {}

void loop() {
  controller.next();
  int size = 48;
  Element elements[size];
  controller.getElements(elements, size);
  act(elements, size);
}

void act(Element * elements, int size) {
  for (int i = 0; i < size / 2; i++) {
    Element e = elements[i];
    strip1.setPixelColor(lightCorrector.corrected[i], strip1.Color(e.r, e.g, e.b));
    strip1.show(); // TODO: Can to be moved out of the loop?
  }
  for (int i = size / 2; i < size; i++) {
    Element e = elements[i];
    strip2.setPixelColor(lightCorrector.corrected[i] - size / 2, strip1.Color(e.r, e.g, e.b));
    strip2.show(); // TODO: Can to be moved out of the loop?
  }
}
