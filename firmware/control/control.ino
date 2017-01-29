
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

String GetMacAddress() {
  uint8_t mac[6];
    char macStr[18] = {0};
  WiFi.macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
    return  String(macStr);
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c) {
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

String urldecode(String input) { // (based on https://code.google.com/p/avr-netino/)
   char c;
   String ret = "";
   
   for(byte t=0;t<input.length();t++) {
     c = input[t];
     if (c == '+') c = ' ';
     if (c == '%') {
        t++;
        c = input[t];
        t++;
        c = (h2int(c) << 4) | h2int(input[t]);
     }
     ret.concat(c);
   }
   return ret;
}

void WriteConfig() {
  DBG_OUTPUT_PORT.println("Writing Config");
  EEPROM.write(0,'a');
  EEPROM.write(1,'l');
  EEPROM.write(2,'t');
  EEPROM.write(3,'b');
  EEPROM.write(4,'o');
  EEPROM.write(5,'t');
  EEPROM.write(6,'C');
  EEPROM.write(7,'F');
  EEPROM.write(8,'G');

  EEPROM.write(16,config.dhcp);

  EEPROM.write(32,config.IP[0]);
  EEPROM.write(33,config.IP[1]);
  EEPROM.write(34,config.IP[2]);
  EEPROM.write(35,config.IP[3]);

  EEPROM.write(36,config.Netmask[0]);
  EEPROM.write(37,config.Netmask[1]);
  EEPROM.write(38,config.Netmask[2]);
  EEPROM.write(39,config.Netmask[3]);

  EEPROM.write(40,config.Gateway[0]);
  EEPROM.write(41,config.Gateway[1]);
  EEPROM.write(42,config.Gateway[2]);
  EEPROM.write(43,config.Gateway[3]);


  WriteStringToEEPROM(64,config.ssid);
  WriteStringToEEPROM(96,config.password);
  WriteStringToEEPROM(128,config.DeviceName);

  EEPROM.commit();
}

boolean ReadConfig() {
  DBG_OUTPUT_PORT.println("Reading Configuration");
  if (EEPROM.read(0) == 'a' && EEPROM.read(1) == 'l'  && EEPROM.read(2) == 't' && 
      EEPROM.read(3) == 'b' && EEPROM.read(4) == 'o'  && EEPROM.read(5) == 't' && 
      EEPROM.read(6) == 'C' && EEPROM.read(7) == 'F'  && EEPROM.read(8) == 'G' ) {
    Serial.println("Configurarion Found!");
    config.dhcp =   EEPROM.read(16);

    config.IP[0] = EEPROM.read(32);
    config.IP[1] = EEPROM.read(33);
    config.IP[2] = EEPROM.read(34);
    config.IP[3] = EEPROM.read(35);
    config.Netmask[0] = EEPROM.read(36);
    config.Netmask[1] = EEPROM.read(37);
    config.Netmask[2] = EEPROM.read(38);
    config.Netmask[3] = EEPROM.read(39);
    config.Gateway[0] = EEPROM.read(40);
    config.Gateway[1] = EEPROM.read(41);
    config.Gateway[2] = EEPROM.read(42);
    config.Gateway[3] = EEPROM.read(43);
    config.ssid = ReadStringFromEEPROM(64);
    config.password = ReadStringFromEEPROM(96);
    config.DeviceName = ReadStringFromEEPROM(128);
    return true;
  } else {
    Serial.println("Configurarion NOT FOUND!!!!");
    return false;
  }
}

// https://github.com/NicHub/ouilogique-ESP8266-Arduino/blob/master/get-esp8266-info/get-esp8266-info.ino 
 
struct rst_info
{
  uint32 reason;
  uint32 exccause;
  uint32 epc1;
  uint32 epc2;
  uint32 epc3;
  uint32 excvaddr;
  uint32 depc;
};

void DumpESPinfo() {

  DBG_OUTPUT_PORT.printf( "\n\n\nESP8266 INFORMATION\n===================\n" );

  //ESP.getVcc() ⇒ may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. ⇒ https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#user-content-esp-specific-apis
  DBG_OUTPUT_PORT.printf( "ESP.getFreeHeap()              : %d\n",   ESP.getFreeHeap() );   //  returns the free heap size.
  DBG_OUTPUT_PORT.printf( "ESP.getChipId()                : 0x%X\n", ESP.getChipId() );   //  returns the ESP8266 chip ID as a 32-bit integer.
  DBG_OUTPUT_PORT.printf( "ESP.getSdkVersion()            : %d\n",   ESP.getSdkVersion() );
  DBG_OUTPUT_PORT.printf( "ESP.getBootVersion()           : %d\n",   ESP.getBootVersion() );
  DBG_OUTPUT_PORT.printf( "ESP.getBootMode()              : %d\n",   ESP.getBootMode() );
  DBG_OUTPUT_PORT.printf( "ESP.getCpuFreqMHz()            : %d\n",   ESP.getCpuFreqMHz() );
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipId()           : 0x%X\n", ESP.getFlashChipId() );
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipRealSize()     : %d\n",   ESP.getFlashChipRealSize() );
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipSize()         : %d\n",   ESP.getFlashChipSize() );  //returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipSpeed()        : %d\n",   ESP.getFlashChipSpeed() ); // returns the flash chip frequency, in Hz.
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipMode()         : %d\n",   ESP.getFlashChipMode() );
  DBG_OUTPUT_PORT.printf( "ESP.getFlashChipSizeByChipId() : 0x%X\n", ESP.getFlashChipSizeByChipId() );
  DBG_OUTPUT_PORT.printf( "ESP.getSketchSize()            : %d\n",   ESP.getSketchSize() );
  DBG_OUTPUT_PORT.printf( "ESP.getFreeSketchSpace()       : %d\n",   ESP.getFreeSketchSpace() );
  DBG_OUTPUT_PORT.printf( "ESP.getCycleCount()            : %d\n",   ESP.getCycleCount() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.
  DBG_OUTPUT_PORT.printf( "macAddress()                   : %s\n",   GetMacAddress().c_str() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.

  rst_info *xyz;
  DBG_OUTPUT_PORT.printf( "ESP.getResetInfoPtr()\n" );
  xyz = ESP.getResetInfoPtr();
  
  DBG_OUTPUT_PORT.println( ( *xyz ).reason );
  DBG_OUTPUT_PORT.println( ( *xyz ).exccause );
  DBG_OUTPUT_PORT.println( ( *xyz ).epc1 );
  DBG_OUTPUT_PORT.println( ( *xyz ).epc2 );
  DBG_OUTPUT_PORT.println( ( *xyz ).epc3 );
  DBG_OUTPUT_PORT.println( ( *xyz ).excvaddr );
  DBG_OUTPUT_PORT.println( ( *xyz ).depc );
}

/*
void startWIFI(void) {
    //set IP if not correct
    IPAddress ip = WiFi.localIP();
    if( ip!= ipadd) {
        WiFi.config(ipadd, ipgat, ipsub); //dsa added 12.04.2015
        Serial.println();
        delay(10);
        Serial.print("ESP8266 IP:");
        delay(10);
        Serial.println(ip);
        delay(10);
        Serial.print("Fixed IP:");
        delay(10);
        Serial.println(ipadd);
        delay(10);
        Serial.print("IP now set to: ");
        delay(10);
        Serial.println(WiFi.localIP());
        delay(10);
    }
    // Connect to WiFi network
    Serial.println();
    delay(10);
    Serial.println();
    delay(10);
    Serial.print("Connecting to ");
    delay(10);
    Serial.println(ssid);
    delay(10);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.print("ESP8266 IP: ");
    Serial.println(WiFi.localIP());

    // Print the server port
    Serial.print("ESP8266 WebServer Port: ");
    Serial.println(SVRPORT);
    delay(300);
}
*/

void WriteStringToEEPROM(int beginaddress, String string) {
  char charBuf[string.length()+1];
  string.toCharArray(charBuf, string.length()+1);
  for (int t=  0; t<sizeof(charBuf);t++) {
    EEPROM.write(beginaddress + t,charBuf[t]);
  }
}
String  ReadStringFromEEPROM(int beginaddress) {
  byte counter=0;
  char rChar;
  String retString = "";
  while (1) {
    rChar = EEPROM.read(beginaddress + counter);
    if (rChar == 0) break;
    if (counter > 31) break;
    counter++;
    retString.concat(rChar);
  }
  return retString;
}
void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address) {
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

#ifdef ACCESSPOINT
const char* ssid = "altBotAP";
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

//-------------- FSBrowser application -----------
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
//if(filename.endsWith(".src")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String sanityzePath(String path) {
  if (!path.startsWith("/"))
    path = "/" + path;
  return path;
}

void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");

  String path = sanityzePath(server.arg(0));
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);

  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");

  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");

  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");

  String path = sanityzePath(server.arg(0));
  
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String(); // TODO: WTF ?
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

// An empty ESP8266 Flash ROM must be formatted before using it, actual a problem
void handleFormat() {
  DBG_OUTPUT_PORT.println("Format SPIFFS");
  if (SPIFFS.format()) {
    if (!SPIFFS.begin()) {
      DBG_OUTPUT_PORT.println("Format SPIFFS failed");
    }
  } else {
    DBG_OUTPUT_PORT.println("Format SPIFFS failed");
  }
  if (!SPIFFS.begin()) {
    DBG_OUTPUT_PORT.println("SPIFFS failed, needs formatting");
  } else {
    DBG_OUTPUT_PORT.println("SPIFFS mounted");
  }
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String(); // WTF?

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);

  if (path.endsWith("/")) 
    return handleFileRead(path + "index.html");

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    size_t contentLength = file.size();
    file.close();
    return true;
  }
  return false;
}

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
<script type='text/javascript' src='/script.js'></script>
</body></html>
)------";

  server.send ( 200, "text/html", message );

  // TODO: WTF?
  analogWrite(5, 0);
  analogWrite(4, 0);
  digitalWrite(0, 1);
  digitalWrite(2, 1);
}


void handleJavascript() {
  String message = R"------(
var lastMove = 0;
var version = 8;
document.getElementById("vector").innerHTML ="vector1";
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
  //document.getElementById("vector").innerHTML ="vector2";
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
  //document.getElementById("block").style.fontSize = "x-large";
  //getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
}
)------";

  server.send ( 200, "text/html", message );

}



void error404() {


}

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
      server.send(404, "text/plain", "File Not Found");
  });

  server.on ( "/", handleRoot );
  server.on ( "/script.js", handleJavascript );
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
