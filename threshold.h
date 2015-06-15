#define CALIBRATIONITER 1000
#define ORANGEZONE 6

#include "GalileoLog.h"

FILE *thrSDFile; // file for threshold storing
char *threshold_path = "media/realroot/threshold.txt";

long nextHour = 0;

float absavg(int *buf, int size) {
  float ret = 0;
  for(int i=0; i < size; i++) {
    ret += (buf[i]<0 ? buf[i]*-1 : buf[i]);
  }
  return ret/size;
}

// Standard Deviation
float stddev(int *buf, int size, float avg) {
  // Formula: RAD ( SUM{i,size}( (x[i] - avg)^2 ) / (size - 1) )
  double sum = 0;
  for(int i=0; i < size; i++) {
    sum += pow(buf[i] - avg, 2);
  }
  
  return sqrt(sum/(size-1));
}
// Write a Double to EEPROM
void writeDouble(int address, double value) {
  byte val[8];
  int i = 0;
  memcpy(val, &value, 8);
  for(i=0; i < 8; i++) {
    EEPROM.write(address+i, val[i]);
  }
}
// Read a Double from EEPROM
double readDouble(int address) {
  byte val[8];
  int i = 0;
  for(i=0; i < 8; i++) {
    val[i] = EEPROM.read(address+i);
  }
  double _value = 0;
  memcpy(&_value, val, 8);
  return _value;
}


// Read a Double from SD CARD
double readDoubleSD(int offset, FILE *facc, size_t sizeacc) {
  double val;
  int i = 0;
  if(fseek(facc, offset, SEEK_SET) == 0){
  fread(&val, sizeacc, 1, facc); 
  //double _value = 0;
  //memcpy(&_value, val, sizeacc);
  if(debugON) Serial.print("doubleRD: ");
  if(debugON) Serial.println(val);
  return val;
  }
  return -1;
}

// Read a Double from SD CARD
void readDoubleSD2(int offset, FILE *facc, size_t sizeacc, double *ptrd) {
  double val;
  int i = 0;
  if(fseek(facc, offset, SEEK_SET) == 0){
  fread(&ptrd, sizeacc, 1, facc); 
  //double _value = 0;
  //memcpy(&_value, val, sizeacc);
  if(debugON) Serial.print("doubleRD222222222222 - ");
  // if(debugON) Serial.println((char*)ptrd, DEC);
  // return *ptrd;
  }
  // return -1;
  delay(50);
}

// Read a Double from SD CARD
void readDoubleSDS(int offset, FILE *facc, size_t sizeacc, double *ptrd) {
  double val;
  int i = 0;
  if(fseek(facc, offset, SEEK_SET) == 0){
  fread(&ptrd, sizeacc, 1, facc); 
  //double _value = 0;
  //memcpy(&_value, val, sizeacc);
  if(debugON) Serial.print("doubleRD222222222222 - ");
  // if(debugON) Serial.println((char*)ptrd, DEC);
  // return *ptrd;
  }
  // return -1;
  delay(50);
}

// Write a Double to SD CARD
void writeDoubleSD(int offset, double *value, FILE *facc, size_t sizeacc ) {
  double val;
  int i = 0;
  //memcpy(&val, &value, sizeacc);
  fseek(facc, offset, SEEK_SET);
  fwrite(&value, sizeacc, 1, facc);
  if(debugON) Serial.print("doubleWR: ");
  if(debugON) Serial.println(*value);
  delay(50);
}

// Initialize the EEPROM memory
void initEEPROM(bool forceInitEEPROM) {
  int c1 = EEPROM.read(0);
  int c2 = EEPROM.read(1);
  int c3 = EEPROM.read(2);
  int c4 = EEPROM.read(3);
  
  if (!forceInitEEPROM && (c1 == 'I' && c2 == 'N' && c3 == 'G' && c4 == 'V')) {
    if (debugON) Serial.println("EEPROM already formatted, skipping...");
  }
  else {
    if (debugON) Serial.println("EEPROM not formatted, let's do it");
    int i=0;
    for (i=4; i < (48*24); i++) { //controllare grandezza eeprom
      EEPROM.write(i, 0);
    }
    EEPROM.write(0, 'I');
    EEPROM.write(1, 'N');
    EEPROM.write(2, 'G');
    EEPROM.write(3, 'V');
    nextHour = (getUNIXTime()  % 86400L) / 3600;
  }
}

// Initialize the SD THRESHOLD FILE
void initThrSD(bool forceInitEEPROM) {
  char headSD[5];
  headSD[0] = 'I';
  headSD[1] = 'N';
  headSD[2] = 'G';
  headSD[3] = 'V';
  headSD[4] = '\0';
  thrSDFile = fopen(threshold_path, "w+");
  // if(thrSDFile != NULL){
    char buffer[5]={0,0,0,0,0};
    //memset(buffer, 0, 4);
    fseek(thrSDFile, 0, SEEK_SET);   /* Seek to the beginning of the file */
    size_t n = fread(&buffer, sizeof(buffer), 1, thrSDFile); // check for initialized file
    
    //buffer[4] = '\0';
    if(debugON) Serial.print("Inizio file: ");
    if(debugON) Serial.println((char*)buffer);
    if(debugON) Serial.print("Dimensione read: ");
    if(debugON) Serial.println(n);
    if ((!forceInitEEPROM) && (strncmp(headSD, buffer,5) == 0)) {
      if (debugON) Serial.println("Threshold File already formatted, skipping...");
      if (debugON &&(strncmp(headSD, buffer,5) == 0)) Serial.println("INGV oooooooooKKKKKKKK...");
    }else {
      if (debugON) Serial.println("Threshold File not formatted, let's do it");
      fseek(thrSDFile, 0, SEEK_SET);   /* Seek to the beginning of the file */
      fwrite(&headSD, sizeof(char), 5, thrSDFile);
      fseek(thrSDFile, 5, SEEK_SET);
      char a[48*24] ={0};
      fwrite(&a, sizeof(char), 48*24, thrSDFile);
      if (debugON) Serial.print("@@@@@@@@@@@@@a: ");
      if (debugON) Serial.println(a);
      fseek(thrSDFile, 0, SEEK_SET); 
      char buft[5];
      fread(&buft, sizeof(byte), 5, thrSDFile); // check for initialized file
      if (debugON) Serial.print("@@@@@@@@@@@@@inixio: ");
      if (debugON) Serial.println(buft);
      nextHour = (getUNIXTime()  % 86400L) / 3600;
    }
    fclose(thrSDFile);
  // }else{if(debugON) Serial.print("FILE NULL!!!!!");}
}

void setThresholdValues(AcceleroMMA7361 ac, int currentHour) {
  int cbufx[CALIBRATIONITER];
  int cbufy[CALIBRATIONITER];
  int cbufz[CALIBRATIONITER];

  if (debugON) Serial.print("Begin calibration for hour: ");
  if (debugON) Serial.println(currentHour);

  int i=0;
  for (i=0; i < CALIBRATIONITER; i++) {
    cbufx[i] = ac.getXAccel();
    cbufy[i] = ac.getYAccel();
    cbufz[i] = ac.getZAccel();
  }

  float avgx = absavg(cbufx, CALIBRATIONITER);
  float avgy = absavg(cbufy, CALIBRATIONITER);
  float avgz = absavg(cbufz, CALIBRATIONITER);
  double sdevx = stddev(cbufx, CALIBRATIONITER, avgx);
  double sdevy = stddev(cbufy, CALIBRATIONITER, avgy);
  double sdevz = stddev(cbufz, CALIBRATIONITER, avgz);

  pthresx = avgx + (sdevx + ORANGEZONE);
  pthresy = avgy + (sdevy + ORANGEZONE);
  pthresz = avgz + (sdevz + ORANGEZONE);

  nthresx = avgx - (sdevx + ORANGEZONE);
  nthresy = avgy - (sdevy + ORANGEZONE);
  nthresz = avgz - (sdevz + ORANGEZONE);
}

void setThresholdValuesBasic(AcceleroMMA7361 ac, int currentHour) {
  pthresx = 10;
  pthresy = 10;
  pthresz = 100;
}
void checkCalibrationNeededNOSD(AcceleroMMA7361 ac, int currentHour) {
   
  // do calibration every random amount of hours? or if it's the first time ever
  if ((nextHour == currentHour) || (pthresx <= 0.00) || forceInitEEPROM ) {
    if(nextHour == currentHour)Serial.print("nextHour = currentHour on SD #-#-#-#-#-#-#-#-#-#: ");
    if(nextHour == currentHour)Serial.println(currentHour);
    if(pthresx <= 0)Serial.println("pthresx <= 0 on NOSD #-#-#-#-#-#-#-#-#-#");
    //setThresholdValuesBasic(ac, currentHour);
    if(!yellowLedStatus){
      digitalWrite(yellow_Led,!yellowLedStatus);
      yellowLedStatus = !yellowLedStatus;
    }
    if (debugON){ 
      Serial.println("WRITE THRESHOLD on SD #-#-#-#-#-#-#-#-#-#");
    }
    setThresholdValues(ac, currentHour);
    nextHour = ((currentHour + 1) % 24);
    forceInitEEPROM = false;
  }
}
void checkCalibrationNeededSD(AcceleroMMA7361 ac, int currentHour) {
  if (debugON){ 
      Serial.println("Calibration on SD START *-*-*-*-*-*-*-*-*");
  }
  // Utility (Galileo):
  // sizeof(unsigned long) = 4
  // sizeof(unsigned int) = 4
  // sizeof(float) = 4
  // sizeof(double) = 8
  
  // I primi 4 bytes sono popolati con "INGV". Se la scritta non e' presente
  // allora la EEPROM non e' inizializzata
  
  // Mappa memoria (offset + 4):
  // [posizione, lunghezza]
  // [i*48, 48] : dati sull'ora "i"
  // --> 8 byte (double) con il valore della soglia positiva per ogni asse [X-Y-Z]
  // --> 8 byte (double) con il valore della soglia negativa per ogni asse [X-Y-Z]
  thrSDFile = fopen(threshold_path, "r+");
  double temp;
  if(thrSDFile != NULL){
    
    Serial.print("temp #-#-#-#-#-#-#-#-#-#: ");
    readDoubleSD2(5 + currentHour*48, thrSDFile, sizeof(double),&temp ); // ??? cosa deve leggere ???
    Serial.println(temp);
  
    // double temp = readDouble(4 + currentHour*48); // ??? cosa deve leggere ???
    
    // do calibration every random amount of hours? or if it's the first time ever
    if ((nextHour == currentHour) || (temp <= 0.00) || forceInitEEPROM ) {
      if(nextHour == currentHour)Serial.print("nextHour = currentHour on SD #-#-#-#-#-#-#-#-#-#: ");
      if(nextHour == currentHour)Serial.println(currentHour);
      if(temp <= 0)Serial.println("temp <= 0 on SD #-#-#-#-#-#-#-#-#-#");
      //setThresholdValuesBasic(ac, currentHour);
      if(!yellowLedStatus){
        digitalWrite(yellow_Led,!yellowLedStatus);
        yellowLedStatus = !yellowLedStatus;
      }
      if (debugON){ 
        Serial.println("WRITE THRESHOLD on SD #-#-#-#-#-#-#-#-#-#");
      }
      setThresholdValues(ac, currentHour);
      int pos = 5 + currentHour*48;
      writeDoubleSD(pos, &pthresx, thrSDFile, sizeof(double));
      pos += 8;
      writeDoubleSD(pos, &pthresy, thrSDFile, sizeof(double));
      pos += 8;
      writeDoubleSD(pos, &pthresz, thrSDFile, sizeof(double));
      pos += 8;
      
      writeDoubleSD(pos, &nthresx, thrSDFile, sizeof(double));
      pos += 8;
      writeDoubleSD(pos, &nthresy, thrSDFile, sizeof(double));
      pos += 8;
      writeDoubleSD(pos, &nthresz, thrSDFile, sizeof(double));
      fclose(thrSDFile);
      if (logON) log("Calibration ended");
      if (debugON){ 
        showThresholdValues();
      }
      //nextHour = (random() % 24);
      nextHour = ((currentHour + 1) % 24);
      if (debugON) Serial.print("Next calibration scheduled for ");
      if (debugON) Serial.println(nextHour);
    }
    else {
      if (debugON) Serial.println("Loading values from SD CARD for new hour");
      int pos = 4 + currentHour*48;
      readDoubleSD2(pos, thrSDFile, sizeof(double), &pthresx);
      if (debugON) Serial.print("pthresx: ");
      if (debugON) Serial.print(pthresx);
      
      pos += 8;
      readDoubleSD2(pos, thrSDFile, sizeof(double), &pthresy);
      if (debugON) Serial.print(" pthresy: ");
      if (debugON) Serial.print(pthresy);
      
      pos += 8;
      readDoubleSD2(pos, thrSDFile, sizeof(double), &pthresz);
      if (debugON) Serial.print(" pthresz: ");
      if (debugON) Serial.println(pthresz);
      
      pos += 8;
      readDoubleSD2(pos, thrSDFile, sizeof(double), &nthresx);
      if (debugON) Serial.print("nthresx: ");
      if (debugON) Serial.print(nthresx);
      
      pos += 8;
      readDoubleSD2(pos, thrSDFile, sizeof(double), &nthresy);
      if (debugON) Serial.print(" nthresy: ");
      if (debugON) Serial.print(nthresy);
      
      pos += 8;
      readDoubleSD2(pos, thrSDFile, sizeof(double),&nthresz);
      fclose(thrSDFile);
      if (debugON) Serial.print(" nthresz: ");
      if (debugON) Serial.println(nthresz);
    }
    if(yellowLedStatus){
      digitalWrite(yellow_Led,!yellowLedStatus);
      yellowLedStatus = !yellowLedStatus;
    }
  }else{Serial.println("NULL -----------------SD THRESHOLD!!!!!!!");
    
    }
}


// void setThresholdValues(AcceleroMMA7361 ac, int currentHour) {
  // int cbufx[CALIBRATIONITER];
  // int cbufy[CALIBRATIONITER];
  // int cbufz[CALIBRATIONITER];

  // if (debugON) Serial.print("Begin calibration for hour: ");
  // if (debugON) Serial.println(currentHour);

  // int i=0;
  // for (i=0; i < CALIBRATIONITER; i++) {
    // cbufx[i] = ac.getXAccel();
    // cbufy[i] = ac.getYAccel();
    // cbufz[i] = ac.getZAccel();
  // }

  // float avgx = absavg(cbufx, CALIBRATIONITER);
  // float avgy = absavg(cbufy, CALIBRATIONITER);
  // float avgz = absavg(cbufz, CALIBRATIONITER);
  // double sdevx = stddev(cbufx, CALIBRATIONITER, avgx);
  // double sdevy = stddev(cbufy, CALIBRATIONITER, avgy);
  // double sdevz = stddev(cbufz, CALIBRATIONITER, avgz);

  // pthresx = avgx + (sdevx + ORANGEZONE);
  // pthresy = avgy + (sdevy + ORANGEZONE);
  // pthresz = avgz + (sdevz + ORANGEZONE);

  // nthresx = avgx - (sdevx + ORANGEZONE);
  // nthresy = avgy - (sdevy + ORANGEZONE);
  // nthresz = avgz - (sdevz + ORANGEZONE);
// }

// void setThresholdValuesBasic(AcceleroMMA7361 ac, int currentHour) {
  // pthresx = 10;
  // pthresy = 10;
  // pthresz = 100;
// }

void checkCalibrationNeeded(AcceleroMMA7361 ac, int currentHour) {
  // Utility (Galileo):
  // sizeof(unsigned long) = 4
  // sizeof(unsigned int) = 4
  // sizeof(float) = 4
  // sizeof(double) = 8
  
  // I primi 4 bytes sono popolati con "INGV". Se la scritta non e' presente
  // allora la EEPROM non e' inizializzata
  
  // Mappa memoria (offset + 4):
  // [posizione, lunghezza]
  // [i*48, 48] : dati sull'ora "i"
  // --> 8 byte (double) con il valore della soglia positiva per ogni asse [X-Y-Z]
  // --> 8 byte (double) con il valore della soglia negativa per ogni asse [X-Y-Z]
  
  double temp = readDouble(4 + currentHour*48); // ??? cosa deve leggere ???
  
  // do calibration every random amount of hours? or if it's the first time ever
  if (nextHour == currentHour || temp == 0) {
    //setThresholdValuesBasic(ac, currentHour);
    if(!yellowLedStatus){
      digitalWrite(yellow_Led,!yellowLedStatus);
      yellowLedStatus = !yellowLedStatus;
    }
    setThresholdValues(ac, currentHour);
    int pos = 4 + currentHour*48;
    
    writeDouble(pos, pthresx);
    pos += 8;
    writeDouble(pos, pthresy);
    pos += 8;
    writeDouble(pos, pthresz);
    pos += 8;
    
    writeDouble(pos, nthresx);
    pos += 8;
    writeDouble(pos, nthresy);
    pos += 8;
    writeDouble(pos, nthresz);
    
    if (logON) log("Calibration ended");
    if (debugON){ 
      Serial.println("Calibration ended - with values:");
      Serial.println("------------------");
      Serial.print("pthresx: ");
      Serial.print(pthresx);
      Serial.print(" pthresy: ");
      Serial.print(pthresy);
      Serial.print(" pthresz: ");
      Serial.println(pthresz);
      Serial.print("nthresx: ");
      Serial.print(nthresx);
      Serial.print(" nthresy: ");
      Serial.print(nthresy);
      Serial.print(" nthresz: ");
      Serial.println(nthresz);
      Serial.println("------------------");
    }
    //nextHour = (random() % 24);
    nextHour = ((currentHour + 1) % 24);
    if (debugON) Serial.print("Next calibration scheduled for ");
    if (debugON) Serial.println(nextHour);
  }
  else {
    if (debugON) Serial.println("Loading values from EEPROM for new hour");
    int pos = 4 + currentHour*48;
    pthresx = readDouble(pos);
    if (debugON) Serial.print("pthresx: ");
    if (debugON) Serial.print(pthresx);
    
    pos += 8;
    pthresy = readDouble(pos);
    if (debugON) Serial.print(" pthresy: ");
    if (debugON) Serial.print(pthresy);
    
    pos += 8;
    pthresz = readDouble(pos);
    if (debugON) Serial.print(" pthresz: ");
    if (debugON) Serial.println(pthresz);
    
    pos += 8;
    nthresx = readDouble(pos);
    if (debugON) Serial.print("nthresx: ");
    if (debugON) Serial.print(nthresx);
    
    pos += 8;
    nthresy = readDouble(pos);
    if (debugON) Serial.print(" nthresy: ");
    if (debugON) Serial.print(nthresy);
    
    pos += 8;
    nthresz = readDouble(pos);
    if (debugON) Serial.print(" nthresz: ");
    if (debugON) Serial.println(nthresz);
  }
  if(yellowLedStatus){
    digitalWrite(yellow_Led,!yellowLedStatus);
    yellowLedStatus = !yellowLedStatus;
  }
}
// void showThresholdValues(){
  
  // Serial.println("Calibration on SD ended - with values:");
  // Serial.println("---------------------------------------------");
  // Serial.print("pthresx: ");
  // Serial.print(pthresx);
  // Serial.print(" pthresy: ");
  // Serial.print(pthresy);
  // Serial.print(" pthresz: ");
  // Serial.println(pthresz);
  // Serial.print("nthresx: ");
  // Serial.print(nthresx);
  // Serial.print(" nthresy: ");
  // Serial.print(nthresy);
  // Serial.print(" nthresz: ");
  // Serial.println(nthresz);
  // Serial.println("---------------------------------------------");
  
// }
