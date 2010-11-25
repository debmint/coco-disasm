# ############################################################ $
# Makefile for win09dis                                        $
#                                                              $
# $Id$
# ############################################################ $

bin_PROGRAMS = win09dis

GCSRC = win09dis.c filestuff.c menu.c odisopts.c amode.c dasmedit.c search.c
#GHDR = win09dis.h proto.h

win09dis_SOURCES = ${GCSRC} windis.rc

win09dis_LDADD  = -lcomctl32
AM_CFLAGS	=@winflags@ -Wall

.rc.o:
	windres -i windis.rc -o windis.o
##proto.h:
##	cproto *.c >proto.h

##menu.c: $(HDRS) menu.h
