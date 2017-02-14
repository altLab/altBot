void initMotors() {
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);

  digitalWrite(5, 0);
  digitalWrite(4, 0);

  digitalWrite(0, 1);
  digitalWrite(2, 1);
}

void handleMotor() {
  String leftText = server.arg(0);
  String rightText = server.arg(1);
  DBG_OUTPUT_PORT.println("[" + leftText + "][" + rightText + "]");
  int left = leftText.toInt();
  int right = rightText.toInt();
  if (left < 0) {
    motorAForward = 0;
  } else {
    motorAForward = 1;
  }
  if (right < 0) {
    motorBForward = 0;
  } else {
    motorBForward = 1;
  }
  
  analogWrite(5, abs(left));      // was left
  analogWrite(4, abs(right));     // was right
  digitalWrite(0, motorBForward); // was A
  digitalWrite(2, motorAForward); // was B

  delay(200);  // every 200 ms a new motion value
  analogWrite(5, 0);
  analogWrite(4, 0);
  digitalWrite(0, 1);
  digitalWrite(2, 1);

  String message = "OK";
  server.send ( 200, "text/html", message );
}

