#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
#define N_LINES 6
#define N_LED 10
#define N 60
#define BUTTON_PIN 3
//
int n_lines = N_LINES;
int n_led = N_LED;
int n_led_total = N;
int brightness = 20;  // 0 - 100

int lightIndex[N];
int lineDuration[N_LINES];
int lineColor[N_LINES][3];

int buttonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(N, LED_PIN, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N, LED_PIN, NEO_GRBW + NEO_KHZ800);

// da-di-di-dit dit dit di-da-dit
int morse[] = {2, -1, 1, -1, 1, -1, 1, -2,
               1, -2,
               1, -2,
               1, -1, 2, -1, 1, -4};
int morse_size = sizeof(morse) / sizeof(morse[0]); 

int t = 0;
int active_line = 0;
int active_morse_pos = 0;
int active_morse = 0;
int pressed_value = 50;
int pressed_threshold = 50;

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif  
  Serial.begin(9600);
  Serial.println("start setup");

  // Lights
  strip.begin();
  setLightIndex();

  // Button
  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("end setup");
}

void loop() {
  // Serial.println("start loop");
  if (pressed_value > pressed_threshold) {
    Serial.print("pressed value: ");
    Serial.println(pressed_value);
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
    pressed_value = min(pressed_value * 2, 1500);
  }
   
  // Set active line color based on active morse
  if (lineDuration[active_line] <= 0) {
    // Serial.print("Setting line: ");
    // Serial.println(active_line);
    if (active_morse > 0) {
      // Serial.println("Setting on");
      lineColor[active_line][0] = 255;
      lineColor[active_line][1] = 255;
      lineColor[active_line][2] = 255;
    } else {
      // Serial.println("Setting off");
      lineColor[active_line][0] = 0;
      lineColor[active_line][1] = 0;
      lineColor[active_line][2] = 0;
    }
    lineDuration[active_line] = abs(active_morse) * 20;
  }
  
  // Set strips
  for (int line=0; line<n_lines; line++) {
    if (pressed_value > pressed_threshold) {
      // Try Wheel
      // if (lineColor[line][0] > 0) {
        setLine(line, Wheel(t));
      // }
    } else {
      setLine(line, lineColor[line][0], lineColor[line][1], lineColor[line][2]);  
    }
  }
  
  // Display
  strip.show();

  // Activate next line
  if (t % 1 == 0) {
    active_line = (active_line + 1) % N_LINES;
    // Serial.print("Active line: ");
    // Serial.println(active_line);
  }

  // Active next morse
  if (active_line == 0) {
    Serial.print("Duration: ");
    Serial.println(lineDuration[active_line]);
    if (lineDuration[active_line] <= 0) {
      active_morse_pos = (active_morse_pos + 1) % morse_size;
      active_morse = morse[active_morse_pos];
      Serial.print("Active morse: ");
      Serial.println(active_morse);
    }
  }
  Serial.print("Active morse pos: ");
  Serial.println(active_morse_pos);
  Serial.print("Active morse: ");
  Serial.println(active_morse);

  // Decrement durations
  for (int line=0; line<n_lines; line++) {
    lineDuration[line] = max(lineDuration[line] - 1, 0);
  }

  // Decrease pressed value
  pressed_value = max(pressed_value - 2, 50);

  // Update
  t = (t + 5) % 400;
  buttonState = 0;
  delay(50);  // 20 fps
  // Serial.println("end loop");
}

void setLine(int line, int r, int g, int b) {
  for (int led=0; led<n_led; led++) { setLight(line, led, r, g, b); }  
}

void setLine(int line, uint32_t color) {
  for (int led=0; led<n_led; led++) {
    strip.setPixelColor(lightIndex[line * n_led + led], color);
  }
}

void setLight(int line, int led, int r, int g, int b) {
  strip.setPixelColor(
    lightIndex[line * n_led + led],
    strip.Color(
      int(r * brightness / 100),
      int(g * brightness / 100),
      int(b * brightness / 100)
    )
  );
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


void ledsOff() {
  for(int i=0; i<n_led_total; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
}

void setLightIndex() {
  // Example given 6 lines and 10 per line
  // pos 0 > i = 0; j = 0 > index 9
  // pos 9 > i = 0; j = 9 > index 0
  // pos 10 > i = 1; j = 0; > index 10
  for (int line=0; line<n_lines; line++) {
    for (int led=0; led<n_led; led++) {
      int pos = line * n_led + led;
      if (line%2 == 0) {
        lightIndex[pos] = line * n_led + n_led - led - 1;
      } else {
        lightIndex[pos] = line * n_led + led;
      }
    }
  }
}

uint32_t getWheelColor(int period) {
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
