
#define CALIBRATIONITER 1000
#define ORANGEZONE 5

long nextHour = 0;

float absavg(int *buf, int size) {
  float ret = 0;
  for(int i=0; i < size; i++) {
    ret += (buf[i]<0 ? buf[i]*-1 : buf[i]);
  }
  return ret/size;
}

float stddev(int *buf, int size, float avg) {
  // Formula: RAD ( SUM{i,size}( (x[i] - avg)^2 ) / (size - 1) )
  double sum = 0;
  for(int i=0; i < size; i++) {
    sum += pow(buf[i] - avg, 2);
  }
  
  return sqrt(sum/(size-1));
}

void writeDouble(int address, double value) {
  byte val[8];
  int i = 0;
  memcpy(val, &value, 8);
  for(i=0; i < 8; i++) {
    EEPROM.write(address+i, val[i]);
  }
}

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

void initEEPROM() {
  int c1 = EEPROM.read(0);
  int c2 = EEPROM.read(1);
  int c3 = EEPROM.read(2);
  int c4 = EEPROM.read(3);
  
  if(c1 == 'I' && c2 == 'N' && c3 == 'G' && c4 == 'V') {
    Serial.println("EEPROM already formatted, skipping...");
  } else {
    Serial.println("EEPROM not formatted, let's do it");
    // Inizializza memoria
    int i=0;
    for(i=4; i < (48*24); i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.write(0, 'I');
    EEPROM.write(1, 'N');
    EEPROM.write(2, 'G');
    EEPROM.write(3, 'V');
    nextHour = (getUNIXTime()  % 86400L) / 3600;
  }
}

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
  
  double temp = readDouble(4 + currentHour*48);
  
  if(nextHour == currentHour || temp == 0) {
    int cbufx[CALIBRATIONITER];
    int cbufy[CALIBRATIONITER];
    int cbufz[CALIBRATIONITER];
    
    Serial.print("Begin calibration for hour: ");
    Serial.println(currentHour);
    
    int i=0;
    for(i=0; i < CALIBRATIONITER; i++) {
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
    
    Serial.println("Calibration ended");
    nextHour = (random() % 23);
    Serial.print("Next calibration scheduled for ");
    Serial.println(nextHour);
    
  } else {
    Serial.println("Loading values from EEPROM for new hour");
    int pos = 4 + currentHour*48;
    pthresx = readDouble(pos);
    Serial.print("pthresx: ");
    Serial.print(pthresx);
    
    pos += 8;
    pthresy = readDouble(pos);
    Serial.print(" pthresy: ");
    Serial.print(pthresy);
    
    pos += 8;
    pthresz = readDouble(pos);
    Serial.print(" pthresz: ");
    Serial.println(pthresz);
    
    pos += 8;
    nthresx = readDouble(pos);
    Serial.print("nthresx: ");
    Serial.print(nthresx);
    
    pos += 8;
    nthresy = readDouble(pos);
    Serial.print(" nthresy: ");
    Serial.print(nthresy);
    
    pos += 8;
    nthresz = readDouble(pos);
    Serial.print(" nthresz: ");
    Serial.println(nthresz);
  }
}
