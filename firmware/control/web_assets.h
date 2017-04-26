#ifndef ALTBOT_WEB_ASSETS_H
#define ALTBOT_WEB_ASSETS_H

//
// This file defines static copies of altBot web server asset files
// to be embedded inside the firmware
//

// Main control screen
static const String server_root_assets = R"+++++(
<!DOCTYPE html>
<html>
<head>
  <title>altBot Web Control</title>
</head>
<body>
  <div id="block" style="font-size:24pt">
    <a href="#" onclick='switch_motion();'>switch motion</a><BR/>
    <a href='#' onclick='move("f");'>forward</a><BR/>
    <a href='#' onclick='move("b");'>backwards</a><BR/>
    <a href='#' onclick='move("l");'>left</a><BR/>
    <a href='#' onclick='move("r");'>right</a><BR/>
    <p id="dmEvent">Accelerometer</p>
    <div id="vector">Direction Vector</div>
  </div>
<script type='text/javascript'>
  var lastMove = 0;
  var motion_enabled = true;

  function switch_motion() {
    motion_enabled = !motion_enabled;
    if (motion_enabled) {
      document.getElementById("dmEvent").innerHTML = "Accelerometer OK";
    } else {
      document.getElementById("dmEvent").innerHTML = "Accelerometer disabled";
    }
    return false;
  }

  function move_car(left, right) {
    var now = Date.now();
    if (lastMove + 200 < now) {  // orig. 200 ms
      lastMove = now;
      var request = new XMLHttpRequest();
      // if direction is opposite, change sign of +left and +right
      request.open("GET", "/engines?left=" + Math.round(-left) + "&right=" + Math.round(-right), true);
      request.send(null);
    }
  }

  function move(dir) {
    if (dir === "f"){ move_car(-1000, -1000);}
    if (dir === "b"){ move_car(1000, 1000);}
    if (dir === "l"){ move_car(-1000, 1000);}
    if (dir === "r"){ move_car(1000, -1000);}
    return false; // no further processing
  }

  if (window.DeviceMotionEvent) {
    window.addEventListener("devicemotion", deviceMotionHandler, false);
    document.getElementById("dmEvent").innerHTML = "Accelerometer OK";
  } else {
    document.getElementById("dmEvent").innerHTML = "Accelerometer not supported.";
  }

  function deviceMotionHandler(eventData) {
    if (motion_enabled) {
    var acceleration = eventData.accelerationIncludingGravity;
    var left = 0;
    var right = 0;
    if (Math.abs(acceleration.y) > 1) { // back-/forward
      var speed1 = acceleration.y * 100;
      if (acceleration.y > 0) { // add 300 to decrease dead zone
        left = Math.min(1023, speed1 + acceleration.x * 40 + 300);
        right = Math.min(1023, speed1 - acceleration.x * 40 + 300);
      } else {
        left = Math.max(-1023, speed1 + acceleration.x * 40 - 300);
        right = Math.max(-1023, speed1 - acceleration.x * 40 - 300);
      }
    } else if (Math.abs(acceleration.x) > 1) { // circle only
      var speed2 = Math.min(1023, Math.abs(acceleration.x) * 100);
      if (acceleration.x > 0) {
        left = Math.min(1023, speed2 + 300);
        right = Math.max(-1023, -speed2 - 300);
      } else {
        left = Math.max(-1023, -speed2 - 300);
        right = Math.min(1023, speed2 + 300);
      }
    }

    if (Math.abs(left) > 200 || Math.abs(right) > 200) { // orig. 100,100
      move_car(left, right);
    }
    var direction = "stop";
    // if direction is opposite, change sign of +left and +right
    var accX = Math.round(acceleration.x);
    var accY = Math.round(acceleration.y);
    var accZ = Math.round(acceleration.z);
    var leftD = Math.round(-left);
    var rightD = Math.round(-right);

    direction = "[" + accX + "," + accY + "," + accZ  + "]<BR/>" + leftD + ", " + rightD;
    document.getElementById("vector").innerHTML = direction;
    }
  }
</script>
</body>
</html>
)+++++";

#endif // ALTBOT_WEB_ASSETS_H
