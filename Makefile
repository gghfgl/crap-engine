CXX=g++ -std=c++17
EXE=a.out

SOURCES=src/main.cpp
SOURCES+=dep/include/IMGUI/*.cpp #TODO: make static lib?

CXXFLAGS=-pedantic-errors -Wall -Wextra
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

clean:
	rm -f a.out
