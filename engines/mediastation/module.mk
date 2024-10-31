MODULE := engines/mediastation

MODULE_OBJS = \
	mediastation.o \
	chunk.o \
	context.o \
	contextparameters.o \
	mediascript/codechunk.o \
	mediascript/function.o \
	mediascript/variabledeclaration.o \
	subfile.o \
	boot.o \
	datum.o \
	datafile.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_MEDIASTATION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
