CC = gcc
SDLC2_FLAGS = `pkg-config --cflags sdl2`
CFLAGS = -Wall -Wextra -pedantic $(SDLC2_FLAGS)
LIBS = -lm `pkg-config --libs sdl2`

bezier: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)


