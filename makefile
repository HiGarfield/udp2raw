# Determine target OS
CXX_TARGET := $(shell $(CXX) -dumpmachine)

TARGET_OS := $(strip $(if $(findstring -w64,$(CXX_TARGET)),Windows,\
                      $(if $(findstring -w32,$(CXX_TARGET)),Windows,\
                      $(if $(findstring -linux,$(CXX_TARGET)),Linux,\
                      $(if $(findstring -apple,$(CXX_TARGET)),MacOS,\
                      $(error "Target OS not supported."))))))
$(info TARGET_OS = $(TARGET_OS))

# Get Git version
UDP2RAW_GIT_VER ?= $(shell git rev-parse HEAD || echo unknown)
$(info UDP2RAW_GIT_VER = $(UDP2RAW_GIT_VER))

# Compiler flags
FLAGS := -std=c++11 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter \
          -Wno-missing-field-initializers
EXTRA_FLAGS := -Os -s
EXTRA_FLAGS += $(if $(filter MacOS,$(TARGET_OS)),,-ffunction-sections -Wl,--gc-sections)
EXTRA_FLAGS += $(if $(filter Windows,$(TARGET_OS)),-static,)

PCAP := $(if $(filter Windows,$(TARGET_OS)), \
            -isystem npcap/Include -lwpcap \
          $(if $(findstring x86_64,$(CXX_TARGET)),-Lnpcap/Lib/x64, \
          $(if $(findstring i386,$(CXX_TARGET)),-Lnpcap/Lib, \
          $(if $(findstring i486,$(CXX_TARGET)),-Lnpcap/Lib, \
          $(if $(findstring i586,$(CXX_TARGET)),-Lnpcap/Lib, \
          $(if $(findstring i686,$(CXX_TARGET)),-Lnpcap/Lib, \
          $(error "Target architecture not supported. CXX_TARGET is $(CXX_TARGET)")))))), \
          -lpcap)

MP := -DUDP2RAW_MP
COMMON := $(wildcard *.cpp lib/*.cpp)
LIBS := -lpthread -isystem libev
LIBS += $(if $(filter Windows,$(TARGET_OS)),-lws2_32)
LIBS += $(if $(filter Linux,$(TARGET_OS)),-lrt)

SOURCES := $(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
SOURCES_AES_ACC = $(COMMON) $(wildcard lib/aes_acc/aes*.c) lib/aes_acc/asm/$@.S
AES_ACC_TARGETS := $(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))
NAME := udp2raw
COMPILE_OPT := -I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS) -o $(NAME)

# Define targets
.PHONY: linux $(AES_ACC_TARGETS) pcap git_version clean

.DEFAULT_GOAL := $(if $(filter Linux,$(TARGET_OS)),linux,pcap)

# Build rules
linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(SOURCES_AES_ACC) $(COMPILE_OPT)

pcap: git_version
	$(CXX) $(SOURCES) $(PCAP) $(MP) $(COMPILE_OPT)

# Generate git version header
git_version:
	@printf 'const char *gitversion = "%s";\n' "$(UDP2RAW_GIT_VER)" > git_version.h

# Clean target
clean:
	-$(RM) $(NAME) git_version.h
