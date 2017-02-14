
/*
 * This sketch let you control an NodeMCU motorshield with attached motors (wifi car),
 * from a smart phone browser by using the orientation of the smart phone.
 * See at the following link, or other vendors.
 * http://www.smartarduino.com/2wd-wifi-rc-smart-car-with-nodemcu-shield-for-esp-12e_p94572.html
 *
 * Program: wifi-car-AP.ino, 2015-11-19 RR
 *  100 ms cycle time are too short - 200 ms aere O2
 * 2015-11-27 - fix AP mode, Rudolf Reuter
 * 
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
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#include "FS.h"

#define ACCESSPOINT

#define DBG_OUTPUT_PORT Serial


// /mode/8/o
// /digital/8/1
// /analog/6/123
// /id

/*
 * Servidor:
 * Paginas HTML
 * Comandos:
 *  left
 *  right
 *  forward
 *  backward
 *  mode
 * Paginas JSON:
 *  informacao
 * Upload
 *  manipulacao do FS
 *  
 * Settings: 
 *  Modo AP ou Host
 *  SSID
 *  Pass
 *  
 * Estatico:
 *  IP / Mask / Gateway
 *  
 * Hostname:
 * 
 * 
 */



#define AdminTimeOut 180 // Defines the time in seconds, when the admin mode will be diabled 

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

const short int BUILTIN_LED2 = 16;  //GPIO16
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

// Access Point mode for car use, Station Access mode for software development.
// comment next line for Station Access to WiFi router

int motorASpeed = 1023;
int motorBSpeed = 1023;
int motorAForward = 1;
int motorBForward = 1;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

//holds the current upload
File fsUploadFile;
String fileName;

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

void error404() {


}


//------------------ setup ---------------
void setup() {
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  /// LED_user setup and test, for debug purpose.
  DumpESPinfo();
  pinMode(BUILTIN_LED2, OUTPUT);
  digitalWrite(BUILTIN_LED2, LOW);
  
  delay(100); // ms
  digitalWrite(BUILTIN_LED2, HIGH);
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
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");



  //---------------- Server init
  // list directory
  server.on("/list", HTTP_GET, handleFileList);
  
  // create file
  server.on("/create", handleFileCreate);
  //delete file
  server.on("/delete", HTTP_GET, handleFileDelete);
  // called after file upload
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
  
  // Format Flash ROM - too dangerous!
  // server.on ( "/format", handleFormat );
  
  // get filename from client for upload; setup is also handle
  server.on("/upload", HTTP_GET, []() {
	server.sendHeader("Connection", "close");
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "text/html", serverIndex);
  });

  server.onFileUpload([]() {  // called several times from Parsing.cpp while upload
    if (server.uri() != "/update") return;
    // get file
    HTTPUpload& upload = server.upload();
    DBG_OUTPUT_PORT.print("status: "); 
    DBG_OUTPUT_PORT.println( (int)upload.status);  // need 2 commands to work!
    if (upload.status == UPLOAD_FILE_START) {
      fileName = upload.filename;
      DBG_OUTPUT_PORT.println("Upload Name: " + fileName);
      String path = "/" + fileName;
      fsUploadFile = SPIFFS.open(path, "w");
      // already existing file will be overwritten!
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (fsUploadFile)
        fsUploadFile.write(upload.buf, upload.currentSize);
      DBG_OUTPUT_PORT.println(fileName + " size: " + upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      DBG_OUTPUT_PORT.print("Upload Size: ");
      DBG_OUTPUT_PORT.println(upload.totalSize);  // need 2 commands to work!
      if (fsUploadFile)
        fsUploadFile.close();
    }
    yield();
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    // ESP.restart();  // not needed
    DBG_OUTPUT_PORT.println("dir SPIFFS");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
  });

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.on ( "/", handleRoot );
//  server.on ( "/index.html", handleRoot );
  server.on ( "/engines", handleMotor );

  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  
  server.on("/pin", HTTP_GET, [](){
    String number_string=server.arg("num");
    String value_string=server.arg("val");
    DBG_OUTPUT_PORT.println("Set Pin: " + number_string + " to " + value_string);
    int number = number_string.toInt();
    int value = value_string.toInt();
    if (number>0) {
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);                       // wait for a second
//        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);                       // wait for a second

//      pinMode(number,OUTPUT);
      digitalWrite(number,value);
      server.send(200, "text/plain", "Pin " + number_string + " is now " + value_string);
    }
    });
  
  //--------------- Start the server
  server.begin();
  DBG_OUTPUT_PORT.println("Server started");

  initMotors();
  
  // show the SPIFFS contents, name and size
  if (!SPIFFS.begin()) {
    DBG_OUTPUT_PORT.println("SPIFFS failed, needs formatting");
  } else {
    DBG_OUTPUT_PORT.println("dir SPIFFS");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.println();
    FSInfo fs_info;
    if (!SPIFFS.info(fs_info)) {
      DBG_OUTPUT_PORT.println("fs_info failed");
    } else {
      DBG_OUTPUT_PORT.printf("Total: %u, Used: %u, Block: %u, Page: %u, Max open files: %u, Max path len: %u\n",
                    fs_info.totalBytes,
                    fs_info.usedBytes,
                    fs_info.blockSize,
                    fs_info.pageSize,
                    fs_info.maxOpenFiles,
                    fs_info.maxPathLength
                   );
    }
  }
  // show READY for use
  digitalWrite(BUILTIN_LED2, LOW);
  delay(300); // ms
  digitalWrite(BUILTIN_LED2, HIGH);
}

void loop() {
  server.handleClient();
  delay(1);
}
