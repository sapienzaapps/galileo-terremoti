#ifndef galileo_log_h
#define galileo_log_h

#include <SD.h>

File myFile;

int galileo_write_log() {
  
  
   myFile = SD.open("log.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println("Galileo Log.");
    // close the file:
    myFile.close();
    return 1;
  }
  return 0;
}

#endif
