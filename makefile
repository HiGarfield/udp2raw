# udp2raw Makefile
#
# Optimizations over the previous version:
#   * Incremental, parallel-safe builds (one .o per source, final link).
#   * Per-flavor object directories (.obj/<flavor>/) so switching between
#     linux / pcap never mixes objects compiled with different
#     -DUDP2RAW_MP / PCAP flags (the old single-shot build recompiled
#     everything each time, masking this hazard).
#   * Header dependency tracking via -MMD -MP (editing a .h rebuilds users).
#   * git_version.h is regenerated only when the commit hash actually
#     changes (no more spurious full rebuilds), while staying in sync.
#   * Added a `debug` target (no -Os, no strip, -g) and `clean` now
#     removes the object tree / .d files too.

CXX ?= g++
CC  ?= gcc

# ---- Detect target OS from the compiler triplet ----
CXX_TARGET := $(shell $(CXX) -dumpmachine)
TARGET_OS := $(strip $(if $(findstring -w64,$(CXX_TARGET)),Windows,\
                      $(if $(findstring -w32,$(CXX_TARGET)),Windows,\
                      $(if $(findstring -linux,$(CXX_TARGET)),Linux,\
                      $(if $(findstring -apple,$(CXX_TARGET)),MacOS,\
                      $(error "Target OS not supported: $(CXX_TARGET)"))))))
$(info TARGET_OS = $(TARGET_OS))

# ---- Git version stamp ----
UDP2RAW_GIT_VER ?= $(shell git rev-parse HEAD 2>/dev/null || echo unknown)
UDP2RAW_GIT_VER_CODE := "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";"
$(info UDP2RAW_GIT_VER = $(UDP2RAW_GIT_VER))

# ---- Base compile flags ----
CXXFLAGS := -std=c++11 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter \
            -Wno-missing-field-initializers -I. -isystem libev -MMD -MP
CFLAGS   := -I. -isystem libev -MMD -MP
ASFLAGS  := -I. -MMD -MP

# Optimization / size flags (overridable, e.g. make OPT_FLAGS=-O2)
OPT_FLAGS ?= -Os
OPT_FLAGS += $(if $(filter MacOS,$(TARGET_OS)),,-ffunction-sections)

# Link flags
LDFLAGS_BASE := $(if $(filter MacOS,$(TARGET_OS)),,-Wl,--gc-sections) \
                $(if $(filter Windows,$(TARGET_OS)),-static,)
STRIP_FLAG ?= -s

# Libraries
LIBS := -lpthread
LIBS += $(if $(filter Windows,$(TARGET_OS)),-lws2_32)
LIBS += $(if $(filter Linux,$(TARGET_OS)),-lrt)

# PCAP: Windows uses the npcap SDK, others use libpcap
ifeq ($(TARGET_OS),Windows)
PCAP_INC := -isystem npcap/Include
PCAP_LIB := -lwpcap $(if $(findstring x86_64,$(CXX_TARGET)),-Lnpcap/Lib/x64, \
            $(if $(findstring i386,$(CXX_TARGET)),-Lnpcap/Lib, \
            $(if $(findstring i486,$(CXX_TARGET)),-Lnpcap/Lib, \
            $(if $(findstring i586,$(CXX_TARGET)),-Lnpcap/Lib, \
            $(if $(findstring i686,$(CXX_TARGET)),-Lnpcap/Lib, \
            $(error "Target architecture not supported: $(CXX_TARGET)"))))))
else
PCAP_INC :=
PCAP_LIB := -lpcap
endif

MP := -DUDP2RAW_MP

# ---- Source / object lists ----
COMMON_SRCS := $(wildcard *.cpp) $(wildcard lib/*.cpp)
AES_FASTER_SRCS := $(wildcard lib/aes_faster_c/*.cpp)
AES_ACC_SRCS := $(wildcard lib/aes_acc/aes*.c)
AES_ACC_TARGETS := $(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))

NAME := udp2raw

# ---- git_version.h: regenerate only when the version actually changes ----
# FORCE makes the recipe run every time; the cmp guard only rewrites the
# file when its content changed, so dependents (misc.o) rebuild only then.
git_version.h: FORCE
	@echo $(UDP2RAW_GIT_VER_CODE) > $@.tmp
	@if cmp -s $@.tmp $@ 2>/dev/null; then rm -f $@.tmp; else mv -f $@.tmp $@; fi
FORCE:
.PHONY: FORCE

# ---- Default goal ----
DEFAULT_FLAVOR := $(if $(filter Linux,$(TARGET_OS)),linux,pcap)
.DEFAULT_GOAL := $(DEFAULT_FLAVOR)

# ---- Per-flavor build template (separate object dirs avoid stale-object mix-ups) ----
# $(1)=flavor name, $(2)=extra TARGET_FLAGS (e.g. MP + PCAP)
define FLAVOR_build
$(1)_OBJS := $$(addprefix .obj/$(1)/,$$(COMMON_SRCS:.cpp=.o) $$(AES_FASTER_SRCS:.cpp=.o))
.obj/$(1)/%.o: %.cpp | git_version.h
	@mkdir -p $$(dir $$@)
	$$(CXX) $$(CXXFLAGS) $$(OPT_FLAGS) $(2) -c $$< -o $$@
$(1): $$($(1)_OBJS) git_version.h
	$$(CXX) $$($(1)_OBJS) $$(LDFLAGS_BASE) $$(STRIP_FLAG) $(2) $$(LIBS) -o $$(NAME)
endef

$(eval $(call FLAVOR_build,linux,))
$(eval $(call FLAVOR_build,pcap,$(MP) $(PCAP_INC) $(PCAP_LIB)))

# debug = pcap flags but unoptimized and not stripped
debug: OPT_FLAGS := -O0 -g
debug: STRIP_FLAG :=
$(eval $(call FLAVOR_build,debug,$(MP) $(PCAP_INC) $(PCAP_LIB)))

# ---- AES hardware-accelerated variants ----
# Each builds the binary with its matching asm implementation.
define AES_ACC_build
$(1)_OBJS := $$(addprefix .obj/aes_$(1)/,$$(COMMON_SRCS:.cpp=.o) $$(AES_ACC_SRCS:.c=.o)) .obj/aes_$(1)/lib/aes_acc/asm/$(1).S.o
.obj/aes_$(1)/%.o: %.cpp | git_version.h
	@mkdir -p $$(dir $$@)
	$$(CXX) $$(CXXFLAGS) $$(OPT_FLAGS) -c $$< -o $$@
.obj/aes_$(1)/%.o: %.c | git_version.h
	@mkdir -p $$(dir $$@)
	$$(CXX) $$(CXXFLAGS) $$(OPT_FLAGS) -c $$< -o $$@
# The asm source lives in the source tree (lib/aes_acc/asm/), not under .obj,
# so it needs an explicit rule (a %.o: %.S pattern would mangle the path).
.obj/aes_$(1)/lib/aes_acc/asm/$(1).S.o: lib/aes_acc/asm/$(1).S | git_version.h
	@mkdir -p $$(dir $$@)
	$$(CXX) $$(CXXFLAGS) $$(OPT_FLAGS) -c $$< -o $$@
$(1): $$(addprefix .obj/aes_$(1)/,$$(COMMON_SRCS:.cpp=.o) $$(AES_ACC_SRCS:.c=.o)) .obj/aes_$(1)/lib/aes_acc/asm/$(1).S.o git_version.h
	$$(CXX) $$(addprefix .obj/aes_$(1)/,$$(COMMON_SRCS:.cpp=.o) $$(AES_ACC_SRCS:.c=.o)) .obj/aes_$(1)/lib/aes_acc/asm/$(1).S.o $$(LDFLAGS_BASE) -Wl,-z,noexecstack $$(STRIP_FLAG) $$(LIBS) -o $$(NAME)
endef

$(foreach t,$(AES_ACC_TARGETS),$(eval $(call AES_ACC_build,$(t))))

# ---- Header dependency files (generated by -MMD) ----
DEPS := $(linux_OBJS:.o=.d) $(pcap_OBJS:.o=.d) $(debug_OBJS:.o=.d) \
        $(foreach t,$(AES_ACC_TARGETS),$($(t)_OBJS:.o=.d))
-include $(DEPS)

# ---- Phony targets ----
.PHONY: all linux pcap debug clean $(AES_ACC_TARGETS)
all: $(DEFAULT_FLAVOR)

clean:
	-$(RM) -r .obj $(NAME) $(NAME).exe git_version.h

.DELETE_ON_ERROR:
