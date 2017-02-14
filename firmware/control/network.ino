//
// network utility functions
//

String GetMacAddress() {
  uint8_t mac[6];
    char macStr[18] = {0};
  WiFi.macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
    return  String(macStr);
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

