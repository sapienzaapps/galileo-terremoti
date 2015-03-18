#ifndef localstream_h
#define localstream_h 1

// const int CONTROLPKTSIZE = 16;
#define CONTROLPKTSIZE 36
byte _pktBuffer[CONTROLPKTSIZE];
EthernetUDP _cmdc;
uint32_t _udpDest = (uint32_t)0;
IPAddress _udpTemp(0, 0, 0, 0);

 
union ArrayToInteger {
  byte array[4];
  uint32_t integer;
};

union ArrayToFloat {
  byte array[4];
  float lat;
  float lon;
};

// check if the mobile APP sent a command to the device
int checkCommandPacket() {
  if (_cmdc.parsePacket() > 0) {  // if it received a packet
     memset(_pktBuffer, 0, 36);
    _cmdc.read(_pktBuffer, CONTROLPKTSIZE);
    // if its a packet from the mobile APP (contains the packet ID: INGV):
    // if the device must be discovered or has already been discovered
    if (memcmp("INGV\0", _pktBuffer, 5) == 0 &&
     (_pktBuffer[5] == 1 || memcmp(_pktBuffer+6, mac, 6) == 0)) {
      
      if(debugON){
        Serial.write(_pktBuffer,CONTROLPKTSIZE);
        Serial.println("");
      }
      byte command = _pktBuffer[5];
      ArrayToInteger cv;
      cv.array[0] = _pktBuffer[12];
      cv.array[1] = _pktBuffer[13];
      cv.array[2] = _pktBuffer[14];
      cv.array[3] = _pktBuffer[15];
     
      /* CONVERTING IP ADDRESS FROM CHAR TO BYTE */
      uint32_t IPinteger = (uint32_t)cv.array[0] << 24 | (uint32_t)cv.array[1] << 16 | (uint32_t)cv.array[2] << 8 | cv.array[3];
      _udpTemp = IPinteger;      
            
      switch (command) {
        case 1: // Discovery
          if (debugON) Serial.println("DISCOVERY");
          _pktBuffer[5] = 1;
          memcpy(_pktBuffer+6, mac, 6);  // store the MAC address of the device inside the packet to let the APP know
          _cmdc.beginPacket(_udpTemp, 62001);
          _cmdc.write(_pktBuffer, CONTROLPKTSIZE);
          _cmdc.endPacket();
          break;
        case 2: // Ping
        	if (debugON) Serial.println("PING");  // start sending packets to the mobile APP
          // Reply
          _pktBuffer[5] = 3;
          _cmdc.beginPacket(_udpTemp, 62001);
          _cmdc.write(_pktBuffer, CONTROLPKTSIZE);
          _cmdc.endPacket();
          if (debugON) Serial.println("PONG");
          break;
        case 4: // Start
        	if (debugON) Serial.println("START");  // start the socket connection with the mobile APP
          _udpDest = IPinteger;
          break;
        case 5: // Stop
        	if (debugON) Serial.println("STOP");  // close the socket connection with the mobile APP
          _udpDest = (uint32_t)0;
          break;        
        case 6: // Setted
        	if (debugON) Serial.println("Setted");  // close the socket connection with the mobile APP
          //if (debugON) Serial.println("PING");  // start sending packets to the mobile APP
          // Reply
          
          _pktBuffer[35] = '\0';
          if (debugON) Serial.println("PACCHETTO DATI SET GPS !!!!!!!!");
          Serial.println((char*)_pktBuffer);
          Serial.println("");
          char *argument = (char*)_pktBuffer+16;
          char latlonBuf[10];
          memcpy(latlonBuf,argument,(size_t) 9); 
          latlonBuf[9] = '\0';
          lat = stringToFloat(latlonBuf) ;
          //argument = strchr(argument,';');
          // argument++;
          argument = (char*)_pktBuffer+26;
          char latlonBuf2[10];
          memcpy(latlonBuf2,argument,(size_t) 9); 
          latlonBuf2[9] = '\0';
          lon = stringToFloat(latlonBuf2);
          _pktBuffer[5]= 6;
          _cmdc.beginPacket(_udpTemp, 62001);
          _cmdc.write(_pktBuffer, CONTROLPKTSIZE);// send ack lat/lon received
          _cmdc.endPacket();
          if (_pktBuffer[35] == '\0') Serial.println("FINE PACCHETTO GIUSTO # !!!");
          if (debugON) Serial.println("OK - DATA LAT/LON RECEIVED!!!");
          start = true;
          break;
      }
    }else{
      if (debugON) Serial.println("PACCHETTO DATI NON RICONOSCIUTO!!!!!!!!!!!!");
      Serial.print((char*)_pktBuffer);
      Serial.println("");
    }
  }
}

// send the accelerometer values to the mobile APP
void sendValues(struct RECORD *db) {
  int valx = db->valx;
  int valy = db->valy;
  int valz = db->valz;
  if(_udpDest != 0) {  // if a socket connection with the mobile APP has been established
    _cmdc.beginPacket(_udpDest, 62002);
    ArrayToInteger cv;
    cv.integer = valx;
    _pktBuffer[0] = cv.array[0];
    _pktBuffer[1] = cv.array[1];
    _pktBuffer[2] = cv.array[2];
    _pktBuffer[3] = cv.array[3];
    
    cv.integer = valy;
    _pktBuffer[4] = cv.array[0];
    _pktBuffer[5] = cv.array[1];
    _pktBuffer[6] = cv.array[2];
    _pktBuffer[7] = cv.array[3];
    
    cv.integer = valz;
    _pktBuffer[8] = cv.array[0];
    _pktBuffer[9] = cv.array[1];
    _pktBuffer[10] = cv.array[2];
    _pktBuffer[11] = cv.array[3];
    
    _cmdc.write(_pktBuffer, 12);
    _cmdc.endPacket();
  }
}

// establish the connection through the 62001 port to interact with the mobile APP
void commandInterfaceInit() {
  _cmdc.begin(62001);
}

#endif
