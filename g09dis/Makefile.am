# Makefile for g09dis

# $Id$

bin_PROGRAMS = g09dis

GCSRC = g09dis.c filestuff.c menu.c odisopts.c dasmedit.c amode.c search.c
GHDR = g09dis.h menu.h proto.h

g09dis_SOURCES = ${GCSRC} ${GHDR} 

AM_CFLAGS	= @gwinflags@ -Wall @pkgcfg_CFLAGS@
g09dis_LDADD	= @pkgcfg_LIBS@

##proto.h:
##	cproto `pkg-config --cflags gtk+-2.0` *.c >proto.h

##menu.c: $(HDRS) menu.h
