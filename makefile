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
UDP2RAW_GIT_VER_CODE := "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";"
$(info UDP2RAW_GIT_VER = $(UDP2RAW_GIT_VER))

# OpenSSL support detection
USE_OPENSSL ?= 1
ifeq ($(USE_OPENSSL), 1)
  OPENSSL_CFLAGS := $(shell pkg-config --cflags openssl 2>/dev/null || echo "-I/usr/include")
  OPENSSL_LIBS := $(shell pkg-config --libs openssl 2>/dev/null || echo "-lssl -lcrypto")
  $(info OpenSSL support: enabled)
  $(info OPENSSL_CFLAGS = $(OPENSSL_CFLAGS))
  $(info OPENSSL_LIBS = $(OPENSSL_LIBS))
else
  OPENSSL_CFLAGS :=
  OPENSSL_LIBS :=
  $(info OpenSSL support: disabled)
endif

# Compiler flags
FLAGS := -std=c++11 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter \
          -Wno-missing-field-initializers
ifeq ($(USE_OPENSSL), 1)
  FLAGS += -DUSE_OPENSSL $(OPENSSL_CFLAGS)
endif
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
ifeq ($(USE_OPENSSL), 1)
  LIBS += $(OPENSSL_LIBS)
endif

SOURCES := $(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
ifeq ($(USE_OPENSSL), 1)
  # Add OpenSSL wrapper when USE_OPENSSL=1, exclude built-in crypto when not needed
  SOURCES := $(filter-out lib/aes_faster_c/aes.cpp lib/aes_faster_c/wrapper.cpp, $(SOURCES))
endif

SOURCES_AES_ACC = $(COMMON) $(wildcard lib/aes_acc/aes*.c) lib/aes_acc/asm/$@.S
ifeq ($(USE_OPENSSL), 1)
  # For AES_ACC builds with OpenSSL, exclude built-in AES implementations
  SOURCES_AES_ACC := $(filter-out lib/aes_acc/aesacc.c lib/aes_acc/aesni.c lib/aes_acc/aesarm.c, $(SOURCES_AES_ACC))
endif

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
	@echo $(UDP2RAW_GIT_VER_CODE) > git_version.h

# Clean target
clean:
	-$(RM) $(NAME) git_version.h
