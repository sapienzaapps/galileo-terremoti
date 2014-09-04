#ifndef httpconn_h
#define httpconn_h 1

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
void *pthread_httpSend(void *ptr);
int tempseqid, tempsendingIter;

struct SEQDB {
  struct RECORD *record;
  struct SEQDB *next;
};
//PTHREAD
pthread_t thread1;
int iret1;

EthernetClient client;
byte inEvent = 0;
unsigned long seqid = 0;
unsigned long nextContact = 0;
//boolean *isSendingMapped;
//boolean *isLastMapped;

boolean isSendingMapped;
boolean isLastMapped;
//unsigned long *nextContactMapped;

unsigned long nextContactMapped;
unsigned long stopClient = 0;
int seqDBfd;
unsigned long sendingIter = 0;
boolean isSending = false;

int getLine(EthernetClient c, char* buffer, int maxsize, int toRead) { //???????
  int i;
  byte done = 0;
  memset(buffer, 0, maxsize);  // set the buffer to 0
  for(i=0; i < maxsize-1 && done == 0; i++) {
    //while(!client.available() && client.connected());
    buffer[i] = c.read();
    if(buffer[i] == '\r') i--; //????????
    if(buffer[i] == '\n' || buffer[i] == -1) {  // if there is nothing more to read
      done = 1;
      buffer[i] = 0;
    }
    if(toRead == -1) {
    	// do nothing: it'll stop only if the buffer is emptied
    } else if(toRead > 1) {
      toRead--;
    } else {
      done = 1;
    }
  }

  return i-1;
}

int getLine(EthernetClient c, char* buffer, int maxsize) {
  return getLine(c, buffer, maxsize, -1);
}

int prepareFirstBuffer(char* buf, struct RECORD *db, struct TDEF *td) {
	// ts, ms, pthresx, pthresy, pthresz, nthresx, nthresy, nthresz, deltax, deltay, deltaz
  return sprintf(buf, "%ld;%ld;%f;%f;%f;%f;%f;%f;%ld;%ld;%ld",
                     db->ts, db->ms, td->pthresx, td->pthresy, td->pthresz, td->nthresx, td->nthresy, td->nthresz, db->valx, db->valy, db->valz);
}

int prepareBuffer(char* buf, struct RECORD *db) {
	// ts, ms, deltax, deltay, deltaz
  return sprintf(buf, "%ld;%ld;%ld;%ld;%ld\r\n", db->ts, db->ms, db->valx, db->valy, db->valz);
}

// open the file into the RAM of the device
int ramopen(int seqid, int sendingIter) {
	if (debugON) Serial.print("Opening file ");
	if (logON) log("Opening file ");
  // Send values
  char filename[100];
  sprintf(filename, "/media/ram/rel%d_%d.dat", seqid, sendingIter);
  if (debugON) Serial.println(filename);
  return open(filename, O_RDWR|O_CREAT);
}

// remove the file from the RAM of the device
void ramunlink(int seqid, int sendingIter) {
	if (debugON) Serial.print("Removing file ");
	if (logON) log("Removing file ");
  // Send values
  char filename[100];
  sprintf(filename, "/media/ram/rel%d_%d.dat", seqid, sendingIter);
  if (debugON) Serial.println(filename);
  unlink(filename);
}

void realHttpSendValues() {
  close(seqDBfd);
  seqDBfd = -1;
  tempseqid = seqid;
  tempsendingIter = sendingIter;
  
  isLastMapped = true; 
  isSendingMapped = true;
  nextContactMapped = 0;
  
  sendingIter++;
  seqDBfd = ramopen(seqid, sendingIter);

  if ((debugON) && (seqDBfd == -1)) Serial.print("Error in ramopen: ");
  //int pid = fork();
  // pthread_creation -->pthread_httpSend()
  iret1 = pthread_create( &thread1, NULL, pthread_httpSend, NULL);
  if(iret1)
  {
   Serial.print("Error - pthread_create() 1 return code: ");
   Serial.println(iret1);
   delay(10);
   exit(EXIT_FAILURE);
  }
}

// send the accelerometer values that got over the threshold
void httpSendValues(struct RECORD *db, struct TDEF *td) {
  if(inEvent == 1) {  // if an "event" is running
    if(seqDBfd != -1) {  // if the file that stores the sequence has not yet been closed
      write(seqDBfd, db, sizeof(struct RECORD));  // write the sequence onto the file into tRAM memory
    }

    if(isSending) { // IF IS SENDING DATA
      // Check if finished
      if(!isSendingMapped) {
      	if (debugON) Serial.println("Child ended");
      	if (logON) log("Child ended");
        isSending = false;
        nextContact = nextContactMapped;

        if (debugON) Serial.print("nextContactMapped: ");
        if (debugON) Serial.print(nextContactMapped);

        if(isLastMapped) {
        	if (debugON) Serial.println("Storing finished");
        	if (logON) log("Storing finished");
          close(seqDBfd);
          ramunlink(seqid, sendingIter);
          seqDBfd = -1;
          inEvent = 0;
        }
      }
    }else if(getUNIXTime() >= nextContact) {   // IF IS THE RIGHT TIME, CONTACT SERVER
    	if (debugON) Serial.println("Child starting");
    	if (logON) log("Child starting");
      realHttpSendValues();
      isSending = true;
    }
  } else {
    // New Event ----------------------------------------------------------
  	if (debugON) Serial.print("New Event, values (X-Y-Z): ");
  	if (logON) log("New Event, values (X-Y-Z): ");
    printRecord(db); // Debug print recorded axis values
    if (debugON) Serial.println();

    if(client.connect(httpServer, 80)) {	// Connecting to server to retrieve "sequence id"
    	if (debugON) Serial.print("Requesting SEQID to:");
    	if (debugON) Serial.println(httpServer);

      char rBuffer[300];
      int rsize = prepareFirstBuffer(rBuffer, db, td);  // prepare the info for the new entry into the DB

      client.print("POST ");
      client.print(path_domain);
      client.print("/device.php?op=put&mac=");
      for(int m=0; m < 6; m++) {
        if(mac[m] < 0x10) client.print("0");
        client.print(mac[m], HEX);
      }
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(httpServer);
      client.println("Content-Type: text/plain");
      client.print("Content-Length: ");
      client.println(rsize);
      client.println("Connection: close");
      client.println("");
      client.print(rBuffer);
      while(!client.available()){;} // Attende che arrivino i dati ******************
      // Reading headers
      int s = getLine(client, rBuffer, 300);
      if(strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {
        int bodySize = 0;
        do {
          s = getLine(client, rBuffer, 300);
          if(strncmp(rBuffer, "Content-Length", 14) == 0) {
            char* separator = strchr(rBuffer, ':');
            if(*(separator+1) == ' ') {
              separator += 2;
            } else {
              separator++;
            }
            bodySize = atoi(separator);
          }
        } while(s > 0);

        // Content
        s = getLine(client, rBuffer, 300, bodySize);

        char* separator = strchr(rBuffer, ';'); //??????????????????
        *separator = 0;
        seqid = atol(rBuffer);  // get the sequence ID
        nextContact = atol(separator+1) + getUNIXTime();//???????????
        if (debugON) Serial.print("SEQID:");
        if (debugON) Serial.println(seqid);
        if (logON) log("SEQID:");
        if (logON) log(rBuffer);

        if (debugON) Serial.print("Next Contact scheduled for: ");
        debugUNIXTime(nextContact);
        inEvent = 1;

        sendingIter = 0;
        seqDBfd = ramopen(seqid, sendingIter);
      } else {
      	if (debugON) Serial.print("Error in reply: ");
      	if (debugON) Serial.println(rBuffer);
      	if (logON) log("Error in reply: ");
      	if (logON) log(rBuffer);
      }
      client.stop();
    }
    free(db);
  }
}


//worker thread send on http*****************************
  void *pthread_httpSend(void *ptr) {  // if its the child process
    int fd = ramopen(tempseqid, tempsendingIter);  // store the file descriptor for the child file
    
    if(client.connect(httpServer, 80)) {	// Connecting to server to retrieve "sequence id"
    	if (debugON) Serial.print("Requesting SEQID to:");
    	if (debugON) Serial.println(httpServer);
      
      char rBuffer[300];
      int rsize = prepareFirstBuffer(rBuffer, db, td);  // prepare the info for the new entry into the DB
    
      client.print("POST ");
      client.print(path_domain);
      client.print("/device.php?op=put&mac=");
      for(int m=0; m < 6; m++) {
        if(mac[m] < 0x10) client.print("0");
        client.print(mac[m], HEX);
      }
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(httpServer);
      client.println("Content-Type: text/plain");
      client.print("Content-Length: ");
      client.println(rsize);
      client.println("Connection: close");
      client.println("");
      client.print(rBuffer);
      // Reading headers
      while(!client.available()){;} // WAITING FOR DATA ******************
      int s = getLine(client, rBuffer, 300);
      if(strncmp(rBuffer, "HTTP/1.1 200", 12) == 0) {//  server RESPONSE OK
        int bodySize = 0;
        do {
          s = getLine(client, rBuffer, 300);
          if(strncmp(rBuffer, "Content-Length", 14) == 0) { // GETTING body size data
            char* separator = strchr(rBuffer, ':');
            if(*(separator+1) == ' ') {
              separator += 2;
            } else {
              separator++;
            }
            bodySize = atoi(separator);   //RESPONSE BODY SIZE
          }
        } while(s > 0);
        
        // Content
        s = getLine(client, rBuffer, 300, bodySize);
        
        char* separator = strchr(rBuffer, ';'); //reading value after "seqid"
        *separator = 0;
        seqid = atol(rBuffer);  // get the sequence ID
        nextContact = atol(separator+1) + getUNIXTime();// get time next contact (60 sec)
        if (debugON) Serial.print("SEQID:");
        if (debugON) Serial.println(seqid);
        if (logON) log("SEQID:");
        if (logON) log(rBuffer);

        if (debugON) Serial.print("Next Contact scheduled for: ");
        debugUNIXTime(nextContact);
        inEvent = 1;
        
        sendingIter = 0;
        seqDBfd = ramopen(seqid, sendingIter);    // aperto file in ram per il nuovo Evento
        if ((debugON) && (seqDBfd ==-1)) Serial.print("Error in ramopen: ");
        if (logON) log("Error in ramopen: httpSendValues");
      } else {
      	if (debugON) Serial.print("Error in reply: ");
      	if (debugON) Serial.println(rBuffer);
      	if (logON) log("Error in reply: ");
      	if (logON) log(rBuffer);
      }
      client.stop();
    }
    free(db);
  }
}

//worker thread send on http*****************************
  void *pthread_httpSend(void *ptr) {  // if its the child process
    int fd = ramopen(tempseqid, tempsendingIter);  // store the file descriptor for the child file
    if ((debugON) && (fd ==-1)) Serial.print("Error in ramopen: ");
    int size = lseek(fd, 0, SEEK_END);  // get the size of the file
    lseek(fd, 0, SEEK_SET);  // set the pointer to the beginning of the file
    
    char *sendBuffer = (char*) malloc(0);
    int offset = 0;
    int r = 0;
    unsigned int totalValues = 0;
    boolean isLast = true;
    do {  // for as long as there is something to read
      struct RECORD *rec = (struct RECORD*)malloc(sizeof(struct RECORD));
      r = read(fd, rec, sizeof(struct RECORD));
      if(r > 0) {
        if(rec->overThreshold) isLast = false;
        totalValues++;
        
        char *rBuffer = (char *)malloc(300 * sizeof(char));
        int ls = prepareBuffer(rBuffer, rec);  // get the length of the string and store the string into the buffer
        sendBuffer = (char*) realloc(sendBuffer, offset+ls);
        memcpy(sendBuffer + offset, rBuffer, ls);
        offset += ls;
        free(rBuffer);
      }
      free(rec);
    } while(r > 0);  // for as long as there is something to read
    sendBuffer = (char*) realloc(sendBuffer, offset+1);
    sendBuffer[offset] = 0;
    
    if(client.connect(httpServer, 80)) {
    	if (debugON) Serial.print("Current time: ");
      debugUNIXTime(nextContact);
      
      if (debugON) {
      	Serial.print("Sending ");
      	Serial.print(totalValues);
      	Serial.print(" values to:");
      	Serial.println(httpServer);
      }
      
      client.print("POST ");
      client.print(path_domain);
      client.print("/device.php?op=put&mac=");
      for(int m=0; m < 6; m++) {
        if(mac[m] < 0x10) client.print("0");
        client.print(mac[m], HEX);
      }
      client.print("&seqid=");
      client.print(seqid);
      if(isLast) client.print("&last=1");
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(httpServer);
      client.println("Connection: close");
      client.print("Content-Length: ");
      client.println(offset);
      client.println("Content-Type: text/plain");
      client.println("");
      
      client.println(sendBuffer);
      
      char rBuffer[300];
      // Reading headers
      int s = getLine(client, rBuffer, 300);
      if(strncmp(rBuffer, "HTTP/1.1 200", 12) != 0) {
      	if (debugON) Serial.print("error in reply: ");
      	if (debugON) Serial.println(rBuffer);
      } else {
        int bodySize = 0;
        do {
          s = getLine(client, rBuffer, 300);
          if(strncmp(rBuffer, "Content-Length", 14) == 0) {
            char* separator = strchr(rBuffer, ':');
            if(*(separator+1) == ' ') {
              separator += 2;
            } else {
              separator++;
            }
            bodySize = atoi(separator);
          }
        } while(s > 0);
        // Content
        s = getLine(client, rBuffer, 300, bodySize);
        nextContactMapped = atol(rBuffer) + getUNIXTime();
        isLastMapped = isLast;

        if (debugON) Serial.println("done");
        if(isLast) {
          //inEvent = 0;
        	if (debugON) Serial.println("No more relevant values, ending now");
        	if (logON) log("No more relevant values, ending now");
        } else {
        	if (debugON) Serial.print("Next Contact scheduled for: ");
          debugUNIXTime(nextContact);
        }
      }
      if (debugON) Serial.println("closing connection... ");
      client.stop();
    }
    if (debugON) Serial.println("closed, freeing memory... ");
    free(sendBuffer);
    // RM file
    close(fd);
    ramunlink(tempseqid, tempsendingIter);
    
    isSendingMapped = false;

    if (debugON) Serial.println("PTHREAD DONE");
    pthread_exit(NULL);
  }

#endif

/* void realHttpSendValues() {
  close(seqDBfd);
  seqDBfd = -1;
  int tempseqid = seqid;
  int tempsendingIter = sendingIter;
  
  isLastMapped = (boolean*)mmap(NULL, sizeof(boolean), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
  isSendingMapped = (boolean*)mmap(NULL, sizeof(boolean), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
  nextContactMapped = (unsigned long*)mmap(NULL, sizeof(unsigned long), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
  
  *isLastMapped = true;
  *isSendingMapped = true;
  *nextContactMapped = 0;

  sendingIter++;
  seqDBfd = ramopen(seqid, sendingIter);
  int pid = fork();
  if(pid == 0) {  // if its the child process
    int fd = ramopen(tempseqid, tempsendingIter);  // store the file descriptor for the child file

    int size = lseek(fd, 0, SEEK_END);  // get the size of the file
    lseek(fd, 0, SEEK_SET);  // set the pointer to the beginning of the file

    char *sendBuffer = (char*) malloc(0);
    int offset = 0;
    int r = 0;
    unsigned int totalValues = 0;
    boolean isLast = true;
    do {  // for as long as there is something to read
      struct RECORD *rec = (struct RECORD*)malloc(sizeof(struct RECORD));
      r = read(fd, rec, sizeof(struct RECORD));
      if(r > 0) {
        if(rec->overThreshold) isLast = false;
        totalValues++;

        char *rBuffer = (char *)malloc(300 * sizeof(char));
        int ls = prepareBuffer(rBuffer, rec);  // get the length of the string and store the string into the buffer
        sendBuffer = (char*) realloc(sendBuffer, offset+ls);
        memcpy(sendBuffer + offset, rBuffer, ls);
        offset += ls;
        free(rBuffer);
      }
      free(rec);
    } while(r > 0);  // for as long as there is something to read
    sendBuffer = (char*) realloc(sendBuffer, offset+1);
    sendBuffer[offset] = 0;
    
    if(client.connect(httpServer, 80)) {
    	if (debugON) Serial.print("Current time: ");
      debugUNIXTime(nextContact);

      if (debugON) {
      	Serial.print("Sending ");
      	Serial.print(totalValues);
      	Serial.print(" values to:");
      	Serial.println(httpServer);
      }
      
      client.print("POST ");
      client.print(path_domain);
      client.print("/device.php?op=put&mac=");
      for(int m=0; m < 6; m++) {
        if(mac[m] < 0x10) client.print("0");
        client.print(mac[m], HEX);
      }
      client.print("&seqid=");
      client.print(seqid);
      if(isLast) client.print("&last=1");
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(httpServer);
      client.println("Connection: close");
      client.print("Content-Length: ");
      client.println(offset);
      client.println("Content-Type: text/plain");
      client.println("");
      
      client.println(sendBuffer);

      char rBuffer[300];
      // Reading headers
      int s = getLine(client, rBuffer, 300);
      if(strncmp(rBuffer, "HTTP/1.1 200", 12) != 0) {
      	if (debugON) Serial.print("error in reply: ");
      	if (debugON) Serial.println(rBuffer);
      } else {
        int bodySize = 0;
        do {
          s = getLine(client, rBuffer, 300);
          if(strncmp(rBuffer, "Content-Length", 14) == 0) {
            char* separator = strchr(rBuffer, ':');
            if(*(separator+1) == ' ') {
              separator += 2;
            } else {
              separator++;
            }
            bodySize = atoi(separator);
          }
        } while(s > 0);
        // Content
        s = getLine(client, rBuffer, 300, bodySize);
        *nextContactMapped = atol(rBuffer) + getUNIXTime();
        *isLastMapped = isLast;

        if (debugON) Serial.println("done");
        if(isLast) {
          //inEvent = 0;
        	if (debugON) Serial.println("No more relevant values, ending now");
        	if (logON) log("No more relevant values, ending now");
        } else {
        	if (debugON) Serial.print("Next Contact scheduled for: ");
          debugUNIXTime(nextContact);
        }
      }
      if (debugON) Serial.println("closing connection... ");
      client.stop();
    }
    if (debugON) Serial.println("closed, freeing memory... ");
    free(sendBuffer);
    // RM file
    close(fd);
    ramunlink(tempseqid, tempsendingIter);

    *isSendingMapped = false;
    
    munmap(isSendingMapped, sizeof(boolean));
    munmap(isLastMapped, sizeof(boolean));
    munmap(nextContactMapped, sizeof(unsigned long));
    
    if (debugON) Serial.println("done");
    exit(0);
  }
}
*/
