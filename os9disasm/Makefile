# Makefile for os9disasm

#include ../Makefile.rules

LDFLAGS	+= 	-L../lib
CFLAGS	=	-Wall
INCDIR	= .
HEADERS	= $(INCDIR)/dtble.h $(INCDIR)/odis.h $(INCDIR)/proto.h $(INCDIR)/structures.h
CFLAGS	+= -I$(INCDIR)
PROG	= os9disasm$(EXE)
OBJS	= os9disasm.o cmdfile.o dprint.o lbltree.o progdis.o util_dis.o
INSTALLDIR	= /usr/local/bin

all:	$(PROG) $(HEADERS)

$(PROG):	$(OBJS)

install:	all
	cp -f $(PROG) $(INSTALLDIR)

package:
	cp -f $(PROG) ../package

clean:
	rm -f $(PROG) $(OBJS) core
