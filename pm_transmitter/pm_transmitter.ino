#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define TxPin 10
#define RxPin 10
#define PowerPin 16

#define axisX A0
#define axisY A1
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define joyclick 2

const unsigned int MAX_INPUT = 50;

RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

void setup()
{
  setup_controller();
  Serial.begin(9600);
  Serial.println("Initializing");
  if (!driver.init())
     Serial.println("init failed");
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

int pos = 0;

void loop() {
  // Send messages through serial input
  while (Serial.available () > 0) {
    processIncomingByte (Serial.read ());
  }
  
  // Test buttons
  if (digitalRead(D6)) {
    Serial.println("left");
    transmit_data("1");
  }
  if (digitalRead(D3)) {
    Serial.println("right");
    transmit_data("2");
  }
  if (digitalRead(D5)) {
    Serial.println("middle");
    transmit_data("3");
  }
//  delay(100);
}
  
void processIncomingByte (const byte inByte) {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;

  switch (inByte) {
    case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      process_data (input_line);
      input_pos = 0;  
      break;

    case '\r':
      break;

    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;

  }
}

void process_data (const char * data) {
  transmit_data(data);
}

void transmit_data(const char * data) {
  driver.send((uint8_t *)data, strlen(data));
  Serial.print("sent ");
  Serial.println(data);
}
