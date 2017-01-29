  
const int TEST_TIME_MS = 500;
byte pins[] = {D0,D1,D2,D3,D4,D5,D6,D7,D8,D9,D10};
const int MAX_PIN = sizeof(pins)-1;



void setup() {
  for (int pin=0; pin <= MAX_PIN; ++pin) {
    pinMode (pins[pin], OUTPUT);
  }
}

void loop() {
  for (int pin = 0; pin <= MAX_PIN; ++pin) {
    digitalWrite (pins[pin], HIGH);
    delay (TEST_TIME_MS);
    digitalWrite (pins[pin], LOW);
    delay (TEST_TIME_MS);
  }
}
