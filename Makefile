# LT's Aim Train 21 Makefile

CC = gcc
CFLAGS = -Wall -O2 -std=c99
TARGET = ltat21

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LIBS = -lglfw3 -lopengl32 -lglu32 -lgdi32
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        LIBS = -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LIBS = -lglfw -lGL -lGLU -lm
    endif
endif

# Source files
SRCS = src/main.c \
       src/core/window.c \
       src/core/renderer.c \
       src/game/game_state.c \
       src/graphics/primitives.c \
       src/graphics/hud.c \
       src/utils/vec3.c \
       src/utils/debug.c

# Object files
OBJS = $(SRCS:.c=.o)

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean