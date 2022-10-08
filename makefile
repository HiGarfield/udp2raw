ifeq ($(OS),Windows_NT)
  HOST_OS:= Windows
else
  HOST_OS:= $(shell uname -s)
endif
$(info HOST_OS = $(HOST_OS))

CXX_TARGET:=$(shell $(CXX) -dumpmachine 2>&1)
ifneq (,$(findstring -w32,$(CXX_TARGET)))
  TARGET_OS:=Windows
else
  ifneq (,$(findstring -w64,$(CXX_TARGET)))
    TARGET_OS:=Windows
  else
    ifneq (,$(findstring -linux,$(CXX_TARGET)))
      TARGET_OS:=Linux
    else
      ifneq (,$(findstring -apple,$(CXX_TARGET)))
        TARGET_OS:=MacOS
      else
        $(error "Target OS not supported.")
      endif
    endif
  endif
endif
$(info TARGET_OS = $(TARGET_OS))

ifeq ($(HOST_OS),Windows)
  UDP2RAW_GIT_VER?=$(shell git rev-parse HEAD 2>nul)
else
  UDP2RAW_GIT_VER?=$(shell git rev-parse HEAD 2>/dev/null)
endif

ifeq ($(UDP2RAW_GIT_VER),)
  UDP2RAW_GIT_VER="unknown"
endif

FLAGS:= \
	-std=c++11 -Wall -Wextra \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers
EXTRA_FLAGS:= \
	-Os -flto
ifeq ($(TARGET_OS),Windows)
  EXTRA_FLAGS+= -static
endif

NAME:=udp2raw

ifeq ($(TARGET_OS),Windows)
  PCAP:= pcap_wrapper.cpp
else
  PCAP:= -lpcap
endif
MP:=-DUDP2RAW_MP
COMMON:=$(filter-out pcap_wrapper.cpp, $(wildcard *.cpp lib/*.cpp))
LIBS:= -lpthread -isystem libev
ifeq ($(TARGET_OS),Windows)
  LIBS+= -lws2_32
endif
SOURCES:=$(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
SOURCES_AES_ACC=$(COMMON) $(wildcard lib/aes_acc/aes*.c) lib/aes_acc/asm/$@.S
COMPILE_OPT:= -I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS) -o $(NAME)
AES_ACC_TARGETS:=$(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))

.PHONY: linux $(AES_ACC_TARGETS) pcap git_version clean

ifeq ($(TARGET_OS),Linux)
.DEFAULT_GOAL:= linux
else
.DEFAULT_GOAL:= pcap
endif

linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(SOURCES_AES_ACC) $(COMPILE_OPT)

pcap: git_version
	$(CXX) $(SOURCES) $(PCAP) $(MP) $(COMPILE_OPT)

git_version:
ifeq ($(HOST_OS),Windows)
	echo const char *gitversion = "$(UDP2RAW_GIT_VER)"; >git_version.h
else
	echo "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";" >git_version.h
endif

clean:
ifeq ($(HOST_OS),Windows)
	-del /f /q $(NAME).exe git_version.h
else
	-rm -f $(NAME) git_version.h
endif
