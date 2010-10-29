SUBDIRS = os9disasm

if WITHG09DIS 
 SUBDIRS += g09dis
endif

if WITHWIN09DIS
 SUBDIRS += win09dis
endif

os9disasm:
	cd os9disasm && $(MAKE) all

g09dis:
	cd g09dis && $(MAKE) all

win09dis:
	cd win09dis && $(MAKE) all

.PHONY: os9disasm g09dis win09dis
