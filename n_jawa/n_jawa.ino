#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define LED_PIN_2 5
// TODO: Add another pin
#define BUTTON_PIN 3
#define N 100
#define LIGHTS_PER_LIGHT 7
#define LIGHTS 8

int brightness = 10;  // 0 through 100
int t = 0;  // Wheel color
int buttonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;
int pressed_value = 50;
int pressed_threshold = 50;
int mode = 0;  // Track mode

int upper = 200;
int lower = 20;
int g = lower;
int dg = 1;
// Start rgb 	(255,223,0)
// End rgb (255,100,0)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(N, LED_PIN_2, NEO_GRB + NEO_KHZ800);

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif  
  Serial.begin(9600);
  strip.begin();  // Lights
  strip_2.begin();  // Lights
  pinMode(BUTTON_PIN, INPUT);  // Button
}

void loop() {
  if (pressed_value > pressed_threshold) {
    Serial.print("pressed value: "); Serial.println(pressed_value);
  }

  // Read button
  if ((millis() - lastDebounceTime) > debounceDelay) {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == HIGH) {
      lastDebounceTime = millis();
    }
  }

  // Respond to button
  if (buttonState == HIGH) {
    Serial.println("Button pressed");
    int numberModes = 3;
    mode = mode + 1 % numberModes;
  }

  switch (mode) {
    case 0:
      g = g + dg;
      all(255, g, 0);
      // all(0, 0, 0);
      if ((g < lower) || (g > upper)) { dg = dg * -1; }
      break;
    case 1:
      brightness = brightness - 5;
      mode = 0;
      Serial.println(brightness);
    default : all(0, 255, 0); break;
  }

  if (brightness < 5) { brightness = 100; }

  strip.show();  // Display
  strip_2.show();  // Display
  t = (t + 5) % 400;  // Update wheel color
  buttonState = 0;  // Reset button
  delay(50);  // 20 fps
}

void all(int r, int g, int b) {
  for (int i = 0; i < 20; i++) { setLight(i, r, g, b); }
}

void setLight(int led, int r, int g, int b) {
  strip.setPixelColor(led, strip.Color(int(r * brightness / 100), int(g * brightness / 100), int(b * brightness / 100)));
  strip_2.setPixelColor(led, strip_2.Color(int(r * brightness / 100), int(g * brightness / 100), int(b * brightness / 100)));
}

void runWheel() {
  for (int i = 0; i < LIGHTS; i++) {
    for (int j = 0; j < LIGHTS_PER_LIGHT; j++) {
      strip.setPixelColor(
        i * LIGHTS_PER_LIGHT + j,
        Wheel((t + (400 / LIGHTS * i)) % 400)
      );
            strip_2.setPixelColor(
        i * LIGHTS_PER_LIGHT + j,
        Wheel((t + (400 / LIGHTS * i)) % 400)
      );
    }
  }
}

void runWheelSame() {
  for (int i = 0; i < LIGHTS; i++) {
    for (int j = 0; j < LIGHTS_PER_LIGHT; j++) {
      strip.setPixelColor(
        i * LIGHTS_PER_LIGHT + j,
        // Wheel((t + (400 / LIGHTS * i)) % 400)
        Wheel((t + (400 / LIGHTS * 0)) % 400)
      );
    }
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