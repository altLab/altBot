
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

#define DBG_OUTPUT_PORT Serial

// Setup config file struct with default values 
struct _config {
  boolean ap_mode = true;
  String ssid = "altBot";
  String password = "altBot";
  byte  ip[4];
  byte  netmask[4];
  byte  gateway[4];
  boolean dhcp;
  String device_id = "altBot";
} config;

// Setup runtime state struct with default values
struct _state {
  int motor_left_speed = 1023;
  int motor_right_speed = 1023;
  int motor_left_dir = 1;
  int motor_right_dir = 1;
} state;

const short int ESP_LED = 16;  //GPIO16

void setup() {

  // Start debug console
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

  // TODO: Read config from EEPROM

  // Connect to WiFi network
 
  if (config.ap_mode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.ssid.c_str(), config.password.c_str());
    IPAddress myIP = WiFi.softAPIP();
    DBG_OUTPUT_PORT.print("AP IP address: ");
    DBG_OUTPUT_PORT.println(myIP);
  } else {
    // Station, access to router
    WiFi.begin(config.ssid.c_str(), config.password.c_str());
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      DBG_OUTPUT_PORT.print(".");
    }
    DBG_OUTPUT_PORT.println("");
    DBG_OUTPUT_PORT.println("WiFi connected");
    // Print the IP address
    DBG_OUTPUT_PORT.println(WiFi.localIP());
  }

  MDNS.begin(config.device_id.c_str());

  // Initialize server and motors
  server_init();
  motor_init();
  
  // show READY for use
  digitalWrite(ESP_LED, LOW);
  delay(300); // ms
  digitalWrite(ESP_LED, HIGH);
}

void loop() {
  server_step();
  yield();
}
