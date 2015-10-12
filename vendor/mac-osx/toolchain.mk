
MAINFLAGS := $(subst -rdynamic,,${MAINFLAGS})

CC         = gcc
CPP        = g++
CFLAGS     = ${MAINFLAGS}
CPPFLAGS   = ${MAINFLAGS} -g -std=c++11 -Ivendor/${PLATFORM}/
LFLAGS     = ${MAINFLAGS} -lm
STRIP      = strip