

// EEPROM Code:

const char *config_signature = "altBot10";

/*
EEPROM Contents:
0..7   altBot10
8..9   SizeOfConfig (excludes CRC)
10..xx  KEY=VALUE\n
...
yy..yy+1   CRC16

*/

const char commands[] = {
 'h',
 'g',
 's',
 'l',
 'x',
 'w',
 'r',
 'b',
 'z'
};

void getHelp(TokenProcessor *cps);
void getValue(TokenProcessor *cps);
void setValue(TokenProcessor *cps);
void listValues(TokenProcessor *cps);
void delValue(TokenProcessor *cps);
void writeValues(TokenProcessor *cps);
void readValues(TokenProcessor *cps);
void reboot(TokenProcessor *cps);
void loadDefaults(TokenProcessor *cps);

void unknownCommand(TokenProcessor *cps);

typedef void(*callback_t)(TokenProcessor *);

callback_t commandFunctions[] = {
    &getHelp,
    &getValue,
    &setValue, 
    &listValues, 
    &delValue, 
    &writeValues, 
    &readValues, 
    &reboot,
    &loadDefaults
};

const char helpStrings_h[] PROGMEM = "- help about commands";
const char helpStrings_g[] PROGMEM = "[KEY] - get value of key";
const char helpStrings_s[] PROGMEM = "[KEY] [VALUE] - set value of key";
const char helpStrings_l[] PROGMEM = "- list all key and values";
const char helpStrings_x[] PROGMEM = "[KEY] - delete key";
const char helpStrings_w[] PROGMEM = "- write keys and values to EEPROM";
const char helpStrings_r[] PROGMEM = "- read keys and values from EEPROM";
const char helpStrings_b[] PROGMEM = "- reboot"; 
const char helpStrings_z[] PROGMEM = "- load default factory keys/values"; 

PGM_P const helpStrings[] PROGMEM = {
    helpStrings_h,
    helpStrings_g,
    helpStrings_s,
    helpStrings_l,
    helpStrings_x,
    helpStrings_w,
    helpStrings_r,
    helpStrings_b,
    helpStrings_z
};

TokenProcessor tp(Serial, 128, ' ', sizeof(commands), commands, commandFunctions, unknownCommand);

void unknownCommand(TokenProcessor *cps) {
  cps->Channel()->println(F("What?"));
  cps->Channel()->print(F("Command '"));
  cps->Channel()->print(cps->getCommand());
  cps->Channel()->println(F("' not found!"));
}

void getHelp(TokenProcessor *cps) {
  char cmd = '\0';
  char buffer[40];

  for (uint16_t i = 0; i < cps->size(); i++) {
    cps->getCommand(i, cmd);
    cps->Channel()->print(cmd);
    cps->Channel()->print(" ");
    strcpy_P(buffer, (PGM_P)pgm_read_dword(&(helpStrings[i])));
    cps->Channel()->print(buffer);
    if (i % 2 == 1) {
      cps->Channel()->println();
    } else {
      for (int16_t j = 0; j < 40 - strlen(buffer); j++)
        cps->Channel()->print(" ");
    }
  }
}

void showKeyValue(TokenProcessor *cps,  String key, String value) {
  showKeyValue(cps, key.c_str(), value.c_str());
}

void showKeyValue(TokenProcessor *cps, const char *key, const char*value) {
  cps->Channel()->print(key);
  cps->Channel()->print(F(" = "));
  cps->Channel()->println(value);  
}

void setValue(TokenProcessor *cps) {
  char *key;
  char *value;

  cps->Channel()->print(F("Setting key "));
  key = cps->nextToken();
  if (key == NULL) {
    cps->Channel()->println(F("Requires 2 arguments"));
    return;
  }

  value = cps->nextToken();
  if (value != NULL) {
    showKeyValue(cps, key, value);
    kvs.put(key, value);
  } else {
    cps->Channel()->println(F("Requires 2 arguments"));
  }
}

void getValue(TokenProcessor *cps) {
  char *key;
  String value;

  key = cps->nextToken();
  if (key != NULL) {

    if (kvs.get(key, value)) {
      showKeyValue(cps, key, value);
    } else {
      cps->Channel()->print(F("Key "));
      cps->Channel()->print(key);
      cps->Channel()->println(F(" not found"));
    }
  } else {
    listValues(cps);
  }
}

void delValue(TokenProcessor *cps) {
  char *key;
  String value;

  key = cps->nextToken();
  if (key != NULL) {
    cps->Channel()->print(F("Key "));
    cps->Channel()->print(key);
    if (kvs.get(key, value)) {
      kvs.remove(key);
      cps->Channel()->println(F(" deleted."));    
    } else {
      cps->Channel()->println(F(" not found."));
    }
  } else {
      cps->Channel()->println(F("Requires 1 argument"));
  }
}

void listValues_kvstring(TokenProcessor *cps, KVString *kv) {
  String k, v;
  uint16_t keycount = kv->size();
  
  cps->Channel()->print(keycount);
  cps->Channel()->println(F(" K/V found."));
  for (uint16_t i = 0; i < keycount; i++) {
    if (kv->get(i, k, v)) {
      showKeyValue(cps, k, v);
    }
  }
}


void listValues(TokenProcessor *cps) {
  cps->Channel()->println(F("Config K/V:"));
  listValues_kvstring(cps, &kvs);

//  cps->Channel()->println(F("Default K/V:"));
//  listValues_kvstring(cps, &defaultkvs);
}

void writeValues(TokenProcessor *cps) {
  char buffer[64];
  CRC16 crc;
  uint16_t configSize = 0;
  uint16_t startEEPROM = 0;
  uint16_t configpos = 0;
  uint8_t i;
  String kv;

#if (defined(ESP8266))
  EEPROM.begin(1024);
#endif

  crc.processBuffer(config_signature, sizeof(config_signature));
  for (i = 0; i < sizeof(config_signature); i++) {
    EEPROM.write(startEEPROM+i, config_signature[i]);
  }
  startEEPROM += sizeof(config_signature);
  for (i = 0; i < kvs.size(); i++) {
    if (kvs.getKV(i, kv)) {
      configSize+=kv.length()+1;
    }
  }
  buffer[0] = configSize >> 8;
  buffer[1] = configSize & 0xff;

  EEPROM.write(startEEPROM    , buffer[0]);
  EEPROM.write(startEEPROM + 1, buffer[1]);

  crc.processBuffer(buffer, 2);

  startEEPROM += 2;
  configpos = 0;
  for (i = 0; i < kvs.size(); i++) {
    if (kvs.getKV(i, kv)) {
      kv += "\n";
      strncpy(buffer, kv.c_str(), sizeof(buffer));
	    crc.processBuffer(buffer, kv.length());
      configpos += EEPROMwriteString(startEEPROM + configpos, kv);
    }
  }

  startEEPROM += configSize;
  uint16_t res_crc = crc.getCrc();	
  EEPROM.write(startEEPROM    , res_crc >> 8);
  EEPROM.write(startEEPROM + 1, res_crc & 0xff);

#if (defined(ESP8266))
  EEPROM.end();
#endif
  cps->Channel()->print(F("CRC=0x"));
  cps->Channel()->print(res_crc,HEX);
  cps->Channel()->println(F(". Configuration Written to EEPROM!"));
}

int EEPROMwriteString(int pos, String s) {
  uint16_t ss = s.length();
  for (uint16_t i = 0; i < ss; i++){
    EEPROM.write(pos + i, s[i]);
  }
  return ss;
}

void readValues(TokenProcessor *cps) {
  readEEPROMValues(*(cps->Channel()));
}


boolean readEEPROMValues(Stream &stm) {
  CRC16 crc;
  char buffer[64];
  uint16_t startEEPROM = 0;
  uint16_t configSize = 0;
  uint16_t configpos = 0;
  uint8_t i;
  
  for(i = 0; i < sizeof(config_signature); i++) {
    buffer[i] = EEPROM.read(startEEPROM+i);
    if (buffer[i] != config_signature[i]) {
      stm.println(F("Signature not found... empty configuration"));
      return false;
    }
  }
  crc.processBuffer(buffer, sizeof(config_signature));
  startEEPROM += sizeof(config_signature);
  
  buffer[0] = EEPROM.read(startEEPROM);
  buffer[1] = EEPROM.read(startEEPROM + 1);
  crc.processBuffer(buffer, 2);
  startEEPROM += 2;
  
  configSize = buffer[0] << 8 | buffer[1];
  configpos = 0;
  while (configpos < configSize) {
    for(i = 0; i < sizeof(buffer) && configpos < configSize; i++) {
       buffer[i] = EEPROM.read(startEEPROM + configpos);
       configpos++;
    }
    crc.processBuffer(buffer, i < sizeof(buffer) ? i : sizeof(buffer));
  }
  uint16_t res_crc = crc.getCrc();
  
  uint16_t testcrc = EEPROM.read(startEEPROM + configSize) << 8 | EEPROM.read(startEEPROM + configSize + 1);
  if (testcrc != res_crc) {
    stm.println(F("Invalid CRC Configuration."));
    stm.print(F("Stored CRC(0x"));
    stm.print(testcrc,HEX);
    stm.print(F(") != CalcCRC(0x"));
    stm.print(res_crc,HEX);
    stm.println(")");
    return false;
  }

  // The checksum is valid so read kv.
  configpos = 0;
  while(configpos < configSize) {
    for(i = 0; i<sizeof(buffer) && configpos < configSize; i++) {
       buffer[i] = EEPROM.read(startEEPROM + configpos);
       configpos++;
       if (buffer[i] == '\n') {
         break;
      }
    }
    buffer[i] = '\0';
    kvs.putKV(buffer);
  }
  stm.println(F("Configuration Read from EEPROM!"));  
  return true;
}

void reboot(TokenProcessor *cps) {
#if (defined(ESP8266))
  cps->Channel()->println(F("Rebooting...."));
  ESP.restart();
#else
  WDTCSR = _BV(WDE);
    while (1); // 16 ms
#endif
}

inline void tp_process() {
  tp.process();
}

void loadDefaultKVs(Stream &stm) {
  String k, v;
  uint16_t keycount = defaultkvs.size();
  int pos = 0;

// Clear All
  while ((pos = kvs.size() - 1) > 0) {
    kvs.remove(pos);
  }
  stm.print(keycount);
  stm.println(F(" K/V found."));
  for (uint16_t i = 0; i < keycount; i++) {
    if (defaultkvs.get(i, k, v)) {
      kvs.put(k, v);
    }
  }
}

void loadDefaults(TokenProcessor *cps) {
  loadDefaultKVs(*(cps->Channel()));
}

