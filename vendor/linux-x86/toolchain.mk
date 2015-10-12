
CC         = gcc
CPP        = g++
CFLAGS     = ${MAINFLAGS}
CPPFLAGS   = ${MAINFLAGS} -g -Wall -std=c++11 -Ivendor/${PLATFORM}/
LFLAGS     = ${MAINFLAGS} -lm -lrt -lpthread
STRIP      = strip
