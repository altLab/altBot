//
// Web server functions 
//

// Create an instance of the server
ESP8266WebServer server(80);

// Statically embed the root HTML and JS assets inside the firmware
static const String server_root_assets = R"+++++(
<!DOCTYPE html>
<html>
<head>
  <title>altBot Web Control</title>
</head>
<body>
  <div id="block" style="font-size:24pt">
    <a href='#' onclick='move("f");'>forward</a><BR/>
    <a href='#' onclick='move("b");'>backwards</a><BR/>
    <a href='#' onclick='move("l");'>left</a><BR/>
    <a href='#' onclick='move("r");'>right</a><BR/>
    <p id="dmEvent">Accelerometer</p>
    <div id="vector">Direction Vector</div>
  </div>
<script type='text/javascript'>
  var lastMove = 0;
  var version = 8;

  function move_car(left, right) {
    var now = Date.now();
    if (lastMove + 200 < now) {  // orig. 200 ms
      lastMove = now; 
      var request = new XMLHttpRequest();
      // if direction is opposite, change sign of +left and +right
      request.open('GET', '/engines?left=' + Math.round(-left) + "&right=" + Math.round(-right), true);
      request.send(null);
    }
  }

  function move(dir) {
    //var e = event.keyCode;
    if (dir=='f'){ move_car(-1000, -1000);}
    if (dir=='b'){ move_car(1000, 1000);}
    if (dir=='l'){ move_car(-1000, 1000);}
    if (dir=='r'){ move_car(1000, -1000);}
  }

  if (window.DeviceMotionEvent) {
    window.addEventListener('devicemotion', deviceMotionHandler, false);
    document.getElementById("dmEvent").innerHTML = "Accelerometer OK";
  } else {
    document.getElementById("dmEvent").innerHTML = "Accelerometer not supported.";
  }

  function deviceMotionHandler(eventData) {
    acceleration = eventData.accelerationIncludingGravity;
    var left = 0;
    var right = 0;
    if (Math.abs(acceleration.y) > 1) { // back-/forward
      var speed = acceleration.y * 100;
      if (acceleration.y > 0) { // add 300 to decrease dead zone
        left = Math.min(1023, speed + acceleration.x * 40 + 300);
        right = Math.min(1023, speed - acceleration.x * 40 + 300);
      } else {
        left = Math.max(-1023, speed + acceleration.x * 40 - 300);
        right = Math.max(-1023, speed - acceleration.x * 40 - 300);
      }
    } else if (Math.abs(acceleration.x) > 1) { // circle only
      var speed = Math.min(1023, Math.abs(acceleration.x) * 100);
      if (acceleration.x > 0) {
        left = Math.min(1023, speed + 300);
        right = Math.max(-1023, -speed - 300); 
      } else {
        left = Math.max(-1023, -speed - 300);  
        right = Math.min(1023, speed + 300);
      }
    }

    if (Math.abs(left) > 200 || Math.abs(right) > 200) { // orig. 100,100
      move_car(left, right);
    }
    var direction = "stop";
    // if direction is opposite, change sign of +left and +right
    var acc_x = Math.round(acceleration.x);
    var acc_y = Math.round(acceleration.y);
    var acc_z = Math.round(acceleration.z);
    var leftD = Math.round(-left);
    var rightD = Math.round(-right);

    direction = "[" + acc_x + "," + acc_y + "," + acc_z  + "]<BR/>" + leftD + ", " + rightD + "<BR/>version: " + version; 
    document.getElementById("vector").innerHTML =direction;
  }
</script>
</body>
</html>
)+++++";

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
