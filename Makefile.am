if WITHG09DIS 
 SUBDIRS = g09dis os9disasm
else
 SUBDIRS = os9disasm
endif

os9disasm:
	cd os9disasm && $(MAKE) all

g09dis:
	cd g09dis && $(MAKE) all

.PHONY: os9disasm g09dis
