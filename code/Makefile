CC = gcc
CFLAGS = -Wall -g `pkg-config clutter-1.0 --cflags`
LDFLAGS = `pkg-config clutter-1.0 --libs` -lm
OBJS = foo-fader.o main.o

all : fader

foo-fader.o : foo-fader.h

main.o : foo-fader.h

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fader : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

.PHONY : all
