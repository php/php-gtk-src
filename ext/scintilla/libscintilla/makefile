# Make file for Scintilla bundled with PHP-GTK.
# Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>

.SUFFIXES: .cxx .c .o .h .a
CC = g++
CCOMP = gcc
AR = ar
RANLIB = touch

ifeq ($(shell uname),Darwin)
RANLIB = ranlib
endif

COMPLIB=./scintilla.a

vpath %.h ./src ./include
vpath %.cxx ./src

INCLUDEDIRS=-I ./include -I ./src
CXXBASEFLAGS=-Wall -Wno-missing-braces -Wno-char-subscripts -pedantic -Os -DGTK -DSCI_LEXER $(INCLUDEDIRS)

ifdef NOTHREADS
THREADFLAGS=-DG_THREADS_IMPL_NONE
else
THREADFLAGS=
endif

ifdef DEBUG
CXXFLAGS=-DDEBUG -g $(CXXBASEFLAGS) $(THREADFLAGS)
else
CXXFLAGS=-DNDEBUG -Os $(CXXBASEFLAGS) $(THREADFLAGS)
endif

CONFIGFLAGS=pkg-config --cflags gtk+-2.0
MARSHALLER=scintilla-marshal.o

.cxx.o:
	$(CC) `$(CONFIGFLAGS)` $(CXXFLAGS) -c $<
.c.o:
	$(CCOMP) `$(CONFIGFLAGS)` $(CXXFLAGS) -w -c $<

LEXOBJS=\
LexAda.o LexAPDL.o LexAsm.o LexAsn1.o LexAU3.o LexAVE.o LexBaan.o LexBash.o \
LexBasic.o LexBullant.o LexCaml.o LexCLW.o LexCmake.o LexConf.o LexCPP.o \
LexCrontab.o LexCsound.o LexCSS.o LexD.o LexEiffel.o LexErlang.o LexEScript.o \
LexFlagship.o LexForth.o LexFortran.o LexGui4Cli.o LexHaskell.o LexHTML.o \
LexInno.o LexKix.o LexLisp.o LexLout.o LexLua.o LexMatlab.o LexMetapost.o \
LexMMIXAL.o LexMPT.o LexMSSQL.o LexNsis.o LexOpal.o LexOthers.o LexPascal.o \
LexPB.o LexPerl.o LexPOV.o LexPS.o LexPython.o LexRebol.o LexRuby.o \
LexScriptol.o LexSmalltalk.o LexSpecman.o LexSpice.o LexSQL.o LexTADS3.o \
LexTCL.o LexTeX.o LexVB.o LexVerilog.o LexVHDL.o LexYAML.o

all: $(COMPLIB)

clean:
	rm -f *.o $(COMPLIB)

deps:
	$(CC) -MM `$(CONFIGFLAGS)` $(CXXFLAGS) *.cxx ./src/*.cxx >deps.mak

$(COMPLIB): DocumentAccessor.o WindowAccessor.o KeyWords.o StyleContext.o \
	CharClassify.o Document.o CallTip.o \
	ScintillaBase.o ContractionState.o Editor.o ExternalLexer.o PropSet.o PlatGTK.o \
	KeyMap.o LineMarker.o ScintillaGTK.o CellBuffer.o ViewStyle.o \
	RESearch.o Style.o Indicator.o AutoComplete.o UniConversion.o XPM.o \
	$(MARSHALLER) $(LEXOBJS)
	$(AR) rc $@ $^
	$(RANLIB) $@

# Automatically generate header dependencies with "make deps"
include deps.mak
