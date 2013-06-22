# RESGen v2 makefile for Linux
#

# Result filename
EXECNAME=resgen

# The compiler
CC=g++

# Include dirs + own dir
INCLUDEDIRS=-I.

# source dir (own dir) plus temp object dir
SRCDIR=.
OBJDIR=$(SRCDIR)/obj

#base flags that are used in any compilation
BASE_CFLAGS=

#safe optimization
#CFLAGS=$(BASE_CFLAGS) -w -s -march=i586 -O1

#full optimization
CFLAGS=$(BASE_CFLAGS) -w -s -march=native -O2 \
	-ffast-math -funroll-loops \
	-fexpensive-optimizations -malign-loops=2 \
	-malign-jumps=2 -malign-functions=2

#use these when debugging
#CFLAGS=$(BASE_CFLAGS) -g -D_DEBUG

LDFLAGS=-lstdc++

DO_CC=$(CC) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<

#############################################################################
# RESGen files
#############################################################################

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(DO_CC)

OBJ = \
	$(OBJDIR)/LinkedList.o \
	$(OBJDIR)/leakcheck.o \
	$(OBJDIR)/listbuilder.o \
	$(OBJDIR)/resgen.o \
	$(OBJDIR)/resgenclass.o \
	$(OBJDIR)/vstring.o

$(EXECNAME) : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

neat:
	-mkdir $(OBJDIR)
clean:
	-rm -f $(OBJ)
	-rm -f $(EXECNAME)
spotless: clean
	-rm -r $(OBJDIR)
get-deps:
	-apt-get install libc6-dev-i386
