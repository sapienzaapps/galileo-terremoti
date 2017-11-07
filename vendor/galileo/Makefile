
VARIANT_MODULE   := variant
VARIANT_OBJECTS  := $(VARIANT_MODULE:%=${OBJDIR}/$(PLATFORM)_$(VARIANT)_%.o)

GALILEO_MODULES  := UtilTime wiring_analog wiring_digital trace sysfs mux TTYUART RingBuffer WString Print fast_gpio_sc \
	fast_gpio_common fast_gpio_nc WMath i2c interrupt IPAddress Tone Stream pulseIn UtilMisc
GALILEO_OBJECTS  := $(GALILEO_MODULES:%=${OBJDIR}/cores_$(VARIANT)_%.o)

VENDOR_MODULES   := AcceleroMMA7361
VENDOR_SOURCES   := $(VENDOR_MODULES:%=vendor/${PLATFORM}/%.cpp)
VENDOR_OBJECTS   := $(VENDOR_MODULES:%=${OBJDIR}/${PLATFORM}_%.o)

vendor:: $(VENDOR_OBJECTS) $(GALILEO_OBJECTS) $(VARIANT_OBJECTS) ${OBJDIR}/${PLATFORM}_generic.o ${OBJDIR}/${PLATFORM}_LED.o

${OBJDIR}/${PLATFORM}_LED.o: vendor/${PLATFORM}/LED.cpp LED.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/${PLATFORM}_generic.o: vendor/${PLATFORM}/generic.cpp generic.h
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/${PLATFORM}_%.o: vendor/${PLATFORM}/%.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/$(PLATFORM)_$(VARIANT)_%.o: $(SDK_ROOT)/hardware/intel/i586-uclibc/variants/$(VARIANT)/%.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/cores_$(VARIANT)_%.o: $(SDK_ROOT)/hardware/intel/i586-uclibc/cores/arduino/%.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OBJDIR}/cores_$(VARIANT)_%.o: $(SDK_ROOT)/hardware/intel/i586-uclibc/cores/arduino/%.c
	$(CC) $(CFLAGS) ${MAINFLAGS} -c -o $@ $<
