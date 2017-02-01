# ------------------------------------------------------------------ #
# Makefile for the html documentation                                #
# 																     #
# For now, we'll just use a hand-made Makefile, as automake does not #
# 	really support html												 #
# ------------------------------------------------------------------ #

%.html: %.xml
%.xml:

HTMLDIR	= html
VPATH	= $(HTMLDIR)

# NOTE: The STYLSHEET parameter will probably need to be manually edited
# for it to work, there are so many variables in the location of stylesheets.
#STYLSHEET = /usr/share/xml/docbook/stylesheet/nwalsh/xhtml/chunk.xsl
#STYLSHEET	= /usr/share/xml/docbook/stylesheet/ldp/html/tldp-sections.xsl
STYLSHEET	= /usr/share/xml/docbook/xsl-stylesheets-1.79.1/html/chunk.xsl

index_SOURCES	= odcmd.xml odquick.xml odisinst.xml odrun.xml \
				  os9disasm.xml g09dis.xml
mkdir_p = mkdir -p

all: index.html

tell:
	@echo Home is $(HAVE_XSLTPROC)

index.html: $(index_SOURCES) Makefile
	xsltproc -o html/ $(STYLSHEET) os9disasm.xml

clean:
	rm -f $(HTMLDIR)/*.html

distdir:	all
	$(mkdir_p) $(distdir)/html/images
	cp README *.xml Makefile $(distdir)
	cp html/*.html html/style.css $(distdir)/html
	cp html/images/*.png $(distdir)/html/images
	cp html/images/*.jpg $(distdir)/html/images

distclean:	clean

