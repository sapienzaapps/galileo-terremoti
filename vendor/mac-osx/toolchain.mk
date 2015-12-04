
MAINFLAGS := $(subst -rdynamic,,${MAINFLAGS})

CC         = gcc
CPP        = g++
CFLAGS     = ${MAINFLAGS}
CPPFLAGS   = ${MAINFLAGS} -g -std=c++11 -Ivendor/${PLATFORM}/ -I${HOME}/homebrew/Cellar/sdl2/2.0.3/include/
LFLAGS     = ${MAINFLAGS} -lm

ifneq (, ${SDLDEMO})
LFLAGS     = ${MAINFLAGS} -lm -lpthread -lSDL2 -L${HOME}/homebrew/Cellar/sdl2/2.0.3/lib/
endif

STRIP      = strip