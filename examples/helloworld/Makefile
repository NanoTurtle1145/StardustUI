CXX ?= g++
CC ?= gcc
LD ?= ld

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/helloworld.elf
APP_OUT := ../../../../out/helloworld.elf
STAGING_APPS_DIR := ../../../../out/XJ380/apps

FRAMEWORK_DIR := ../..
ROOT_DIR := ../../../..
XAPI_DIR := $(ROOT_DIR)/user/xapi
XAPI_OUT_DIR := $(ROOT_DIR)/out/xapi
XAPI_START_OBJECT := $(XAPI_OUT_DIR)/start.cpp.o
XAPI_CORE_OBJECTS = $(shell find $(XAPI_OUT_DIR) -maxdepth 1 -name '*.o' | grep -v 'constart.cpp.o' | grep -v 'start.cpp.o')

SRC_FILES := \
	helloworld.cpp \
	$(FRAMEWORK_DIR)/src/window.cpp \
	$(FRAMEWORK_DIR)/src/platforms/xj380.cpp \
	$(FRAMEWORK_DIR)/src/components/base.cpp \
	$(FRAMEWORK_DIR)/src/components/lable.cpp

OBJ_FILES := \
	$(OBJ_DIR)/helloworld.o \
	$(OBJ_DIR)/framework/window.o \
	$(OBJ_DIR)/framework/platforms/xj380.o \
	$(OBJ_DIR)/framework/components/base.o \
	$(OBJ_DIR)/framework/components/lable.o

XAPI_HEADERS := $(shell find $(XAPI_DIR)/include -type f \( -name '*.h' -o -name '*.hpp' \))
FRAMEWORK_HEADERS := $(shell find $(FRAMEWORK_DIR) -type f \( -name '*.h' -o -name '*.hpp' \))

CPPFLAGS += -I$(FRAMEWORK_DIR) -I$(FRAMEWORK_DIR)/includes
CXXFLAGS ?= -O0 -Wall -g -ffreestanding -fno-builtin -m64 -mstackrealign -std=gnu++17 -fno-stack-protector -fno-exceptions -fno-rtti -fshort-wchar -nostdinc -Wno-write-strings
LDFLAGS ?= -Ttext=0x200000 -e xapi_start

.PHONY: all clean copy xapi

all: $(TARGET)

xapi:
	@$(MAKE) -C $(XAPI_DIR) build_all_api

$(TARGET): xapi $(OBJ_FILES)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) $(XAPI_START_OBJECT) $(XAPI_CORE_OBJECTS) $(XAPI_OUT_DIR)/liballoc-x86_64.a $(OBJ_FILES) -o $@
	@echo "LINK $(TARGET)"

$(OBJ_DIR)/helloworld.o: helloworld.cpp $(FRAMEWORK_HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

$(OBJ_DIR)/framework/window.o: $(FRAMEWORK_DIR)/src/window.cpp $(FRAMEWORK_HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

$(OBJ_DIR)/framework/platforms/xj380.o: $(FRAMEWORK_DIR)/src/platforms/xj380.cpp $(FRAMEWORK_HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

$(OBJ_DIR)/framework/components/base.o: $(FRAMEWORK_DIR)/src/components/base.cpp $(FRAMEWORK_HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

$(OBJ_DIR)/framework/components/lable.o: $(FRAMEWORK_DIR)/src/components/lable.cpp $(FRAMEWORK_HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "CPP $< -> $@"

copy: $(TARGET)
	@mkdir -p $(dir $(APP_OUT))
	@cp -a $(TARGET) $(APP_OUT)
	@if [ -d "$(STAGING_APPS_DIR)" ]; then cp -a $(TARGET) $(STAGING_APPS_DIR)/helloworld.elf; fi
	@echo "COPY $(TARGET) -> $(APP_OUT)"

clean:
	rm -rf $(BUILD_DIR)
