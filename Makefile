
include build-cfg.mk

OUTDIR=$(shell pwd)/build/out_$(PLATFORM)$(VARIANT)
OBJDIR=$(shell pwd)/build/tmp_$(PLATFORM)$(VARIANT)
MAINFLAGS=-DPLATFORM=\"${PLATFORM}\" -Wno-unknown-pragmas -g -O3

include vendor/${PLATFORM}/toolchain.mk


MODULES := avg CommandInterface Config Log Seismometer Utils galileo-core
SOURCES := $(MODULES:%=%.cpp)
OBJECTS := $(MODULES:%=${OBJDIR}/%.o)

all: clean vendor net ${OUTDIR}/sketch.elf

vendor::

net::

include net/Makefile
include vendor/${PLATFORM}/Makefile

${OBJDIR}/%.o: %.cpp
	$(CPP) $(CPPFLAGS) ${MAINFLAGS} -c -o $@ $<

${OUTDIR}/sketch.elf: $(OBJECTS)
	${CPP} ${OBJDIR}/*.o ${LFLAGS} ${MAINFLAGS} -o ${OUTDIR}/sketch.elf

clean:
	mkdir -p $(OBJDIR)
	mkdir -p $(OUTDIR)
	rm -f ${OUTDIR}/* ${OBJDIR}/*

