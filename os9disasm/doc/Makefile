%.html: %.xml
%.xml:

HTMLDIR	=html
VPATH	=$(HTMLDIR)
STYLSHEET	=/usr/share/sgml/docbook/stylesheet/xsl/ldp/ldp-html-chunk.xsl
index_SOURCES	=odcmd.xml odisinst.xml odrun.xml os9disasm.xml g09dis.xml

all: index.html

index.html: $(index_SOURCES)
	xsltproc -o html/ $(STYLSHEET) os9disasm.xml

distdir:
distclean:
