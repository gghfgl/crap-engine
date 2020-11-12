INCLUDE=-Idep/include
LIBDIR=-Ldep/lib
#ADDSRC=dep/src/glad.c

CC=g++ -std=c++17
HARDFLAGS=-Weffc++ -Wsign-conversion
FLAGS=-pedantic-errors -Wall -Wextra -Wno-error=unused-variable
CFLAGS=$(FLAGS) $(INCLUDE)
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp
DIRS=$(shell mkdir -p bin)
LINUX_BINARY=bin/a.out


build:
	$(DIRS)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info ----------------------- BUILD -----------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(CC) $(FLAGS) $(INCLUDE) -o $(LINUX_BINARY) src/main.cpp $(LIBDIR) $(LDFLAGS)

build-nodebug:
	$(DIRS)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info ------------------- NO WARNING ----------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(CC) $(INCLUDE) -o $(LINUX_BINARY) src/main.cpp $(LIBDIR) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f bin/*
