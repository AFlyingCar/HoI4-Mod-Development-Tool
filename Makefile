
CXX=clang++

BUILD_DIR=build
OUT_DIR=bin
SRC_DIR=src
INC_DIR=inc

SOURCES=$(SRC_DIR)/BitMap.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/ShapeFinder.cpp $(SRC_DIR)/GraphicalDebugger.cpp $(SRC_DIR)/UniqueColorGenerator.cpp
OBJECTS=$(BUILD_DIR)/BitMap.o $(BUILD_DIR)/main.o $(BUILD_DIR)/ShapeFinder.o $(BUILD_DIR)/GraphicalDebugger.o $(BUILD_DIR)/UniqueColorGenerator.o
INCLUDES=$(INC_DIR)/BitMap.h $(INC_DIR)/ShapeFinder.h $(INC_DIR)/GraphicalDebugger.h $(INC_DIR)/UniqueColorGenerator.h

ifndef NODEBUG
DEBUG_FLAG=-g
else
DEBUG_FLAG=-O3 -flto=thin
LFLAGS=-flto=thin #-fuse-ld=gold
endif

ENABLE_GRAPHICS_C=-DENABLE_GRAPHICS `sdl2-config --cflags`
ENABLE_GRAPHICS_L=-DENABLE_GRAPHICS `sdl2-config --libs`

INCFLAGS+=-I$(INC_DIR)/
CXXFLAGS+=$(DEBUG_FLAG) $(ENABLE_GRAPHICS_C) -std=c++14 $(INCFLAGS)
LFLAGS+=-pthread $(ENABLE_GRAPHICS_L)

OUT=$(OUT_DIR)/fp

all: $(OBJECTS) $(OUT_DIR)/
	$(CXX) $(LFLAGS) $(OBJECTS) -o $(OUT)

clean:
	rm $(OBJECTS) $(OUT)

$(BUILD_DIR)/BitMap.o: $(INC_DIR)/BitMap.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/BitMap.cpp -o $(BUILD_DIR)/BitMap.o

$(BUILD_DIR)/main.o: $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/ShapeFinder.o: $(INC_DIR)/ShapeFinder.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/ShapeFinder.cpp -o $(BUILD_DIR)/ShapeFinder.o

$(BUILD_DIR)/GraphicalDebugger.o: $(INC_DIR)/GraphicalDebugger.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/GraphicalDebugger.cpp -o $(BUILD_DIR)/GraphicalDebugger.o

$(BUILD_DIR)/UniqueColorGenerator.o: $(INC_DIR)/UniqueColorGenerator.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/UniqueColorGenerator.cpp -o $(BUILD_DIR)/UniqueColorGenerator.o

$(OUT_DIR)/:
	mkdir -p $(OUT_DIR)

$(BUILD_DIR)/:
	mkdir -p $(BUILD_DIR)

