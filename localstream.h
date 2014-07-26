#ifndef localstream_h
#define localstream_h 1

const int CONTROLPKTSIZE = 16;
byte _pktBuffer[CONTROLPKTSIZE];
EthernetUDP _cmdc;
uint32_t _udpDest = (uint32_t)0;
IPAddress _udpTemp(0, 0, 0, 0);

 
union ArrayToInteger {
  byte array[4];
  uint32_t integer;
};


// check if the mobile APP sent a command to the device
int checkCommandPacket() {
  if(_cmdc.parsePacket()) {  // if it received a packet
     
    _cmdc.read(_pktBuffer, CONTROLPKTSIZE);
    // if its a packet from the mobile APP (contains the packet ID: INGV):
    // if the device must be discovered or has already been discovered
    if(memcmp("INGV\0", _pktBuffer, 5) == 0 &&
     (_pktBuffer[5] == 1 || memcmp(_pktBuffer+6, mac, 6) == 0)) {
      
      byte command = _pktBuffer[5];
      ArrayToInteger cv;
      cv.array[0] = _pktBuffer[12];
      cv.array[1] = _pktBuffer[13];
      cv.array[2] = _pktBuffer[14];
      cv.array[3] = _pktBuffer[15];
     
      /* CONVERTING IP ADDRESS FROM CHAR TO BYTE */
      uint32_t IPinteger = (uint32_t)cv.array[0] << 24 | (uint32_t)cv.array[1] << 16 | (uint32_t)cv.array[2] << 8 | cv.array[3];
      _udpTemp = IPinteger;      
            
      switch(command) {
        case 1: // Discovery
          Serial.println("DISCOVERY");
          _pktBuffer[5] = 1;
          memcpy(_pktBuffer+6, mac, 6);  // store the MAC address of the device inside the packet to let the APP know
          _cmdc.beginPacket(_udpTemp, 62001);
          _cmdc.write(_pktBuffer, CONTROLPKTSIZE);
          _cmdc.endPacket();
          break;
        case 2: // Ping
          Serial.println("PING");  // start sending packets to the mobile APP
          // Reply
          _pktBuffer[5] = 3;
          _cmdc.beginPacket(_udpTemp, 62001);
          _cmdc.write(_pktBuffer, CONTROLPKTSIZE);
          _cmdc.endPacket();
          Serial.println("PONG");
          break;
        case 4: // Start
          Serial.println("START");  // start the socket connection with the mobile APP
          _udpDest = IPinteger;
          break;
        case 5: // Stop
          Serial.println("STOP");  // close the socket connection with the mobile APP
          _udpDest = (uint32_t)0;
          break;
      }
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
