CXX ?= clang++
AR ?= ar

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/libStardustUI.a 

SRC_FILES := \
	src/window.cpp \
	src/platforms/xj380.cpp \
	src/components/base.cpp \
	src/components/lable.cpp

OBJ_FILES := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

CPPFLAGS += -I. -I./includes
CXXFLAGS ?= -O0 -g -std=gnu++17 -Wall -Wextra -Wpedantic -Wwrite-strings -fno-builtin
ARFLAGS ?= rcs

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(OBJ_FILES)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
