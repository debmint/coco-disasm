# Makefile for os9disasm

#include ../Makefile.rules

##SUBDIRS	= doc
EXTRA_DIST = doc/html/*.html doc/README doc/*.xml doc/Makefile

AM_LDFLAGS	=-L../lib

AM_CFLAGS	=-Wall

bin_PROGRAMS = os9disasm
os9disasm_SOURCES = os9disasm.c cmdfile.c dprint.c lbltree.c progdis.c util_dis.c amodes.h odis.h structures.h dtble.h proto.h
