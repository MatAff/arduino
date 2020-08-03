// How to Use
// set board: SparkFunPro Micro
// set port
// Ctrl + Shift + U
 
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

#define TxPin 10
#define RxPin 10
#define PowerPin 16

#define LeftPin 4
#define RightPin 3
#define MiddlePin 2

const unsigned int MAX_INPUT = 50;

RH_ASK driver(2000, RxPin, TxPin, PowerPin, false);

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing");
  if (!driver.init())
     Serial.println("init failed");
}

int pos = 0;

void loop() {
  // Send messages through serial input
  while (Serial.available () > 0) {
    processIncomingByte (Serial.read ());
  }
  
  // Test buttons
  if (digitalRead(LeftPin)) {
    Serial.println("left");
    transmit_data("1");
  }
  if (digitalRead(RightPin)) {
    Serial.println("right");
    transmit_data("2");
  }
  if (digitalRead(MiddlePin)) {
    Serial.println("middle");
    transmit_data("3");
  }
  delay(100);
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
