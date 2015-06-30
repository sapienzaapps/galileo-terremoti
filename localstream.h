#ifndef localstream_h
#define localstream_h 1

// const int CONTROLPKTSIZE = 16;
#define CONTROLPKTSIZE 48 //36

int checkCommandPacket();
void sendValues(struct RECORD *db);
void commandInterfaceInit();

#endif
