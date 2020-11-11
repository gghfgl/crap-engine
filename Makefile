INCLUDE=-Idep/include
LIBDIR=-Ldep/lib
ADDSRC=dep/src/glad.c

CC=g++ -std=c++17
#CC=gcc
FLAGS=-pedantic-errors -Wall -Weffc++ -Wextra -Wsign-conversion
CFLAGS=$(FLAGS) $(INCLUDE)
LDFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
LINUX_BINARY=bin/a.out

EXEC=bin/a.out

build-debug:
	$(info -------------------- DEBUG BUILD --------------------)
	$(CC) $(FLAGS) $(INCLUDE) -o $(LINUX_BINARY) $(ADDSRC) src/*.cpp $(LIBDIR) $(LDFLAGS)

build:
	$(info -------------------- DEFAULT BUILD --------------------)
	$(CC) $(INCLUDE) -o $(LINUX_BINARY) $(ADDSRC) src/*.cpp $(LIBDIR) $(LDFLAGS)

clean:
	rm -f bin/*
