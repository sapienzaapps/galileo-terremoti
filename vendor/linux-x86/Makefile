

VENDOR_MODULES := LED DummyAccelerometer vendor
VENDOR_SOURCES := $(VENDOR_MODULES:%=vendor/${PLATFORM}/%.cpp)
VENDOR_OBJECTS := $(VENDOR_MODULES:%=${OBJDIR}/${PLATFORM}_%.o)

vendor:: $(VENDOR_OBJECTS)

${OBJDIR}/${PLATFORM}_%.o: vendor/${PLATFORM}/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
