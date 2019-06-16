#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = chipdrum

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	ChipDrumPlugin.cpp \
	ChipDrumShared.cpp

FILES_UI  = \
	ChipDrumUI.cpp \
	ChipDrumShared.cpp \
	Artwork.cpp \
	common/components/ColorPalette.cpp \
	common/components/BasicWidget.cpp \
	common/components/TextEdit.cpp \
	common/components/TextLabel.cpp \
	common/components/ToggleButton.cpp \
	common/components/ValueFill.cpp

# --------------------------------------------------------------
# Do some magic

UI_TYPE = cairo
include ../../dpf/Makefile.plugins.mk

BUILD_C_FLAGS += $(shell $(PKG_CONFIG) pangocairo --cflags)
BUILD_CXX_FLAGS += $(shell $(PKG_CONFIG) pangocairo --cflags)
LINK_FLAGS += $(shell $(PKG_CONFIG) pangocairo --libs)

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif

TARGETS += lv2_sep
TARGETS += vst

all: $(TARGETS)
tools: bin/ConvertPresets$(APP_EXT)

clean: clean-tools
.PHONY: clean-tools

Artwork.cpp:
	../../dpf/utils/res2c.py Artwork artwork

bin/ConvertPresets$(APP_EXT): tools/ConvertPresets.cpp ChipDrumShared.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -I../../dpf/distrho -I. $^

clean-tools:
	rm -rf bin

# --------------------------------------------------------------
