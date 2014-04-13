# RESGen v2 makefile for Linux

# The compiler
CC=g++

# Include dirs + own dir
INCLUDEDIRS=-I. -I..

# Define folders
SRCDIR=.
MAIN_SRCDIR=..
OBJDIR=$(SRCDIR)/obj
MAIN_OBJDIR=../obj
BINDIR=$(SRCDIR)

# Define binary filename
EXECNAME=testrunner

#base flags that are used in any compilation
BASE_CFLAGS=

CFLAGS=$(BASE_CFLAGS) -Wall -Wextra -pedantic

LDFLAGS=
LDLIBS=-lstdc++ -lcppunit

DO_CC=$(CC) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<

#############################################################################
# RESGen files
#############################################################################

all: directories $(EXECNAME)

debug: CFLAGS += -ggdb -g3
debug: directories
debug: $(EXECNAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(DO_CC)

OBJ = \
	$(OBJDIR)/test.o \
	$(MAIN_OBJDIR)/LinkedList.o \
	$(MAIN_OBJDIR)/listbuilder.o \
	$(MAIN_OBJDIR)/resgenclass.o \
	$(MAIN_OBJDIR)/vstring.o


$(EXECNAME) : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

directories:
	mkdir -p $(OBJDIR)

test: all
	./$(EXECNAME)

clean:
	rm $(BINDIR)/$(EXECNAME)
	rm -r $(OBJDIR)

get-deps:
	apt-get install libc6-dev-i386 libcppunit-dev
