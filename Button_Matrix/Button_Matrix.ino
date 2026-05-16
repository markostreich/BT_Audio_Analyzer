#include <Keypad.h>

const byte ROWS = 6;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {4,   3,  2,  1},
  {8,   7,  6,  5},
  {12, 11, 10,  9},
  {16, 15, 14, 13},
  {20, 19, 18, 17},
  {24, 23, 22, 21}
};

byte rowPins[ROWS] = {2, 3, 4, 5, 6, 7};
byte colPins[COLS] = {8, 9, 10, 11};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
}

void loop() {
  char button = keypad.getKey();

  if (button) {
    Serial.write((byte)button);
  }
}
