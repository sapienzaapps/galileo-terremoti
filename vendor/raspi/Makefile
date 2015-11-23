

VENDOR_MODULES := LED ADXL345Accelerometer generic smbus
VENDOR_SOURCES := $(VENDOR_MODULES:%=vendor/${PLATFORM}/%.cpp)
VENDOR_OBJECTS := $(VENDOR_MODULES:%=${OBJDIR}/${PLATFORM}_%.o)

vendor:: $(VENDOR_OBJECTS)

${OBJDIR}/${PLATFORM}_%.o: vendor/${PLATFORM}/%.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

