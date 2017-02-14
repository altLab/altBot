
/*
 * altBot wifi motor control sketch
 * 
 * Based on WiFiCar-NodeMCU project by Rudolf Reiter:
 * http://www.rudiswiki.de/wiki9/WiFiCar-NodeMCU
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#define ACCESSPOINT

#define DBG_OUTPUT_PORT Serial

struct strConfig {
  String ssid;
  String password;
  byte  IP[4];
  byte  Netmask[4];
  byte  Gateway[4];
  boolean dhcp;
  String DeviceName;
  byte motorMode;
  uint16_t motorSpeed;
} config;

#ifdef ACCESSPOINT
const char* ssid = "wifi-car";
const char* password = "";
const char* host = "esp8266fs";

#else
// Station Access mode, adopt strings to your WiFi router
const char* ssid = ".......";
const char* password = ".......";
#endif

const short int ESP_LED = 16;  //GPIO16

// Access Point mode for car use, Station Access mode for software development.
// comment next line for Station Access to WiFi router

int motorASpeed = 1023;
int motorBSpeed = 1023;
int motorAForward = 1;
int motorBForward = 1;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

//------------------- WiFi Car application -----------
void handleRoot() {
  String message = R"------(
<!DOCTYPE html><html><head>
</head><body>
<div id="block" style="font-size:24pt">
<a href='#' onclick='move("f");'>forward</a><BR/>
<a href='#' onclick='move("b");'>backwards</a><BR/>
<a href='#' onclick='move("l");'>left</a><BR/>
<a href='#' onclick='move("r");'>right</a><BR/>
<p id="dmEvent">Acc</p>
<div id="vector"</div>
</div>
<script type='text/javascript' src='/wifi-carAP.js'></script>
</body></html>
)------";

  server.send ( 200, "text/html", message );

  // TODO: WTF?
  analogWrite(5, 0);
  analogWrite(4, 0);
  digitalWrite(0, 1);
  digitalWrite(2, 1);
}




//------------------ setup ---------------
void setup() {
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  /// LED_user setup and test, for debug purpose.
  DumpESPinfo();
  pinMode(ESP_LED, OUTPUT);
  digitalWrite(ESP_LED, LOW);
  
  delay(100); // ms
  digitalWrite(ESP_LED, HIGH);
  delay(300); // ms, pause because of AP mode

  // Connect to WiFi network
#ifdef ACCESSPOINT
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  DBG_OUTPUT_PORT.print("AP IP address: ");
  DBG_OUTPUT_PORT.println(myIP);
#else
  // Station, access to router
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.println("WiFi connected");
  // Print the IP address
  DBG_OUTPUT_PORT.println(WiFi.localIP());
#endif

  MDNS.begin(host);

  //---------------- Server init
  
  server.on ( "/", handleRoot );
  server.on ( "/engines", handleMotor );
 
  //--------------- Start the server
  server.begin();
  DBG_OUTPUT_PORT.println("Server started");

  initMotors();
  
  
  // show READY for use
  digitalWrite(ESP_LED, LOW);
  delay(300); // ms
  digitalWrite(ESP_LED, HIGH);
}

void loop() {
  server.handleClient();
  delay(1);
}
