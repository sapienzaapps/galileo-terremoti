#ifndef __THRESHOLD_H
#define __THRESHOLD_H

#include "AcceleroMMA7361.h"
#include "Log.h"
#include "Config.h"


// accelerometer values
extern double pthresx;
extern double pthresy;
extern double pthresz;
extern double nthresx;
extern double nthresy;
extern double nthresz;

float absavg(int *buf, int size);
double stddev(int *buf, int size, float avg);
void setThresholdValues(AcceleroMMA7361 ac, int currentHour);
void checkCalibrationNeededNOSD(AcceleroMMA7361 ac, int currentHour);
void showThresholdValues();

#endif