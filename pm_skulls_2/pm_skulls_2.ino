
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
        if (eye == 0) {
          return (light + 8) % 12 + skull * 24 + (1 - eye) * 12;
        }
        if (eye == 1) {
          return (light + 8) % 12 + skull * 24 + (1 - eye) * 12;
        }
      } else {
        if (eye == 0) {
          return (light + 3) % 12 + skull * 24 + eye * 12;
        }
        if (eye == 1) {
          return (light + 9) % 12 + skull * 24 + eye * 12;
        }
      }
      return -1;
    }
};

class HueToRGB {
  public:
    Element rgbs[48];
    HueToRGB() {
      for (int i = 0; i < 48; i++) {
        Element rgbElement = hsvToRgb(i * 5, 255, 127);
        rgbs[i] = { rgbElement.r, rgbElement.g, rgbElement.b, 0, 0 };
      }
    }
    Element hsvToRgb(int h, int s, int v) {
      int region = h / 43;
      int remainder = (h - region * 43) * 6;
      int p = (v * (255 - s)) >> 8;
      int q = (v * (255 - ((s * remainder) >> 8))) >> 8;
      int t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
      int r; int g; int b;
      switch (region) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
      }
      return { max(r, 0), max(g, 0), max(b, 0), 0, 0 };
    }
};

class Controller {
  public:
    int overallCount = 0;
    int hueStart = 0;
    int runnerPos = 0;

    int blinkVal = 200;
    int blinkDelta = -1;
    int eyeVal = 2;

    int fadeX = 0;
    int fadeDelta = 10;

    LightLocator lightLocator;
    HueToRGB hueToRGB;
    Controller() {
      lightLocator = LightLocator();
      hueToRGB = HueToRGB();
    }
    void next() {
      overallCount++;
    }

    void getElements(Element * elements, int size) {
      // return fade(elements, size);
      if (overallCount < 200) {
        return firsts(elements, size);
      }
      if (overallCount < 400) {
        return runner(elements, size);
      }
      if (overallCount < 600) {
        return red(elements, size);
      }
      if (overallCount < 800) {
        return rainbowFade(elements, size);
      }
      if (overallCount < 1000) {
        return rainbowChase(elements, size);
      }
      if (overallCount < 1200) {
        return blink(elements, size);
      }
      if (overallCount < 1400) {
        return fade(elements, size);
      }
      overallCount = 0;
    }

    void fade(Element * elements, int size) {
      for (int i = 0; i < 48; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        float dist = abs(fadeX - x);
//        if (dist < 10) {
//           elements[i] = { 255, 0, 0, int(x), int(y) };
//        } else {
//          elements[i] = { 0, 255, 0, int(x), int(y) };
//        }
        elements[i] = { int(dist / 600 * 255), 255 - int(dist / 600 * 255), 0, x, y };
      }
      fadeX = fadeX + fadeDelta;
      if (fadeX > 500) { fadeDelta = fadeDelta * -1; }
      if (fadeX < 0) { fadeDelta = fadeDelta * -1; }
    }

    void blink(Element * elements, int size) {

      for (int i = 0; i < 48; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        int skull =  int(i / 24);
        int eye = int((i - skull * 24) / 12);
        if (skull * 2 + eye == eyeVal) {
          if (abs(y - 50) > blinkVal) {
            elements[i] = Element{ 0, 0, 0, int(x), int(y) };
          } else {
            elements[i] = Element{ 255, 200, 200, int(x), int(y) };
          }
        } else {
          elements[i] = Element{ 255, 200, 200, int(x), int(y) };
        }
      }

      blinkVal += blinkDelta;
      if (blinkVal < -10) {
        blinkDelta = 1;
      }
      if (blinkVal > 200) {
        blinkDelta = -1;
        eyeVal = eyeVal + 1;
        if (eyeVal > 3) {
          eyeVal = 0;
        }
      }
    }

    void firsts(Element * elements, int size) {
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        if (i % 12 == 0) {
          elements[i] = Element{ 255, 0, 0, int(x), int(y) };
        } else {
          if (i % 12 == 3) {
            elements[i] = Element{ 0, 255, 0, int(x), int(y) };
          } else {
            elements[i] = Element{ 0, 0, 0, int(x), int(y) };
          }
        }
      }
    }

    void runner(Element * elements, int size) {
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        if (i == runnerPos) {
          elements[i] = Element{ 255, 0, 0, int(x), int(y) };
        } else {
          elements[i] = Element{ 0, 0, 0, int(x), int(y) };
        }
      }
      runnerPos = (runnerPos + 1) % 48;
    }

    void red(Element * elements, int size) {
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        elements[i] = Element{ 255, 0, 0, int(x), int(y) };
      }
    }

    void strobe(Element * elements, int size) {
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        switch (overallCount % 3) {
          case 0:
            elements[i] = Element{ 255, 0, 0, int(x), int(y) };
            break;
          case 1:
            elements[i] = Element{ 0, 0, 255, int(x), int(y) };
            break;
          case 2:
            elements[i] = Element{ 0, 255, 0, int(x), int(y) };
            break;
        }
      }
    }

    void rainbowFade(Element * elements, int size) {
      // Accidental finding
      hueStart += 1;
      int hue = hueStart % 48;
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        Element rgbElement = this->hueToRGB.rgbs[hue];
        // Element rgbElement = hsvToRgb(hue, 255, 127);
        elements[i] = { rgbElement.r, rgbElement.g, rgbElement.b, int(x), int(y) };
      }
      //hue = (hue + (255 / 48)) % 255;
      hue = hue + 5 % 240;
    }

    void rainbowChase(Element * elements, int size) {
      hueStart += 1;
      return rainbow(elements, size);
    }

    void rainbow(Element * elements, int size) {
      int hue = hueStart % 48;
      for (int i = 0; i < size; i++) {
        float x = this->lightLocator.x[i];
        float y = this->lightLocator.y[i];
        Element rgbElement = this->hueToRGB.rgbs[hue];
        // Element rgbElement = hsvToRgb(hue, 255, 127);
        elements[i] = { rgbElement.r, rgbElement.g, rgbElement.b, int(x), int(y) };
        hue = (hue + 1) % 48;
      }
    }
};
// END SHARED

LightCorrector lightCorrector = LightCorrector();
Controller controller = Controller();

void setup() {
  strip1.begin();
  strip2.begin();
}

void loop() {
  controller.next();
  int size = 48;
  Element elements[size];
  controller.getElements(elements, size);
  act(elements, size);
}

void act(Element * elements, int size) {
  //  strip1.clear();
  //  strip2.clear();
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
  delay(0);
}
