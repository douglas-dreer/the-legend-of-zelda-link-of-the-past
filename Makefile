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
          $(SRC_DIR)/audio/apu.cpp \
          $(SRC_DIR)/engine/reset.cpp \
          $(SRC_DIR)/engine/nmi.cpp \
          $(SRC_DIR)/engine/main_loop.cpp \
          $(SRC_DIR)/data/palettes.cpp \
          $(SRC_DIR)/data/tiles.cpp \
          $(SRC_DIR)/data/maps.cpp \
          $(SRC_DIR)/data/dungeon_data.cpp \
          $(SRC_DIR)/data/compression.cpp \
          $(SRC_DIR)/data/music_data.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# ---- Test suite ----
TEST_DIR = $(SRC_DIR)/test
TEST_BUILD_DIR = $(BUILD_DIR)/test
TEST_TARGET = zelda-test

TEST_SOURCES = $(TEST_DIR)/test_main.cpp \
               $(TEST_DIR)/test_palettes.cpp \
               $(TEST_DIR)/test_tiles.cpp \
               $(TEST_DIR)/test_audio.cpp \
               $(TEST_DIR)/test_game_state.cpp \
               $(TEST_DIR)/test_dma.cpp \
               $(TEST_DIR)/test_sprites.cpp

# Test sources link against the palette data (ALL_PALETTES etc.)
TEST_LIB_SOURCES = $(SRC_DIR)/data/palettes.cpp
TEST_LIB_OBJECTS = $(TEST_LIB_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.cpp=$(TEST_BUILD_DIR)/%.o)

.PHONY: all clean dirs test

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)/snes $(BUILD_DIR)/game $(BUILD_DIR)/audio $(BUILD_DIR)/engine $(BUILD_DIR)/data
	@mkdir -p $(TEST_BUILD_DIR) output/test

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c -o $@ $<

$(TEST_TARGET): dirs $(TEST_OBJECTS) $(TEST_LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJECTS) $(TEST_LIB_OBJECTS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)
