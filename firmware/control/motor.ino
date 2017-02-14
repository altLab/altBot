//
// Motor control functions
//

// Human readable names for motor pins
#define PWM_PIN_LEFT D1
#define PWM_PIN_RIGHT D2
#define DIR_PIN_LEFT D3
#define DIR_PIN_RIGHT D4

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

