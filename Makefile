
include build-cfg.mk

OUTDIR        = $(shell pwd)/build/out_$(PLATFORM)$(VARIANT)
OBJDIR        = $(shell pwd)/build/tmp_$(PLATFORM)$(VARIANT)
BUILDVERSION := $(shell git describe --tags)
MAINFLAGS    := -DPLATFORM=\"${PLATFORM}\" -Wall -Wextra -pedantic-errors -fdiagnostics-show-option -Wno-unknown-pragmas -DBUILD_VERSION=\"${BUILDVERSION}\"

ifneq (, ${DEBUG})
MAINFLAGS += -g -rdynamic -DDEBUG
endif

ifneq (, ${DEBUG_SERVER})
MAINFLAGS += -DDEBUG_SERVER
endif

include vendor/${PLATFORM}/toolchain.mk

MODULES := avg CommandInterface Config Log Seismometer Utils galileo-core

ifneq (, ${NOWATCHDOG})
MAINFLAGS += -DNOWATCHDOG
else
MODULES += Watchdog
endif

SOURCES := $(MODULES:%=%.cpp)
OBJECTS := $(MODULES:%=${OBJDIR}/%.o)

all: createdir vendor net ${OUTDIR}/sketch.elf

ifeq (, ${REMOTEHOST})
upload: all
	$(error No REMOTEHOST specified)
else
upload: all
	scp $(OUTDIR)/sketch.elf root@${REMOTEHOST}:/sketch/sketch.new
	ssh root@${REMOTEHOST} "killall sketch.elf; mv /sketch/sketch.new /sketch/sketch.elf && reboot"

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

ifeq (, ${DEBUG})
${OUTDIR}/sketch.elf: $(OBJECTS) $(NET_OBJECTS) $(VENDOR_OBJECTS)
	${CPP} ${OBJDIR}/*.o ${LFLAGS} ${MAINFLAGS} -o ${OUTDIR}/sketch.elf
	${STRIP} ${OUTDIR}/sketch.elf
else
${OUTDIR}/sketch.elf: $(OBJECTS) $(NET_OBJECTS) $(VENDOR_OBJECTS)
	${CPP} ${OBJDIR}/*.o ${LFLAGS} ${MAINFLAGS} -o ${OUTDIR}/sketch.elf
endif

clean:
	rm -f ${OUTDIR}/* ${OBJDIR}/*

