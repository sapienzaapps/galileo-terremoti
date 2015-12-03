
CC        ?= gcc
CPP       ?= g++
CFLAGS     = ${MAINFLAGS}
CPPFLAGS   = ${MAINFLAGS} -g -Wall -std=c++11 -Ivendor/${PLATFORM}/
LFLAGS     = ${MAINFLAGS} -lm -lrt -lpthread -lwiringPi

ifneq (, ${SDLDEMO})
LFLAGS     = ${MAINFLAGS} -lm -lrt -lpthread -lwiringPi -lSDL2
endif

STRIP      = strip
