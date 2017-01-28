
const int MAX_PIN = 13;
const int TEST_TIME_MS = 500;

void setup() {
  for (int pin=0; pin <= MAX_PIN; ++pin) {
    pinMode (pin, OUTPUT);
  }
}

void loop() {
  for (int pin = 0; pin <= MAX_PIN; ++pin) {
    digitalWrite (pin, HIGH);
    delay (TEST_TIME_MS);
    digitalWrite (pin, LOW);
    delay (TEST_TIME_MS);
  }
}
