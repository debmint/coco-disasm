# Makefile for g09dis

# If g09dis can be compiled in cygwin,
# insert here some conditional whereby
# the CFLAGS is init'ed to "-mms-bitfields"


bin_PROGRAMS = g09dis

g09dis_SOURCES = g09dis.c filestuff.c menu.c odisopts.c dasmedit.c g09dis.h menu.h proto.h

AM_CFLAGS	=$(MINGFLAGS) -Wall @pkgcfg_CFLAGS@
##g09dis_LDFLAGS	= `pkg-config --libs gtk+-2.0`
g09dis_LDADD	=@pkgcfg_LIBS@

##proto.h:
##	cproto `pkg-config --cflags gtk+-2.0` *.c >proto.h

##menu.c: $(HDRS) menu.h
