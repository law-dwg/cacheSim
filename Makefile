TARGET := cache-replacement-simulator

BUILD_DIR := ./build

# Main
SRC_MAIN := ./main.cpp
OBJ_MAIN := $(BUILD_DIR)/main.o

# CPP code
SRC_UTILS := ./utils
SRC_PROGRAMS := ./programs
SRC_CACHE := ./cache
SRC_CACHE_REPLACEMENT_POLICIES := ./policies
CC = g++
CC_FLAGS = -std=c++17
CC_LINKER_FLAGS = -lstdc++fs
CC_DEBUG_FLAGS = -g

SOURCES_CPP = 	$(SRC_UTILS)/utils.cpp \
				$(SRC_PROGRAMS)/matMul.cpp

SOURCES_HPP = 	$(SRC_UTILS)/utils.hpp \
				$(SRC_UTILS)/wrappers.hpp \
				$(SRC_UTILS)/logger.hpp \
				$(SRC_PROGRAMS)/sort.hpp \
				$(SRC_PROGRAMS)/matMul.hpp \
				$(SRC_CACHE)/cache.hpp \
				$(SRC_CACHE)/../cache/set.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/fifo.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/lfru.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/lfu.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/lru.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/mru.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/plru.hpp \
				$(SRC_CACHE_REPLACEMENT_POLICIES)/slru.hpp

# OBJS_CPP := $(SOURCES_CPP:%=$(BUILD_DIR)/%.o)

# Main build
$(BUILD_DIR)/$(TARGET): $(BUILD_DIR) $(SRC_MAIN) $(SOURCES_CPP) $(SOURCES_HPP)
	$(CC) $(CC_DEBUG_FLAGS) $(CC_FLAGS) $(SRC_MAIN) $(SOURCES_CPP) $(CC_LINKER_FLAGS) -o $@

$(BUILD_DIR): $(RESULTS_DIR)
	mkdir $@

# PHONYS
.PHONY: all clean run format

all: clean format run

clean:
	rm -r $(BUILD_DIR) || true

run: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET)
