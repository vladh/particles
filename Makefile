TARGET=particles
BINPATH=bin/
CC=g++
LDFLAGS=-lm -lglfw -lglew -lpng -lz -lfreetype -lsfml-audio -lsfml-system
CFLAGS=-g -Wall -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -I/usr/local/Cellar/freetype/2.9.1/include/freetype2

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.hpp)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $(BINPATH)$@

clean:
	-rm -f *.o
	-rm -f $(BINPATH)$(TARGET)
