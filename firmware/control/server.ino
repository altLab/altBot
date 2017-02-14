//
// Web server functions 
//

#include "web_assets.h"

// Create an instance of the server
ESP8266WebServer server(80);

// Initialize the server
inline void server_init() {
  server.on ( "/", server_root_handler);
  server.on ( "/engines", server_motor_handler);
  server.onNotFound (server_handle_404);
  
  server.begin();
  
  debug_println("Server started");
}

// Check for requests on the Web API
inline void server_step() {
  server.handleClient();
}

// Handle Web API request for root page
void server_root_handler() {
  server.send(200, "text/html", server_root_assets);
  
  // stop all motors when user reloads index page
  motor_all_stop();
}


// Handle Web API requests for motor control
void server_motor_handler() {
  
  // parse request parameters
  String leftText = server.arg(0);
  String rightText = server.arg(1);
  int left_speed = leftText.toInt();
  int right_speed = rightText.toInt();

  debug_println ("Motor Handler: [" + leftText + "][" + rightText + "]");

  // compute motor direction from speed value
  state.motor_left_dir = ((left_speed > 0) ? DIR_FWD : DIR_BAK);
  state.motor_right_dir = ((right_speed > 0) ? DIR_FWD : DIR_BAK);

  // move motors
  motor_left_move (state.motor_left_dir, abs (left_speed));
  motor_right_move (state.motor_right_dir, abs (right_speed));

  // every 200 ms a new motion value
  delay(200);  
  motor_all_stop();

  // send response to client
  String message = "OK";
  server.send ( 200, "text/html", message );
}

// Handle requests for non-existent URLs
void server_handle_404() {
  // use a temporary redirect because we might use the URL someday
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}
