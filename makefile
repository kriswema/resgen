# RESGen v2 makefile for Linux

# The compiler
CC=g++

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
BASE_CFLAGS=

#safe optimization
#CFLAGS=$(BASE_CFLAGS) -w -s -march=i586 -O1

#full optimization
#CFLAGS=$(BASE_CFLAGS) -Wall -Wextra -pedantic
CFLAGS=$(BASE_CFLAGS) -std=c++11 -Wall -Wextra -pedantic

#use these when debugging
#CFLAGS=$(BASE_CFLAGS) -g -D_DEBUG

LDFLAGS=-lstdc++

DO_CC=$(CC) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<

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
	$(DO_CC)

OBJ = \
	$(OBJDIR)/listbuilder.o \
	$(OBJDIR)/resgen.o \
	$(OBJDIR)/resgenclass.o \
	$(OBJDIR)/util.o \
	$(OBJDIR)/vstring.o

$(BINDIR)/$(EXECNAME) : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

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
