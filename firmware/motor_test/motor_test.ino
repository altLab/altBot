//
// altBot Motor Test
//
// Tries to makes the bot go in a figure 8.
//
// Adapted from ...
//

// Control pins
const int LEFT_PIN = 5;
const int RIGHT_PIN = 9;

void setup() {
  // initialize the digital pins as an outputs:
  pinMode (LEFT_PIN, OUTPUT);
  pinMode (RIGHT_PIN, OUTPUT);
}

void move_forward() {
  digitalWrite (LEFT_PIN, HIGH);
  digitalWrite (RIGHT_PIN, HIGH);
}

void stop() {
  digitalWrite (LEFT_PIN, LOW);
  digitalWrite (RIGHT_PIN, LOW);
}

void turn_left() {
  digitalWrite (LEFT_PIN, LOW);
  digitalWrite (RIGHT_PIN, HIGH);
}

void turn_right() {
  digitalWrite (LEFT_PIN, HIGH);
  digitalWrite (RIGHT_PIN, LOW);
}

void loop() {
  move_forward();
  delay(1000);
  turn_right();
  delay(3000);
  move_forward();
  delay(1000);
  turn_left();
  delay(3000);
  move_forward();
  delay(1000);
  turn_right();
  delay(3000);
}
