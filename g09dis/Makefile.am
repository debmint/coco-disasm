# Makefile for g09dis


bin_PROGRAMS = g09dis

g09dis_SOURCES = g09dis.c filestuff.c menu.c odisopts.c dasmedit.c g09dis.h menu.h proto.h

AM_CFLAGS	=@winflags@ -Wall @pkgcfg_CFLAGS@
g09dis_LDADD	=@pkgcfg_LIBS@

##proto.h:
##	cproto `pkg-config --cflags gtk+-2.0` *.c >proto.h

##menu.c: $(HDRS) menu.h
