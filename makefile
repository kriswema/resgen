# RESGen v2 makefile for Linux

# The compiler
CXX=g++

# Include dirs + own dir
INCLUDEDIRS=-I.

# Define folders
SRCDIR=.
OBJDIR=$(SRCDIR)/obj
BINDIR=$(SRCDIR)/bin
TESTDIR=test

# Define binary filename
EXECNAME=resgen

#base flags that are used in any compilation
BASE_CFLAGS=-O3

CFLAGS=$(BASE_CFLAGS) -std=c++0x -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused

#use these when debugging
#CFLAGS=$(BASE_CFLAGS) -g -D_DEBUG

LDFLAGS=-lstdc++

DO_CXX=$(CXX) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<

OBJ = \
	$(OBJDIR)/enttokenizer.o \
	$(OBJDIR)/listbuilder.o \
	$(OBJDIR)/resgen.o \
	$(OBJDIR)/resgenclass.o \
	$(OBJDIR)/resourcelistbuilder.o \
	$(OBJDIR)/util.o

#############################################################################
# RESGen files
#############################################################################

.PHONY: all debug directories clean get-deps install test

all: directories \
	$(BINDIR)/$(EXECNAME)

debug: CFLAGS += -ggdb -g3
debug: directories
debug: $(BINDIR)/$(EXECNAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(DO_CXX)

$(BINDIR)/$(EXECNAME) : $(OBJ)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

directories:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR)
	$(MAKE) -C $(TESTDIR) clean

get-deps:
	apt-get install libc6-dev-i386 libcppunit-dev

prof: CFLAGS += -pg
prof: LDFLAGS += -pg
prof: all

install:
	cp $(BINDIR)/$(EXECNAME) /usr/bin/$(EXECNAME)

test:
	$(MAKE) -C $(TESTDIR) test
