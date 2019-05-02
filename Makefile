
CXX=clang++

BUILD_DIR=build
OUT_DIR=bin

SOURCES=BitMap.cpp main.cpp ShapeFinder.cpp GraphicalDebugger.cpp
OBJECTS=$(BUILD_DIR)/BitMap.o $(BUILD_DIR)/main.o $(BUILD_DIR)/ShapeFinder.o $(BUILD_DIR)/GraphicalDebugger.o

DEBUG_FLAG=-g
ENABLE_GRAPHICS_C=-DENABLE_GRAPHICS `sdl2-config --cflags`
ENABLE_GRAPHICS_L=-DENABLE_GRAPHICS `sdl2-config --libs`

CXXFLAGS=$(DEBUG_FLAG) $(ENABLE_GRAPHICS_C) -std=c++14
LFLAGS=-pthread $(ENABLE_GRAPHICS_L)

OUT=$(OUT_DIR)/fp

all: $(OBJECTS) $(OUT_DIR)/
	$(CXX) $(LFLAGS) $(OBJECTS) -o $(OUT)

clean:
	rm $(OBJECTS) $(OUT)

$(BUILD_DIR)/BitMap.o: BitMap.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c BitMap.cpp -o $(BUILD_DIR)/BitMap.o

$(BUILD_DIR)/main.o: $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c main.cpp -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/ShapeFinder.o: ShapeFinder.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c ShapeFinder.cpp -o $(BUILD_DIR)/ShapeFinder.o

$(BUILD_DIR)/GraphicalDebugger.o: GraphicalDebugger.h $(BUILD_DIR)/
	$(CXX) $(CXXFLAGS) -c GraphicalDebugger.cpp -o $(BUILD_DIR)/GraphicalDebugger.o

$(OUT_DIR)/:
	mkdir -p $(OUT_DIR)

$(BUILD_DIR)/:
	mkdir -p $(BUILD_DIR)

