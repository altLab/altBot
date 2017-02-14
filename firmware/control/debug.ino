//
// Device debugging info
//

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

