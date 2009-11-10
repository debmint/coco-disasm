# ------------------------------------------------------------------ #
# Makefile for the html documentation                                #
# 																     #
# For now, we'll just use a hand-made Makefile, as automake does not #
# 	really support html												 #
# ------------------------------------------------------------------ #

%.html: %.xml
%.xml:

HTMLDIR	=html
VPATH	=$(HTMLDIR)
#STYLSHEET = /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/chunk.xsl
STYLSHEET	= /usr/share/xml/docbook/stylesheet/ldp/html/tldp-sections.xsl
index_SOURCES	= odcmd.xml odquick.xml odisinst.xml odrun.xml \
				  os9disasm.xml g09dis.xml
mkdir_p = mkdir -p --

all: index.html

index.html: $(index_SOURCES) Makefile
	xsltproc -o html/ $(STYLSHEET) os9disasm.xml

clean:
	rm -f $(HTMLDIR)/*.html

distdir:	all
	$(mkdir_p) $(distdir) $(distdir)/html $(distdir)/html/images
	cp README *.xml Makefile $(distdir)
	cp html/*.html html/style.css $(distdir)/html
	cp html/images/*.png $(distdir)/html/images
	cp html/images/*.jpg $(distdir)/html/images

#	doc/html/*.html doc/README doc/*.xml doc/Makefile doc/html/images

distclean:	clean

