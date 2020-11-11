INCLUDE=-Idep/include
LIBDIR=-Ldep/lib
ADDSRC=dep/src/glad.c

CC=g++ -std=c++17
HARDFLAGS=-Weffc++
FLAGS=-pedantic-errors -Wall -Wextra -Wsign-conversion -Wno-error=unused-variable
CFLAGS=$(FLAGS) $(INCLUDE)
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
DIRS=$(shell mkdir -p bin)
LINUX_BINARY=bin/a.out


build-debug:
	$(DIRS)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -------------------- DEBUG BUILD --------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(CC) $(FLAGS) $(INCLUDE) -o $(LINUX_BINARY) $(ADDSRC) src/main.cpp $(LIBDIR) $(LDFLAGS)

build:
	$(DIRS)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info ----------------------- BUILD -----------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(CC) $(INCLUDE) -o $(LINUX_BINARY) $(ADDSRC) src/main.cpp $(LIBDIR) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f bin/*
