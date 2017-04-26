
/*
   altBot wifi motor control sketch

   Based on WiFiCar-NodeMCU project by Rudolf Reiter:
   http://www.rudiswiki.de/wiki9/WiFiCar-NodeMCU

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/



#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


#include <EEPROM.h>
#include "FS.h"

#include <crc16.h>
#include <kvstring.h>
#include <TokenProcessor.h>

// Setup config file struct with default values
/*
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
*/

#define DEFAULT_CONFIG "ssid=altBot\n" \
  "password=altBot12\n" \
  "ap_ip=192.168.4.1\n" \
  "ap_mask=255.255.255.0\n" \
  "dhcp=1\n" \
  "apmode=1\n" \
  "captivemode=1\n" \
  "hostname=altBot\n" \
  "motor_mode=1\n" \
  "update_user=admin\n" \
  "update_pass=mypass"

// Setup runtime state struct with default values
struct _state {
  int motor_left_speed = 1023;
  int motor_right_speed = 1023;
  int motor_left_dir = 1;
  int motor_right_dir = 1;
} state;

boolean captive_portal = false;

const byte DNS_PORT = 53;
DNSServer dnsServer;

const short int ESP_LED = 16;  //GPIO16

KVString kvs;
KVString defaultkvs;

// KEY_LED functions
uint8_t key_led(uint8_t level) {
  uint8_t temp;
  digitalWrite(ESP_LED, 1);
  pinMode(ESP_LED, INPUT);
  temp = digitalRead(ESP_LED);
  pinMode(ESP_LED, OUTPUT);
  digitalWrite(ESP_LED, level);
  return temp;
}


void setup() {

  // Start serial console
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  // init debug and print out ESP device info
  debug_init();
  debug_esp_info();
  debug_blink();

  // Read default config to KV
  defaultkvs.putKVs(DEFAULT_CONFIG);

  uint8_t gpio16 = key_led(1);

  // Read config from EEPROM
  if (!readEEPROMValues(Serial) || gpio16) {
    debug_print( "Loading Default settings! - ");
    if (gpio16) {
      debug_println("User key pressed");
    } else {
      debug_println("No configuration found in EEPROM");
    }
    loadDefaultKVs(Serial);
  }

  String ap_mode;
  kvs.get("apmode", ap_mode);

  // Connect to WiFi network
  if (ap_mode == "1") {
    WiFi.mode(WIFI_AP);
    WiFi.disconnect();
    delay(100);

    String ssid;
    String password;
    String ap_ip_string;
    String ap_mask_string;
    String ap_gw_string;

    kvs.get("ssid", ssid);
    kvs.get("password", password);
    kvs.get("ap_ip", ap_ip_string);
    kvs.get("ap_mask", ap_mask_string);
    IPAddress apIP, apMask;
    if (!(apIP.fromString(ap_ip_string) && apMask.fromString(ap_mask_string))) {
      Serial.println("invalid ip set in ap_ip or ap_mask");
    } else {
      WiFi.softAPConfig(apIP, apIP, apMask);
    }
    debug_print( "AP Running? ");
    debug_println(WiFi.softAP(ssid.c_str(), password.c_str()) ? "Ready" : "Failed!");
    IPAddress myIP = WiFi.softAPIP();
    debug_print ("AP IP address: ");
    debug_println (myIP);
    debug_print("MAC address = ");
    debug_println(WiFi.softAPmacAddress().c_str());

    String buffer;
    kvs.get("captivemode", buffer);
    captive_portal = (buffer == "1");
    
    if (captive_portal) {
      // Setup Captive portal:
      dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
      dnsServer.start(DNS_PORT, "*", myIP);
    }
    // ---------
  } else {

    // Station, access to router
    String ssid;
    String password;
    kvs.get("ssid", ssid);
    kvs.get("password", password);
    WiFi.begin(ssid.c_str(), password.c_str());

    // TODO: Set IPAddress

    while (WiFi.status() != WL_CONNECTED) {
      debug_blink();
      debug_print(".");
    }
    
    debug_println("");
    debug_print("\nWiFi connected with IP address ");
    debug_println(WiFi.localIP());
  }

  // Start MDNS responder
  String hostname;
  if (kvs.get("hostname", hostname)) {
    debug_print("device hostname: ");
    debug_println(hostname);
    if (MDNS.begin(hostname.c_str())) {
      MDNS.addService("http", "tcp", 80);
    }
  }

  WiFi.printDiag(Serial);

  // Initialize server and motors
  server_init();
  motor_init();
  spiffs_init();

  // show READY for use
  debug_blink();
}

void loop() {
  if (captive_portal) {
    dnsServer.processNextRequest();
  }
  server_step();
  tp_process();
  yield();
}
