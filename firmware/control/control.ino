
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

  // Start serial console
  Serial.begin(115200);
  
  // init debug and print out ESP device info
  debug_init();
  debug_esp_info();
  debug_blink();

  // TODO: Read config from EEPROM

  // Connect to WiFi network
  if (config.ap_mode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.ssid.c_str(), config.password.c_str());
    IPAddress myIP = WiFi.softAPIP();
    debug_print ("AP IP address: ");
    debug_println (myIP);
  } else {
    // Station, access to router
    WiFi.begin(config.ssid.c_str(), config.password.c_str());
  
    while (WiFi.status() != WL_CONNECTED) {
      debug_blink();
      debug_print(".");
    }
    
    debug_println("");
    debug_print("WiFi connected with IP address ");
    debug_println(WiFi.localIP());
  }

  // Start MDNS responder
  MDNS.begin(config.device_id.c_str());

  // Initialize server and motors
  server_init();
  motor_init();
  
  // show READY for use
  debug_blink();
}

void loop() {
  server_step();
  yield();
}
