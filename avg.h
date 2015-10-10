#ifndef avg_h
#define avg_h

#define K 10  //Max buffer length

/**
 * Get average from array
 * @param bfr Array of values
 * @param currsize Array size
 * @return Average value
 */
int getAverage(int bfr[], int currsize);

/**
 * Add an X value to cyclic buffer and returns the new avg
 */
int getAvgX(int x);

/**
 * Add an Y value to cyclic buffer and returns the new avg
 */
int getAvgY(int y);

/**
 * Add an Z value to cyclic buffer and returns the new avg
 */
int getAvgZ(int z);

#endif
