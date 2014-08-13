#ifndef ntp_alt_h
#define ntp_alt_h

unsigned int localPort = 8888;  // local port to listen for UDP packets
const int NTP_PACKET_SIZE= 48;  // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets

// An UDP instance to let us send and receive packets over UDP
char cmd[30] = "";
char cmd1[] = "/bin/date ";
char cmd2[] = "/bin/date -s @";
static  char bufSTR[13];

// conversion date from epoch
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

#define SECONDS_PER_DAY 86400L
#define GMT 3600*2 //Italy Time Zone

#define SECONDS_FROM_1970_TO_2000 946684800
uint8_t yOff, m, d, hh, mm, ss;  // data

/* *** OLD NTP.H *** START */
unsigned long _unixTimeTS = 0;
unsigned long _unixTimeUpdate = 0;
/* *** OLD NTP.H *** END */

////////////////////////////////////////////////////////////////////////////////
// utility code to get Human Date From epoch

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
EthernetUDP UDP_as_NTP;


void dateGalileo(uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000;  // bring to 2000 timestamp from 1970

	ss = t % 60;
	t /= 60;
	mm = t % 60;
	t /= 60;
	hh = t % 24;
	uint16_t days = t / 24;
	uint8_t leap;
	for (yOff = 0; ; ++yOff) {
			leap = yOff % 4 == 0;
			if (days < 365 + leap)
					break;
			days -= 365 + leap;
	}
	for (m = 1; ; ++m) {
			uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
			if (leap && m == 2)
					++daysPerMonth;
			if (days < daysPerMonth)
					break;
			days -= daysPerMonth;
	}
	d = days + 1;
  // Displaying Current Date and Time
	Serial.print(hh);
	Serial.print(":");
	Serial.print(mm);
	Serial.print(":");
	Serial.println(ss);
	Serial.print(d);
	Serial.print("/");
	Serial.print(m);
	Serial.print("/");
	Serial.println(2000+yOff);
}
// end conversion date from epoch

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  UDP_as_NTP.beginPacket(address, 123); //NTP requests are to port 123
  UDP_as_NTP.write(packetBuffer,NTP_PACKET_SIZE);
  UDP_as_NTP.endPacket();
}

void NTPdataPacket() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  delay(1000);
  if ( UDP_as_NTP.parsePacket() ) {
    // We've received a packet, read the data from it
  	UDP_as_NTP.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears + GMT;
    _unixTimeTS = epoch;
    // print Unix time:
    Serial.println(epoch);
    //dateGalileo(epoch);
    delay(50);
    strcat(cmd, cmd2);
    snprintf(bufSTR,12, "%lu" ,epoch);
    strcat(cmd, bufSTR);
    
/*     strcat(cmd, cmd1);
    if(m < 10) strcat(cmd, "0");
    snprintf(bufSTR,4, "%d" ,m);
    strcat(cmd, bufSTR );
    if(d < 10) strcat(cmd, "0");
    snprintf(bufSTR,4, "%d" ,d);
    strcat(cmd, bufSTR );
    if(hh < 10) strcat(cmd, "0");
    snprintf(bufSTR,4, "%d" ,hh);
    strcat(cmd, bufSTR );
    if(mm < 10) strcat(cmd, "0");
    snprintf(bufSTR,4, "%d" ,mm);
    strcat(cmd, bufSTR );
    snprintf(bufSTR,4, "%d" ,yOff);
    strcat(cmd, bufSTR ); */
    Serial.print("Date and Time Command: ");
    Serial.println(cmd);
    //memset(cmd, 0, 21);
  }else{
    Serial.println("ERROR NTP PACKET NOT RECEIVED");
}
// Connect to NTP server and set System Clock
void initNTP() {
	UDP_as_NTP.begin(localPort);
	delay(1000);
	NTPdataPacket();
  // setting system clock
	char buf[64];
	  FILE *ptr;
	  Serial.print("COMANDO: ");
	  Serial.println(cmd);
	  if ((ptr = popen((char *)cmd, "r")) != NULL)
	  {
	    while (fgets(buf, 64, ptr) != NULL)
	    {
	      Serial.print(buf);
	    }
	  }
	  (void) pclose(ptr);
	  _unixTimeUpdate = millis();
}

// for debug purpose only
void testNTP() {
	char *cmd2 = "/bin/date +%F%t%T";
	  char buf[64];
	  FILE *ptr;

	  if ((ptr = popen(cmd2, "r")) != NULL)
	  {
	    while (fgets(buf, 64, ptr) != NULL)
	    {
	      Serial.print(buf);
	    }
	  }
	  (void) pclose(ptr);
	  delay(1000);
}

/* *** OLD NTP.H *** START */
unsigned long getUNIXTime() {
  unsigned long diff = millis() - _unixTimeUpdate;
  return (_unixTimeTS + (diff/1000));
}

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
/* *** OLD NTP.H *** END */

#endif
