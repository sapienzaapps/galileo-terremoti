#ifndef avg_h
#define avg_h

#define K 10  //Max buffer length

/**
 * Get average from array
 * @param bfr Array of values
 * @param currsize Array size
 * @return Average value
 */
float getAverage(float bfr[], int currsize);

/**
 * Add an X value to cyclic buffer and returns the new avg
 */
float getAvgX(float x);

/**
 * Add an Y value to cyclic buffer and returns the new avg
 */
float getAvgY(float y);

/**
 * Add an Z value to cyclic buffer and returns the new avg
 */
float getAvgZ(float z);

#endif
