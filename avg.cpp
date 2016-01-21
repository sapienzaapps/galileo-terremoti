
#include "avg.h"

int currsizeX = 0;
int currsizeY = 0;
int currsizeZ = 0;

float bfrX[K];
float bfrY[K];
float bfrZ[K];

float getAverage(float bfr[], int currsize) {
	int i;
	int sum = 0;

	for (i = 0; i < currsize; i++)
		sum += bfr[i];

	return (sum / currsize);
}

float getAvgX(float x) {
	if (currsizeX >= K) {
		currsizeX = 0;
		bfrX[currsizeX] = x;
		currsizeX++;
	} else {
		bfrX[currsizeX] = x;
		currsizeX++;
	}
	return getAverage(bfrX, currsizeX);
}

float getAvgY(float y) {
	if (currsizeY >= K) {
		currsizeY = 0;
		bfrY[currsizeY] = y;
		currsizeY++;
	} else {
		bfrY[currsizeY] = y;
		currsizeY++;
	}
	return getAverage(bfrY, currsizeY);
}

float getAvgZ(float z) {
	if (currsizeZ >= K) {
		currsizeZ = 0;
		bfrZ[currsizeZ] = z;
		currsizeZ++;
	} else {
		bfrZ[currsizeZ] = z;
		currsizeZ++;
	}
	return getAverage(bfrZ, currsizeZ);
}
