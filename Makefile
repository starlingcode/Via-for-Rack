SLUG = Starling_Via_Meta
VERSION = 0.6.0

# FLAGS will be passed to both the C and C++ compiler
FLAGS += \
	-DBUILD_VIRTUAL \
	-I./Via/via_cplusplus/modules/inc \
	-I./Via/via_cplusplus/synthesis/inc \
	-I./Via/via_cplusplus/io/inc \
	-I./Via/via_cplusplus/ui/inc 
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

SOURCES += $(wildcard Via/via_cplusplus/synthesis/oscillators/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/synthesis/cmsis_dsp/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/synthesis/sequencers/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/synthesis/signal_processors/*.cpp)

SOURCES += $(wildcard Via/via_cplusplus/io/src/*.cpp)

SOURCES += $(wildcard Via/via_cplusplus/ui/src/*.cpp)

SOURCES += $(wildcard Via/via_cplusplus/modules/meta/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/modules/scanner/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/modules/sync/*.cpp)
SOURCES += $(wildcard Via/via_cplusplus/modules/trigseq/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..

# Include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk
