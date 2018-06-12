
include build-cfg.mk

OUTDIR        = $(shell pwd)/build/out_$(PLATFORM)$(VARIANT)
OBJDIR        = $(shell pwd)/build/tmp_$(PLATFORM)$(VARIANT)
BUILDOPTFILE  = $(shell pwd)/build/buildopt_$(PLATFORM)$(VARIANT)
BUILDVERSION := $(shell git describe --tags)
MAINFLAGS    := -DPLATFORM=\"${PLATFORM}\" -Wall -Wextra -pedantic-errors -fdiagnostics-show-option -Wno-unknown-pragmas -DBUILD_VERSION=\"${BUILDVERSION}\"

ifneq (, ${SDLDEMO})
MAINFLAGS += -DSDL_DEMO
endif

ifneq (, ${DEBUG})
MAINFLAGS += -g -rdynamic -DDEBUG
endif

ifneq (, ${DEBUG_SERVER})
MAINFLAGS += -DDEBUG_SERVER
endif

include vendor/${PLATFORM}/toolchain.mk

MODULES := CommandInterface Config Log Seismometer Utils galileo-core base64

ifneq (, ${SDLDEMO})
MODULES += LCDMonitor
endif

ifeq (${PLATFORM}, raspi)
NOWATCHDOG=y
endif

ifneq (, ${NOWATCHDOG})
MAINFLAGS += -DNOWATCHDOG
else
MODULES += Watchdog
endif

STOREDFLAGS := $(shell cat ${BUILDOPTFILE} 2>/dev/null)

SOURCES := $(MODULES:%=%.cpp)
OBJECTS := $(MODULES:%=${OBJDIR}/%.o)

ci: all

ifeq (${STOREDFLAGS}, ${MAINFLAGS})
all: createdir vendor net ${OUTDIR}/sketch.elf
else
all: clean createdir vendor net ${OUTDIR}/sketch.elf
endif

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
	echo -n '${MAINFLAGS}' > $(BUILDOPTFILE)

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
	rm -f ${OUTDIR}/* ${OBJDIR}/* $(BUILDOPTFILE)

