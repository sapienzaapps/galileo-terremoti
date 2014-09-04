#ifndef ntp_h
#define ntp_h

const int NTP_PACKET_SIZE = 48;

byte _ntpPacketBuffer[NTP_PACKET_SIZE];
EthernetUDP _ntpc;
unsigned long _unixTimeTS = 0;
unsigned long _unixTimeUpdate = 0;
unsigned long _ntpInterval = 600000;
unsigned int _ntpSent = 0;

unsigned long fixword(byte b1, byte b2) {
#ifdef __IS_GALILEO
  return (b1 << 8) | b2;
#else
  return word(b1, b2);
#endif
}

// Return time in seconds 
unsigned long getUNIXTime() {
  unsigned long diff = millis() - _unixTimeUpdate;
  return (_unixTimeTS + (diff/1000));
}

// Return time in milliseconds
unsigned long getUNIXTimeMS() {
  unsigned long diff = millis() - _unixTimeUpdate;
  return diff % 1000;
}

void debugUNIXTime(unsigned long epoch) {
  // print the hour, minute and second:
  //Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
  Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  Serial.print(':');
  if ( ((epoch % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
  Serial.print(':');
  if ( (epoch % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
  }
  Serial.print(epoch %60); // print the second
  Serial.println(" UTC");
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
	n_NTP_connections++;

  // set all bytes in the buffer to 0
  memset(_ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  _ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  _ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
  _ntpPacketBuffer[2] = 6;     // Polling Interval
  _ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  _ntpPacketBuffer[12]  = 49;
  _ntpPacketBuffer[13]  = 0x4E;
  _ntpPacketBuffer[14]  = 49;
  _ntpPacketBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  // NTP requests are to port 123
  _ntpc.beginPacket(address, 123);  // the function would return failure but it's ok, because NTP is connectionless
  _ntpc.write(_ntpPacketBuffer,NTP_PACKET_SIZE);  // send the packet
  _ntpc.endPacket();
  Serial.print("NTP connection number: ");  // DEBUG
  Serial.println(n_NTP_connections);  // DEBUG
}

int checkNTPPacket() {
  int r = 0;
  if(_ntpc.parsePacket()) {
    r = 1;
    _ntpc.read(_ntpPacketBuffer, NTP_PACKET_SIZE);  // read the NTP packet
    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
    unsigned long highWord = fixword(_ntpPacketBuffer[40], _ntpPacketBuffer[41]);
    unsigned long lowWord = fixword(_ntpPacketBuffer[42], _ntpPacketBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;    
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;                          

    _unixTimeTS = epoch;
    _unixTimeUpdate = millis();

    unsigned long diff = epoch - getUNIXTime();// +2 fuso orario???
    Serial.print("Diff from NTP: ");
    Serial.println(diff);
    
  }
  return r;
}

void doNTPActions() {
  if(_ntpSent == 0) {
    unsigned long diff = millis() - _unixTimeUpdate;
    if(diff >= _ntpInterval) {
      sendNTPpacket(timeServer);
      _ntpSent = 1;
    }
  } else {
    if(checkNTPPacket() > 0) {
      debugUNIXTime(getUNIXTime());
      _ntpSent = 0;
    } else {
      _ntpSent++;
      // If the packet is lost, resend it and restart counter
      if(_ntpSent > 3000) {
        sendNTPpacket(timeServer);
        _ntpSent = 1;
      }
    }
  }
}

void forceNTPUpdate() {
  sendNTPpacket(timeServer);  // aggiungere delay()
  delay(1000);// Solo per il setup
  int r = checkNTPPacket();
  for(int i=0; r == 0; i++) {
    // If the packet is lost, send it again and restart counter
    if(i == 1500) {
      Serial.println("NTP not responding, re-trying...");
      sendNTPpacket(timeServer);
    } else if(i > 3000) {
      forceConfigUpdate(); // why ??
      Serial.print("HTTP Server: ");
      Serial.println(httpServer);
      Serial.print("NTP Server (ntp.h): ");
      Serial.println(timeServer);
      sendNTPpacket(timeServer);
      i = 0;
      continue;
    }
    delay(10);
    r = checkNTPPacket();
  }
  debugUNIXTime(getUNIXTime());
}

// initialize the NTP connection through the 123 port (standard)
void initNTP() {
  if (_ntpc.begin(123)) {
    Serial.println("NTP begin successful");
  }
  else {
    Serial.println("NTP begin NOT successful");
  }
  
  forceNTPUpdate();
}

#endif
