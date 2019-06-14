#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = chipwave

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	ChipWavePlugin.cpp \
	ChipWaveShared.cpp \
	DistrhoPluginInfo.cpp

FILES_UI  = \
	ChipWaveUI.cpp \
	ChipWaveShared.cpp \
	Artwork.cpp \
	DistrhoPluginInfo.cpp \
	common/components/ColorPalette.cpp \
	common/components/BasicWidget.cpp \
	common/components/TextEdit.cpp \
	common/components/TextLabel.cpp \
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

Artwork.cpp:
	../../dpf/utils/res2c.py Artwork artwork

# --------------------------------------------------------------
