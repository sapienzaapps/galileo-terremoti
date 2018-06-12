

VENDOR_MODULES := DummyAccelerometer
VENDOR_SOURCES := $(VENDOR_MODULES:%=vendor/${PLATFORM}/%.cpp)
VENDOR_OBJECTS := $(VENDOR_MODULES:%=${OBJDIR}/${PLATFORM}_%.o)

vendor:: $(VENDOR_OBJECTS) ${OBJDIR}/${PLATFORM}_generic.o ${OBJDIR}/${PLATFORM}_LED.o

${OBJDIR}/${PLATFORM}_LED.o: vendor/${PLATFORM}/LED.cpp LED.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/${PLATFORM}_generic.o: vendor/${PLATFORM}/generic.cpp generic.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/${PLATFORM}_%.o: vendor/${PLATFORM}/%.cpp vendor/${PLATFORM}/%.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

