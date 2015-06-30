#ifndef ntp_alt_h
#define ntp_alt_h

#include <Arduino.h>
#include <Ethernet.h>

unsigned long fixword(byte b1, byte b2);
void dateGalileo(uint32_t t);
void sendNTPpacket(IPAddress &address);
bool NTPdataPacket();
void execSystemTimeUpdate();
void initNTP();
void testNTP();
unsigned long getUNIXTime();
unsigned long int getUNIXTimeMS();

#endif
