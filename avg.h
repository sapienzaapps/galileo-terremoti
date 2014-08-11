#ifndef avg_h
#define avg_h
#define K 10	//Max buffer length

int currsizeX = 0;
int currsizeY = 0;
int currsizeZ = 0;

int bfrX[K];
int bfrY[K];
int bfrZ[K];

// Calcola la media di un buffer
int getAverage(int bfr[], int currsize) {  
  int i;
  int sum=0;

  for (i = 0; i < currsize; i++)
    sum+=bfr[i];

  return (sum/currsize);	
}


// prende una rilevazione la mostra nel buffer e ritorna la media
int getAvgX(int x) {
  if(currsizeX >= K) {
    currsizeX = 0; 
    bfrX[currsizeX] = x; 
    currsizeX++;
  } else {
    bfrX[currsizeX] = x;
    currsizeX++;
  }
  return getAverage(bfrX, currsizeX);
}

int getAvgY(int y) {
  if(currsizeY >= K) {
    currsizeY = 0; 
    bfrY[currsizeY] = y; 
    currsizeY++;
  } else {
    bfrY[currsizeY] = y;
    currsizeY++;
  }
  return getAverage(bfrY, currsizeY);
}

int getAvgZ(int z) {
  if(currsizeZ >= K) {
    currsizeZ = 0; 
    bfrZ[currsizeZ] = z; 
    currsizeZ++;
  } else {
    bfrZ[currsizeZ] = z;
    currsizeZ++;
  }
  return getAverage(bfrZ,currsizeZ);
}





#endif
