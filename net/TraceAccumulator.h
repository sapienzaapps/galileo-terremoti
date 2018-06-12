//
// Created by Enrico on 24/03/16.
//

#ifndef GALILEO_TERREMOTI_TRACEACCUMULATOR_H
#define GALILEO_TERREMOTI_TRACEACCUMULATOR_H

#include <cstdint>
#include <cstdio>
#include "SCSAPI.h"

/**
 * Trace file format:
 * At the beginning of file you'll find 1 byte indicating the version (currently 1), then 3 bytes which indicates
 * the size of timestamp, value and threshold fields. After that you'll find a series of triplets for timestamp (integer,
 * UNIX time in seconds), value (float value) and threshold (float value)
 */

class TraceAccumulator {

public:
	static void traceValue(unsigned long ts, float val, float threshold,
						   float avg, float stddev, float sigma);
	static void setTrace(bool v);

private:
	static FILE* traceFile;
	static unsigned long traceStartedAt;

};


#endif //GALILEO_TERREMOTI_TRACEACCUMULATOR_H
