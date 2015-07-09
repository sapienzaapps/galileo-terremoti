#ifndef cfgupdate_h
#define cfgupdate_h 1

#include "Log.h"

extern unsigned long lastCfgUpdate;

IPAddress getFromString(char *ipAddr);
int prepareConfigBuffer(char *buf);
boolean getConfigNew();
void initConfigUpdates();

#endif
