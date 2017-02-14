//
// Device debugging functions
//

// Comment this definition to compile out debugging info
#define ALTBOT_DEBUG

#ifdef ALTBOT_DEBUG

// Initialize debugging info
inline void debug_init() {
  Serial.print("\n");
  Serial.setDebugOutput(true);
  pinMode(ESP_LED, OUTPUT);
}

inline void debug_print (const String &msg) {
  Serial.print (msg);
}

inline void debug_println (const String &msg) {
  Serial.println (msg);
}

inline void debug_println (const Printable &msg) {
  Serial.println (msg);
}

inline void debug_printf () {
  
}

inline void debug_blink() {
  digitalWrite(ESP_LED, LOW);
  delay(100);
  digitalWrite(ESP_LED, HIGH);
  delay(400);
}

// Print out internal ESP device info
// Adapted from: https://github.com/NicHub/ouilogique-ESP8266-Arduino/blob/master/get-esp8266-info/get-esp8266-info.ino 
 
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

void debug_esp_info() {

  Serial.printf( "\n\n\nESP8266 INFORMATION\n===================\n" );

  //ESP.getVcc() ⇒ may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. ⇒ https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#user-content-esp-specific-apis
  Serial.printf( "Free Heap Memory               : %d\n",   ESP.getFreeHeap() );   //  returns the free heap size.
  Serial.printf( "Chip ID                        : 0x%X\n", ESP.getChipId() );   //  returns the ESP8266 chip ID as a 32-bit integer.
  Serial.printf( "SDK Version                    : %d\n",   ESP.getSdkVersion() );
  Serial.printf( "Boot Version                   : %d\n",   ESP.getBootVersion() );
  Serial.printf( "Boot Mode                      : %d\n",   ESP.getBootMode() );
  Serial.printf( "CPU Frequency (Mhz)            : %d\n",   ESP.getCpuFreqMHz() );
  Serial.printf( "Flash Chip ID                  : 0x%X\n", ESP.getFlashChipId() );
  Serial.printf( "Flash Chip Real Size           : %d\n",   ESP.getFlashChipRealSize() );
  Serial.printf( "Flash Chip Size                : %d\n",   ESP.getFlashChipSize() );  //returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  Serial.printf( "Flash Chip Speed               : %d\n",   ESP.getFlashChipSpeed() ); // returns the flash chip frequency, in Hz.
  Serial.printf( "Flash Chip Mode                : %d\n",   ESP.getFlashChipMode() );
  Serial.printf( "Flash Chip Size (by Chip ID)   : 0x%X\n", ESP.getFlashChipSizeByChipId() );
  Serial.printf( "Sketch Size                    : %d\n",   ESP.getSketchSize() );
  Serial.printf( "Free Sketch Space              : %d\n",   ESP.getFreeSketchSpace() );
  Serial.printf( "Cycle Count                    : %d\n",   ESP.getCycleCount() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.
  Serial.printf( "MAC Address                    : %s\n",   debug_get_mac_addr().c_str() ); // returns the cpu instruction cycle count since start as an unsigned 32-bit. This is useful for accurate timing of very short actions like bit banging.

  rst_info *xyz;
  Serial.printf( "Reset Info Pointer\n" );
  xyz = ESP.getResetInfoPtr();
  
  Serial.println( ( *xyz ).reason );
  Serial.println( ( *xyz ).exccause );
  Serial.println( ( *xyz ).epc1 );
  Serial.println( ( *xyz ).epc2 );
  Serial.println( ( *xyz ).epc3 );
  Serial.println( ( *xyz ).excvaddr );
  Serial.println( ( *xyz ).depc );
}

// Retrieve MAC address as a String value
String debug_get_mac_addr() {
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
  return  String(macStr);
}

#endif // ALTBOT_DEBUG
