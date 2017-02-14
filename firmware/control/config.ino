//
// Device Configuration
//

boolean ReadConfig() {
  debug_println("Reading Configuration");
  if (EEPROM.read(0) == 'a' && EEPROM.read(1) == 'l'  && EEPROM.read(2) == 't' && 
      EEPROM.read(3) == 'b' && EEPROM.read(4) == 'o'  && EEPROM.read(5) == 't' && 
      EEPROM.read(6) == 'C' && EEPROM.read(7) == 'F'  && EEPROM.read(8) == 'G' ) {
    Serial.println("Configurarion Found!");
    config.dhcp =   EEPROM.read(16);

    config.ip[0] = EEPROM.read(32);
    config.ip[1] = EEPROM.read(33);
    config.ip[2] = EEPROM.read(34);
    config.ip[3] = EEPROM.read(35);
    config.netmask[0] = EEPROM.read(36);
    config.netmask[1] = EEPROM.read(37);
    config.netmask[2] = EEPROM.read(38);
    config.netmask[3] = EEPROM.read(39);
    config.gateway[0] = EEPROM.read(40);
    config.gateway[1] = EEPROM.read(41);
    config.gateway[2] = EEPROM.read(42);
    config.gateway[3] = EEPROM.read(43);
    config.ssid = ReadStringFromEEPROM(64);
    config.password = ReadStringFromEEPROM(96);
    config.device_id = ReadStringFromEEPROM(128);
    return true;
  } else {
    Serial.println("Configurarion NOT FOUND!!!!");
    return false;
  }
}

void WriteConfig() {
  debug_println("Writing Config");
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

  EEPROM.write(32,config.ip[0]);
  EEPROM.write(33,config.ip[1]);
  EEPROM.write(34,config.ip[2]);
  EEPROM.write(35,config.ip[3]);

  EEPROM.write(36,config.netmask[0]);
  EEPROM.write(37,config.netmask[1]);
  EEPROM.write(38,config.netmask[2]);
  EEPROM.write(39,config.netmask[3]);

  EEPROM.write(40,config.gateway[0]);
  EEPROM.write(41,config.gateway[1]);
  EEPROM.write(42,config.gateway[2]);
  EEPROM.write(43,config.gateway[3]);

  WriteStringToEEPROM(64,config.ssid);
  WriteStringToEEPROM(96,config.password);
  WriteStringToEEPROM(128,config.device_id);

  EEPROM.commit();
}
