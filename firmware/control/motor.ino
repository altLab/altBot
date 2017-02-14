
// Human readable names for motor pins
#define PWM_PIN_LEFT 5
#define PWM_PIN_RIGHT 4
#define DIR_PIN_LEFT 0
#define DIR_PIN_RIGHT 2

// Human readable names for motor direction
#define DIR_FWD 1
#define DIR_BAK 0

// Stop all motors
inline void motor_all_stop() {
  motor_left_stop();
  motor_right_stop();
}

// Stop the left motor
inline void motor_left_stop() {
  analogWrite(PWM_PIN_LEFT, 0);
  digitalWrite(DIR_PIN_LEFT, DIR_FWD);
}

// Stop the right motor
inline void motor_right_stop() {
  analogWrite(PWM_PIN_RIGHT, 0);
  digitalWrite(DIR_PIN_RIGHT, DIR_FWD); 
}

// Move the left motor
inline void motor_left_move (int dir, int speed) {
  analogWrite(PWM_PIN_LEFT, speed);
  digitalWrite(DIR_PIN_LEFT, dir);  
}

inline void motor_right_move (int dir, int speed) {
  analogWrite(PWM_PIN_RIGHT, speed);
  digitalWrite(DIR_PIN_RIGHT, dir);  
}

inline void motor_init() {
  pinMode(PWM_PIN_LEFT, OUTPUT);
  pinMode(PWM_PIN_RIGHT, OUTPUT);
  pinMode(DIR_PIN_LEFT, OUTPUT);
  pinMode(DIR_PIN_RIGHT, OUTPUT);

  motor_all_stop();
}

void handleMotor() {
  
  // parse request parameters
  String leftText = server.arg(0);
  String rightText = server.arg(1);
  int left_speed = leftText.toInt();
  int right_speed = rightText.toInt();

  DBG_OUTPUT_PORT.println("[" + leftText + "][" + rightText + "]");

  // compute motor direction from speed value
  motor_left_dir = ((left_speed > 0) ? DIR_FWD : DIR_BAK);
  motor_right_dir = ((right_speed > 0) ? DIR_FWD : DIR_BAK);

  // move motors
  motor_left_move (motor_left_dir, abs (left_speed));
  motor_right_move (motor_right_dir, abs (right_speed));

  // every 200 ms a new motion value
  delay(200);  
  motor_all_stop();

  // send response to client
  String message = "OK";
  server.send ( 200, "text/html", message );
}

