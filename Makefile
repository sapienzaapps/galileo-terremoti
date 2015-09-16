
include build-cfg.mk

OUTDIR=$(shell pwd)/build/out_$(PLATFORM)$(VARIANT)
OBJDIR=$(shell pwd)/build/tmp_$(PLATFORM)$(VARIANT)
MAINFLAGS=-DPLATFORM=\"${PLATFORM}\" -Wno-unknown-pragmas -g

include vendor/${PLATFORM}/toolchain.mk


MODULES := avg CommandInterface Config Log Seismometer Utils Watchdog galileo-core
SOURCES := $(MODULES:%=%.cpp)
OBJECTS := $(MODULES:%=${OBJDIR}/%.o)

all: createdir vendor net ${OUTDIR}/sketch.elf

createdir:
	mkdir -p $(OBJDIR)
	mkdir -p $(OUTDIR)

vendor::

net::

include net/Makefile
include vendor/${PLATFORM}/Makefile

${OBJDIR}/%.o: %.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OUTDIR}/sketch.elf: $(OBJECTS)
	${CPP} ${OBJDIR}/*.o ${LFLAGS} ${MAINFLAGS} -o ${OUTDIR}/sketch.elf

clean:
	rm -f ${OUTDIR}/* ${OBJDIR}/*

