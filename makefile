CC = gcc
PROGRAM = boids
SRC_PATH = src
INC_PATH = include
RAYLIB_PATH = /home/pedro/software/raylib/src

CFLAGS = -Iinclude -I$(RAYLIB_PATH) -Wall -Wextra -Wshadow -g
LFLAGS = -L$(RAYLIB_PATH) -lraylib -lm

_SRC = main.c boids.c boids_ui.c
_DEPS = boids.h boids_ui.h

SRC = $(_SRC:%=$(SRC_PATH)/%)
DEPS = $(_DEPS:%=$(INC_PATH)/%) $(RAYLIB_PATH)/raylib.h $(RAYLIB_PATH)/raymath.h

$(PROGRAM): $(SRC) $(DEPS)
	$(CC) -o $@ $(SRC) $(CFLAGS) $(LFLAGS)

all: $(PROGRAM)

purge:
	rm -f $(PROGRAM)
