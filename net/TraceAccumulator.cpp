//
// Created by Enrico on 24/03/16.
//

#include "TraceAccumulator.h"
#include "HTTPClient.h"
#include "../Log.h"
#include "SCSAPI.h"
#include "../generic.h"

FILE *TraceAccumulator::traceFile = NULL;
unsigned long TraceAccumulator::traceStartedAt = 0;

void TraceAccumulator::traceValue(unsigned long ts, float val, float threshold, float avg,
								  float stddev, float sigma) {
#ifdef TRACEACCUMULATOR_FILE
	if (traceStartedAt != 0 && traceStartedAt < (getUNIXTime() - (60 * 15))) {
		setTrace(false);
		traceStartedAt = 0;
	} else if (traceFile != NULL) {
		fwrite(&ts, sizeof(unsigned long), 1, traceFile);
		fwrite(&val, sizeof(float), 1, traceFile);
		fwrite(&threshold, sizeof(float), 1, traceFile);
		fwrite(&avg, sizeof(float), 1, traceFile);
		fwrite(&stddev, sizeof(float), 1, traceFile);
		fwrite(&sigma, sizeof(float), 1, traceFile);
	}
#endif
}

void TraceAccumulator::setTrace(bool v) {
#ifdef TRACEACCUMULATOR_FILE
	if (!v && traceFile != NULL) {

		Log::d("Disabling trace");
		fclose(traceFile);
		traceFile = NULL;

		Log::d("Sending trace file to server");
		HTTPResponse *resp = HTTPClient::httpPostFile(
				std::string("http://www.seismocloud.com/trace/push.php?deviceid=") + Config::getMacAddress(),
				std::string(TRACEACCUMULATOR_FILE)
		);

		Log::d("Trace file sent - HTTP Status: %i", resp->responseCode);
		delete resp;

		unlink(TRACEACCUMULATOR_FILE);
	} else if (v && traceFile == NULL) {
		if (unlink(TRACEACCUMULATOR_FILE) !== 0) {
			Log::e("Cannot open trace file");
			return;
		}
		traceFile = fopen(TRACEACCUMULATOR_FILE, "w");
		if (tracefile != NULL) {
			traceStartedAt = getUNIXTime();
			char head[3];
			head[0] = 1; // Version
			head[1] = sizeof(unsigned long);
			head[2] = sizeof(float);
			fwrite(head, 3, 1, traceFile);
			Log::d("Trace started");
		}
	}
#endif
}
