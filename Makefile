
# FLAGS will be passed to both the C and C++ compiler
FLAGS += \
	-DBUILD_VIRTUAL \
	-I./Via/modules/inc \
	-I./Via/synthesis/inc \
	-I./Via/io/inc \
	-I./Via/ui/inc \

CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

SOURCES += $(wildcard Via/synthesis/oscillators/*.cpp)
SOURCES += $(wildcard Via/synthesis/cmsis_dsp/*.cpp)
SOURCES += $(wildcard Via/synthesis/sequencers/*.cpp)
SOURCES += $(wildcard Via/synthesis/signal_processors/*.cpp)

SOURCES += $(wildcard Via/io/src/*.cpp)

SOURCES += $(wildcard Via/ui/src/*.cpp)

SOURCES += $(wildcard Via/modules/meta/*.cpp)
SOURCES += $(wildcard Via/modules/scanner/*.cpp)
SOURCES += $(wildcard Via/modules/sync/*.cpp)
SOURCES += $(wildcard Via/modules/gateseq/*.cpp)
SOURCES += $(wildcard Via/modules/atsr/*.cpp)
SOURCES += $(wildcard Via/modules/osc3/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..

# Include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk
