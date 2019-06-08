#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = evalua

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	EvaluaPlugin.cpp \
	ev.cpp

FILES_UI  = \
	EvaluaUI.cpp \
	ColorPalette.cpp \
	components/TextEdit.cpp \
	components/TextLabel.cpp \
	components/ValueFill.cpp \
	ev.cpp

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

all: $(TARGETS)

# --------------------------------------------------------------
