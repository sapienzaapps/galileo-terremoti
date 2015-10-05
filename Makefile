
include build-cfg.mk

OUTDIR    := $(shell pwd)/build/out_$(PLATFORM)$(VARIANT)
OBJDIR    := $(shell pwd)/build/tmp_$(PLATFORM)$(VARIANT)
MAINFLAGS := -DPLATFORM=\"${PLATFORM}\" -Wno-unknown-pragmas -g

ifneq (, ${DEBUG})
MAINFLAGS += -DDEBUG
endif

include vendor/${PLATFORM}/toolchain.mk

MODULES := avg CommandInterface Config Log Seismometer Utils Watchdog galileo-core
SOURCES := $(MODULES:%=%.cpp)
OBJECTS := $(MODULES:%=${OBJDIR}/%.o)

all: createdir vendor net ${OUTDIR}/sketch.elf

ifeq (, ${REMOTEHOST})
upload: all
	$(error No REMOTEHOST specified)
else
upload: all
	scp $(OUTDIR)/sketch.elf root@${REMOTEHOST}:/sketch/sketch.new
	ssh root@${REMOTEHOST} "killall sketch.elf && mv /sketch/sketch.new /sketch/sketch.elf && reboot"

endif

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
	${STRIP} ${OUTDIR}/sketch.elf

clean:
	rm -f ${OUTDIR}/* ${OBJDIR}/*

