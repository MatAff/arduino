#define axisX A0
#define axisY A1
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define joyclick 2

void setup(void) {
  setup_controller();
  Serial.begin(9600);
}

void setup_controller() {
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(joyclick, INPUT);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  digitalWrite(joyclick, HIGH);
}

void loop(void) {
    double x = analogRead(axisX) / 1024.0;
    double y = analogRead(axisY) / 1024.0;
    Serial.print("x: ");
    Serial.print(x);
    Serial.print(", y: ");
    Serial.print(y);
    Serial.println();
//    delay(250);

    if(buttonPressed(joyclick)) {
        Serial.println("Joyclick pressed!");
        delay(500);
    }
    if(buttonPressed(D3)) {
        Serial.println("D3 pressed!");
        delay(500);
    }
    if(buttonPressed(D4)) {
        Serial.println("D4 pressed!");
        delay(500);
    }
    if(buttonPressed(D5)) {
        Serial.println("D5 pressed!");
        delay(500);
    }
    if(buttonPressed(D6)) {
        Serial.println("D6 pressed!");
        delay(500);
    }
}

// https://medium.com/arduino-playground/checking-for-a-button-press-in-arduino-7681cbb7bde7
// Detects button up events (fires once per press)
int buttonPressed(uint8_t pin) {
  static uint16_t lastStates = 0;
  uint8_t state = digitalRead(pin);
  if (state != ((lastStates >> pin) & 1)) {
    lastStates ^= 1 << pin;
    return state == HIGH;
  }
  return false;
}
