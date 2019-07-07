
CXX=clang++

BUILD_DIR=build
OUT_DIR=bin
SRC_DIR=src
INC_DIR=inc

CXXSOURCES=$(SRC_DIR)/BitMap.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/ShapeFinder.cpp $(SRC_DIR)/GraphicalDebugger.cpp $(SRC_DIR)/UniqueColorGenerator.cpp $(SRC_DIR)/ProvinceMapBuilder.cpp $(SRC_DIR)/Util.cpp
CXXOBJECTS=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CXXSOURCES))
# OBJECTS=$(BUILD_DIR)/BitMap.o $(BUILD_DIR)/main.o $(BUILD_DIR)/ShapeFinder.o $(BUILD_DIR)/GraphicalDebugger.o $(BUILD_DIR)/UniqueColorGenerator.o $(BUILD_DIR)/ProvinceMapBuilder.o $(BUILD_DIR)/Util.o $(BUILD_DIR)/ColorArray.o
INCLUDES=$(INC_DIR)/BitMap.h $(INC_DIR)/ShapeFinder.h $(INC_DIR)/GraphicalDebugger.h $(INC_DIR)/UniqueColorGenerator.h $(INC_DIR)/ProvinceMapBuilder.h $(INC_DIR)/Util.h
COLOR_BINS=lakes.bin lands.bin unknowns.bin seas.bin

# TODO: Add support for other architectures
ASMSOURCES=$(SRC_DIR)/ColorArray_x86.asm
ASMOBJECTS=$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASMSOURCES))

COLOR_GEN=color_generator.py

ifndef NODEBUG
DEBUG_FLAG=-g
else
DEBUG_FLAG=-O3 -flto=thin
LFLAGS=-flto=thin #-fuse-ld=gold
endif

WFLAGS=-Wall -Werror -Wextra -Wno-unused-command-line-argument -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter

ifeq ($(OS),Windows_NT)
SDL_CFLAGS=-I./SDL/ -D_REENTRANT
SDL_LFLAGS=-lSDL2 -lsdl2main

CXXFLAGS+=-Xclang -flto-visibility-public-std -m32
LFLAGS+=-m32

PYTHON=python

else
SDL_CFLAGS:=`sdl2-config --cflags`
SDL_LFLAGS:=`sdl2-config --libs`

LFLAGS+=-lstdc++fs

PYTHON=python3

endif

ENABLE_GRAPHICS_C=-DENABLE_GRAPHICS $(SDL_CFLAGS)
ENABLE_GRAPHICS_L=-DENABLE_GRAPHICS $(SDL_LFLAGS)

CPP_VER=-std=c++17

INCFLAGS+=-I$(INC_DIR)/
CXXFLAGS+=$(DEBUG_FLAG) $(ENABLE_GRAPHICS_C) $(CPP_VER) $(INCFLAGS) $(WFLAGS)
LFLAGS+=-pthread $(ENABLE_GRAPHICS_L) $(CPP_VER)

OUT=$(OUT_DIR)/fp

.PHONY: all clean color_files

all: $(ASMOBJECTS) $(CXXOBJECTS) |$(OUT_DIR)/
	$(CXX) $(ASMOBJECTS) $(CXXOBJECTS) $(LFLAGS) -o $(OUT)

clean:
	rm $(CXXOBJECTS) $(OUT)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ColorArray_x86.o: $(SRC_DIR)/ColorArray_x86.asm $(INC_DIR)/ColorArray.h color_files
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/ColorArray_x86.asm -o $@

$(CXXOBJECTS) $(ASMOBJECTS): |$(BUILD_DIR)

color_files: |$(COLOR_GEN)
	$(PYTHON) $(COLOR_GEN)

$(OUT_DIR)/:
	mkdir -p $(OUT_DIR)

$(BUILD_DIR)/:
	mkdir -p $(BUILD_DIR)

