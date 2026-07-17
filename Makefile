CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I src/
TARGET = zelda-lttp-decomp

SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/snes/hardware.cpp \
          $(SRC_DIR)/snes/ppu.cpp \
          $(SRC_DIR)/snes/dma.cpp \
          $(SRC_DIR)/snes/input.cpp \
          $(SRC_DIR)/game/state.cpp \
          $(SRC_DIR)/game/sprites.cpp \
          $(SRC_DIR)/game/collision.cpp \
          $(SRC_DIR)/game/room.cpp \
          $(SRC_DIR)/audio/apu.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)/snes $(BUILD_DIR)/game $(BUILD_DIR)/audio

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)