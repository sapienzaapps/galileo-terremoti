#ifndef httpconn_h
#define httpconn_h 1

#include <Arduino.h>
#include <Ethernet.h>

extern EthernetClient client;
extern byte inEvent;
extern unsigned long nextContact;

void byteMacToString(byte mac_address[]);
int getLine(EthernetClient c, char *buffer, int maxsize, int toRead);
int getLine(EthernetClient c, char *buffer, int maxsize);
int getPipe(EthernetClient c, char *buffer, int maxsize);
int getPipe(EthernetClient c, char *buffer, int maxsize, int toRead);
int prepareFastBuffer(char *buf, struct RECORD *db, struct TDEF *td);
int prepareMacBuffer(char *buf);
int prepareFirstBuffer(char *buf, struct RECORD *db, struct TDEF *td);
int prepareBuffer(char *buf, struct RECORD *db);
int ramopen(int seqid, int sendingIter);
void ramunlink(int seqid, int sendingIter);
void httpSendAlert1(struct RECORD *db, struct TDEF *td);
void getMacAddressFromServer();
uint8_t *HEXtoDecimal(const char *in, size_t len, uint8_t *out);
void readConfig();
void byteMacToString(byte mac_address[]);

#endif
