
ifneq (, ${CXX})
CPP=${CXX}
endif

CC        ?= gcc
CPP       ?= g++
CFLAGS     = ${MAINFLAGS}
CPPFLAGS   = ${MAINFLAGS} -g -Wall -std=c++11 -Ivendor/${PLATFORM}/
LFLAGS     = ${MAINFLAGS} -lm -lrt -lpthread

ifneq (, ${SDLDEMO})
LFLAGS     = ${MAINFLAGS} -lm -lrt -lpthread -lSDL2
endif

STRIP      = strip

