# Makefile for g09dis

ifeq ($(MSYSTEM), MINGW32)
CFLAGS	+=	-mno-cygwin -mms-bitfields
endif

# If g09dis can be compiled in cygwin,
# insert here some conditional whereby
# the CFLAGS is init'ed to "-mms-bitfields"

CFLAGS	+=	-g -Wall `pkg-config --cflags gtk+-2.0`
OBJS	=	g09dis.o filestuff.o menu.o odisopts.o dasmedit.o

LFLAGS	=	-g `pkg-config --libs gtk+-2.0`
HDRS	=	g09dis.h proto.h

g09dis: $(OBJS) $(HDRS)
	$(CC) $(OBJS) $(LFLAGS) -o g09dis

clean:
	rm -f g09dis $(OBJS) proto.h

$(OBJS): $(HDRS)

proto.h:
	cproto `pkg-config --cflags gtk+-2.0` *.c >proto.h

menu.c: $(HDRS) menu.h
