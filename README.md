WHAT IS os9disasm?
----------

**os9disasm** is a disassembler for the Tandy Color Computer, primarily targeting
the Microware OS9 operating system, but also capable of disassembling Coco
Basic programs.  This program can be built on other systems (Linux, Windows,
and originally was designed to target OS9-68000 systems, but this has mostly
been abandoned).

The disassembler was inspired by a program called Dynamite+ written for the
OS9-68xx systems.  However, disassembly of a major program requires repeated
disassembly, adding commands, and re-disassembling.  The speed offered by
systems running faster processors makes the process much less tedious.

PACKAGE REQUIREMENTS
-----------

This package comes in two sections.  **Os9disasm** is the disassembler itself,
and is all that is really required.  With this, you have a command-line
disassembler.

For those who desire a GUI interface, the **g09dis**/**win09dis** packages are provided.
These are simply front-ends for **os9disasm**, and are not required, but can
help bypass the learning curve a bit, as there are several command-line
options and such with which one must be familiar to get well-formatted
disassembly output.  The **g09dis** portion is a **gtk** front-end, that can
be run on any system that has **gtk** installed on it.  the **win09dis** part
builds a front-end that runs with the native Windows interface, not requiring
**gtk** to be present.

**The default configuration is to compile WITHOUT G09DIS or WIN09DIS**.  To get
either (or both) to build, see below under "CONFIGURE OPTIONS".

To compile these sources, especially **g09dis**, specific packages must be
present. 
1. The Gtk2 package is a primary requirement.
1. GLib is also needed.

_To build, the dev packages are needed, and to run, only the runtime packages
are required._

To build in Windows, the current preferred method is to build under **mingw**
(**msys2** includes it all).  The needed packages under this system correspond
to those for Linux.

To compile, package,  either **git clone** thie project or untar the package file
and **cd** into the top directory.  Execute the command **./configure** to set
up for building under your particular system.

Both programs can be compiled simultaneously from the top Makefile, or you can
issue the command **make os9disasm** or **make g09dis** (if enabled) from the top
Makefile to make either program separately.  Alternatively, you can **cd** into the
individual directories and compile either program individually.  

CONFIGURE OPTIONS
--------

Issuing the command **./configure --help** shows a brief summary of all options
available to configure.  Most of the options are standard configure options,
however, there are some extra options specific to this package:

  **--with-g09dis[=OPTION]**           where OPTION is either "yes" or "no".
          
	  The default is "no"  **--with-g09dis** by itelf is the same as including
	  the "yes" option.  **--with-g09dis=no** or the equivalent
	  **--without-g09dis** would simply be the default.

        Note that if this option is specified, the package "pkg-config"
        must be installed for configure to succeed.  This package should
        be available with most linux distributions and for win32 apps
        under MinGW/msys, is available in the msys2 package.

                            -------
The other option is:

  **--with-win09dis[=OPTION]**    where OPTION is either "yes" or "no".

        This works the same as with **--with-g09dis**, except that this option
        controls whether to build the native Windows GUI.

        _These options are not mutually exclusive, that is, both can be built
        in the same pass, if both are desired._

                            ---------

BUILDING and INSTALLING DOCUMENTATION
--------

It was decided that documentation building and installing would not be
done automatically, because it might not be desirable for some people.

Therefore, at least for now, the documentation will be controlled by
a manually-created Makefile in the doc directory (os9disasm/doc).
Please read the README in that directory for information on how to
install.
