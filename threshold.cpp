#include "threshold.h"

#define CALIBRATIONITER 1000
#define ORANGEZONE 6

// accelerometer values
double pthresx = 0;
double pthresy = 0;
double pthresz = 0;
double nthresx = 0;
double nthresy = 0;
double nthresz = 0;
long nextHour = 0;

float absavg(int *buf, int size) {
	float ret = 0;
	for (int i = 0; i < size; i++) {
		ret += (buf[i] < 0 ? buf[i] * -1 : buf[i]);
	}
	return ret / size;
}

// Standard Deviation
double stddev(int *buf, int size, float avg) {
	// Formula: RAD ( SUM{i,size}( (x[i] - avg)^2 ) / (size - 1) )
	double sum = 0;
	for (int i = 0; i < size; i++) {
		sum += pow(buf[i] - avg, 2);
	}

	return sqrt(sum / (size - 1));
}

void setThresholdValues(AcceleroMMA7361 ac, int currentHour) {
	int cbufx[CALIBRATIONITER];
	int cbufy[CALIBRATIONITER];
	int cbufz[CALIBRATIONITER];

	Log::d("Begin calibration for hour: %i", currentHour);

	int i = 0;
	for (i = 0; i < CALIBRATIONITER; i++) {
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

void checkCalibrationNeededNOSD(AcceleroMMA7361 ac, int currentHour) {

	// do calibration every random amount of hours? or if it's the first time ever
	if ((nextHour == currentHour) || (pthresx <= 0.00) || forceInitEEPROM) {
		if (nextHour == currentHour)
			Log::i("nextHour = currentHour on SD #-#-#-#-#-#-#-#-#-#: %i", currentHour);

		if (pthresx <= 0)
			Log::i("pthresx <= 0 on NOSD #-#-#-#-#-#-#-#-#-#");
		//setThresholdValuesBasic(ac, currentHour);
		if (!yellowLedStatus) {
			digitalWrite(LED_YELLOW, yellowLedStatus ? (uint8_t) 0 : (uint8_t) 1);
			yellowLedStatus = !yellowLedStatus;
		}

		Log::d("WRITE THRESHOLD on SD #-#-#-#-#-#-#-#-#-#");
		setThresholdValues(ac, currentHour);
		nextHour = ((currentHour + 1) % 24);
		forceInitEEPROM = false;
	}
}

void showThresholdValues() {
	Log::i("Calibration on SD ended");
	Log::i("Positive thresholds X:%lf Y:%lf Z:%lf", pthresx, pthresy, pthresz);
	Log::i("Negative thresholds X:%lf Y:%lf Z:%lf", nthresx, nthresy, nthresz);
}
