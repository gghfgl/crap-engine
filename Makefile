CXX=g++ -std=c++17
EXE=a.out

SOURCES=src/main.cpp
SOURCES+=dep/src/glad.c #TODO: make static lib?
SOURCES+=dep/include/IMGUI/*.cpp #TODO: make static lib?
SOURCES+=dep/include/igfd/*.cpp #TODO: make static lib?

CXXFLAGS=-pedantic-errors -Wall -Wextra -g
#CXXFLAGS+=-Weffc++ -Wsign-conversion

CXXFLAGS+=-Idep/include
CXXFLAGS+=-Ldep/lib
CXXFLAGS+=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp
CXXFLAGS+=-DIMGUI_IMPL_OPENGL_LOADER_GLAD

build:
	$(DIRS)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info ----------------------- BUILD -----------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(info -----------------------------------------------------)
	$(CXX) -o $(EXE) $(SOURCES) $(CXXFLAGS)

.PHONY: clean

valgrind:
	valgrind --leak-check=full --track-origins=yes ./a.out

clean:
	rm -f a.out
