##############################################################################
#                                                                            #
# os9disasm - OS9 Cross Disassembler                                         # 
#             following the example of Dynamite+                             #
#                                                                            #
# # ######################################################################## #
#
#  $Id$
#                                                                            #
#  Edition History:                                                          #
#  #  Date       Comments                                              by    #
#  -- ---------- -------------------------------------------------     ---   #
#     2007/10/17 Began Perl version                                    dlb   #
#     2004/04/24 Added 6309 mnemonics                                  dlb   #
#  01 2003/01/31 First began project                                   dlb   #
##############################################################################
# File:  dtble.h                                                             #
# Purpose: lookup tables for disassembly                                     #
##############################################################################

# ********************************************************* *
# Calling procedure:  if need data for tables, don't define *
# GOT_DTBL.  If not needed (already gotten by another       *
# module, define GOT_DTBL and only load definitions and     *
# bypass reloading the data.  This allows loading only the  *
# header data without resorting to an external header file  *
# keeping everything here for in case the sructure changes  *
# ********************************************************* */

# ############################################################ #
# $Id:$
# ############################################################ #

use strict;
use warnings;

# CPU TYPE DEFS
use constant M_09 => 0;
use constant M_03 => 1;

### amode.h Defines for addressing modes 
##
##use constant AMODES => 15; # only 12, but allow for start @1 plus one 
##
### Addressing modes (storage order) 
### defines offset into data default string 
### IMPORTANT!  These first 12 entries _MUST_ remain in the given order! 
##
##use constant AM_INH => 0; # Inherent (no postbytes) 
##use constant AM_DIMM => 2; # D Immediate 
##use constant AM_XIMM => 3; # X Immediate 
##use constant AM_YIMM => 4; # Y Immediate 
##use constant AM_UIMM => 5; # U Immediate 
##use constant AM_SIMM => 6; # S Immediate 
##use constant AM_XIDX => 7; # X-Indexed 
##
##	# The following 3 classes do not apear in dtble.h - and never explicitly
##	# mentioned in the source.  They are here to reserve space in the
##	# Y,U,and S indexed modes, and are given names for clarity
##	 
##use constant AM_YIDX => 8; # Y-Indexed 
##use constant AM_UIDX => 9; # U-Indexed 
##use constant AM_SIDX => 10; # S-Indexed 
##
##	# All the modes above _MUST_ remain in the exact order
##	# for processing in the function Domode (in cmdfile.c
##
##use constant AM_BYTI => 1; # 1-byte immediate 
##use constant AM_DRCT => 11; # Direct 
##use constant AM_EXT => 12; # Extended 
##use constant AM_REL => 13; # Relative 
##
### Classes below don't affect addressing modes 
##
##use constant AM_REG => 14; # Register - no addressing 
##use constant AM_PSH => 15; # Psh/pul instr 
##use constant AM_WIMM => 16;
##use constant AM_TFM => 17;
##use constant AM_BIT => 18;

# End amodes
# ########################################################### #

my %Pre10 = (
	  "21" => {opcode => hex "21", opttype => 'am_rel', mnem => "lbrn ", CPU => M_09, postbytes => 2}, 
	  "22" => {opcode => hex "22", opttype => 'am_rel', mnem => "lbhi ", CPU => M_09, postbytes => 2}, 
	  "23" => {opcode => hex "23", opttype => 'am_rel', mnem => "lbls ", CPU => M_09, postbytes => 2}, 
	  "24" => {opcode => hex "24", opttype => 'am_rel', mnem => "lbhs ", CPU => M_09, postbytes => 2}, 	# also LBCC
	  "25" => {opcode => hex "25", opttype => 'am_rel', mnem => "lblo ", CPU => M_09, postbytes => 2}, 	# also LBCS
	  "26" => {opcode => hex "26", opttype => 'am_rel', mnem => "lbne ", CPU => M_09, postbytes => 2}, 
	  "27" => {opcode => hex "27", opttype => 'am_rel', mnem => "lbeq ", CPU => M_09, postbytes => 2}, 
	  "28" => {opcode => hex "28", opttype => 'am_rel', mnem => "lbvc ", CPU => M_09, postbytes => 2}, 
	  "29" => {opcode => hex "29", opttype => 'am_rel', mnem => "lbvs ", CPU => M_09, postbytes => 2}, 
	  "2a" => {opcode => hex "2a", opttype => 'am_rel', mnem => "lbpl ", CPU => M_09, postbytes => 2}, 
	  "2b" => {opcode => hex "2b", opttype => 'am_rel', mnem => "lbmi ", CPU => M_09, postbytes => 2}, 
	  "2c" => {opcode => hex "2c", opttype => 'am_rel', mnem => "lbge ", CPU => M_09, postbytes => 2}, 
	  "2d" => {opcode => hex "2d", opttype => 'am_rel', mnem => "lblt ", CPU => M_09, postbytes => 2}, 
	  "2e" => {opcode => hex "2e", opttype => 'am_rel', mnem => "lbgt ", CPU => M_09, postbytes => 2}, 
	  "2f" => {opcode => hex "2f", opttype => 'am_rel', mnem => "lble ", CPU => M_09, postbytes => 2}, 

	  "30" => {opcode => hex "30", opttype => 'am_reg', mnem => "addr ", CPU => M_03, postbytes => 1}, 
	  "31" => {opcode => hex "31", opttype => 'am_reg', mnem => "adcr ", CPU => M_03, postbytes => 1}, 
	  "32" => {opcode => hex "32", opttype => 'am_reg', mnem => "subr ", CPU => M_03, postbytes => 1}, 
	  "33" => {opcode => hex "33", opttype => 'am_reg', mnem => "subcr", CPU => M_03, postbytes => 1}, 
	  "34" => {opcode => hex "34", opttype => 'am_reg', mnem => "andr ", CPU => M_03, postbytes => 1}, 
	  "35" => {opcode => hex "35", opttype => 'am_reg', mnem => "orr  ", CPU => M_03, postbytes => 1}, 
	  "36" => {opcode => hex "36", opttype => 'am_reg', mnem => "eorr ", CPU => M_03, postbytes => 1}, 
	  "37" => {opcode => hex "37", opttype => 'am_reg', mnem => "cmpr ", CPU => M_03, postbytes => 1}, 
	  "38" => {opcode => hex "38", opttype => 'am_inh', mnem => "pshsw", CPU => M_03, postbytes => 0}, 
	  "39" => {opcode => hex "39", opttype => 'am_inh', mnem => "pulsw", CPU => M_03, postbytes => 0}, 
	  "3a" => {opcode => hex "3a", opttype => 'am_inh', mnem => "pshuw", CPU => M_03, postbytes => 0}, 
	  "3b" => {opcode => hex "3b", opttype => 'am_inh', mnem => "puluw", CPU => M_03, postbytes => 0}, 

	  "3f" => {opcode => hex "3f", opttype => 'am_bytei', mnem => "swi2 ", CPU => M_09, postbytes => 1}, 	# may have to be changed

      "40" => {opcode => hex "40", opttype => 'am_inh', mnem => "negd ", CPU => M_03, postbytes => 0}, 
	  "43" => {opcode => hex "43", opttype => 'am_inh', mnem => "comd ", CPU => M_03, postbytes => 0}, 
	  "44" => {opcode => hex "44", opttype => 'am_inh', mnem => "lsrd ", CPU => M_03, postbytes => 0}, 
	  "46" => {opcode => hex "46", opttype => 'am_inh', mnem => "rord ", CPU => M_03, postbytes => 0}, 
	  "47" => {opcode => hex "47", opttype => 'am_inh', mnem => "asrd ", CPU => M_03, postbytes => 0}, 
	  "48" => {opcode => hex "48", opttype => 'am_inh', mnem => "asld ", CPU => M_03, postbytes => 0}, 
	  "49" => {opcode => hex "49", opttype => 'am_inh', mnem => "rold ", CPU => M_03, postbytes => 0}, 
	  "4a" => {opcode => hex "4a", opttype => 'am_inh', mnem => "decd ", CPU => M_03, postbytes => 0}, 
	  "4c" => {opcode => hex "4c", opttype => 'am_inh', mnem => "incd ", CPU => M_03, postbytes => 0}, 
	  "4d" => {opcode => hex "4d", opttype => 'am_inh', mnem => "tstd ", CPU => M_03, postbytes => 0}, 
	  "4f" => {opcode => hex "4f", opttype => 'am_inh', mnem => "clrd ", CPU => M_03, postbytes => 0}, 
	  "53" => {opcode => hex "53", opttype => 'am_inh', mnem => "comw ", CPU => M_03, postbytes => 0}, 
	  "54" => {opcode => hex "54", opttype => 'am_inh', mnem => "lsrw ", CPU => M_03, postbytes => 0}, 
	  "56" => {opcode => hex "56", opttype => 'am_inh', mnem => "rorw ", CPU => M_03, postbytes => 0}, 
	  "59" => {opcode => hex "59", opttype => 'am_inh', mnem => "rolw ", CPU => M_03, postbytes => 0}, 
	  "5a" => {opcode => hex "5a", opttype => 'am_inh', mnem => "decw ", CPU => M_03, postbytes => 0}, 
	  "5c" => {opcode => hex "5c", opttype => 'am_inh', mnem => "incw ", CPU => M_03, postbytes => 0}, 
	  "5d" => {opcode => hex "5d", opttype => 'am_inh', mnem => "tstw ", CPU => M_03, postbytes => 0}, 
	  "80" => {opcode => hex "80", opttype => 'am_wimm', mnem => "subw ", CPU => M_03, postbytes => 2}, 
	  "81" => {opcode => hex "81", opttype => 'am_wimm', mnem => "cmpw ", CPU => M_03, postbytes => 2}, 
	  "82" => {opcode => hex "82", opttype => 'am_dimm', mnem => "sbcd ", CPU => M_03, postbytes => 2}, 
	
	  "83" => {opcode => hex "83", opttype => 'am_dimm', mnem => "cmpd ", CPU => M_09, postbytes => 2}, 

	  "84" => {opcode => hex "84", opttype => 'am_dimm', mnem => "andd ", CPU => M_03, postbytes => 2}, 
	  "85" => {opcode => hex "85", opttype => 'am_dimm', mnem => "bitd ", CPU => M_03, postbytes => 2}, 
	  "86" => {opcode => hex "86", opttype => 'am_wimm', mnem => "ldw  ", CPU => M_03, postbytes => 2}, 
	  "88" => {opcode => hex "88", opttype => 'am_dimm', mnem => "eord ", CPU => M_03, postbytes => 2}, 
	  "89" => {opcode => hex "89", opttype => 'am_dimm', mnem => "adcd ", CPU => M_03, postbytes => 2}, 
	  "8a" => {opcode => hex "8a", opttype => 'am_dimm', mnem => "ord  ", CPU => M_03, postbytes => 2}, 
	  "8b" => {opcode => hex "8b", opttype => 'am_wimm', mnem => "addw ", CPU => M_03, postbytes => 2}, 
	
	  "8c" => {opcode => hex "8c", opttype => 'am_yimm', mnem => "cmpy ", CPU => M_09, postbytes => 2}, 
	  "8e" => {opcode => hex "8e", opttype => 'am_yimm', mnem => "ldy  ", CPU => M_09, postbytes => 2}, 

	  "90" => {opcode => hex "90", opttype => 'am_drct', mnem => "subw ", CPU => M_03, postbytes => 1}, 
	  "91" => {opcode => hex "91", opttype => 'am_drct', mnem => "cmpw ", CPU => M_03, postbytes => 1}, 
	  "92" => {opcode => hex "92", opttype => 'am_drct', mnem => "sbcd ", CPU => M_03, postbytes => 1}, 
	
	  "93" => {opcode => hex "93", opttype => 'am_drct', mnem => "cmpd ", CPU => M_09, postbytes => 1}, 

	  "94" => {opcode => hex "94", opttype => 'am_drct', mnem => "andd ", CPU => M_03, postbytes => 1}, 
	  "95" => {opcode => hex "95", opttype => 'am_drct', mnem => "bitd ", CPU => M_03, postbytes => 1}, 
	  "96" => {opcode => hex "96", opttype => 'am_drct', mnem => "ldw  ", CPU => M_03, postbytes => 1}, 
	  "97" => {opcode => hex "97", opttype => 'am_drct', mnem => "stw  ", CPU => M_03, postbytes => 1}, 
	  "98" => {opcode => hex "98", opttype => 'am_drct', mnem => "eord ", CPU => M_03, postbytes => 1}, 
	  "99" => {opcode => hex "99", opttype => 'am_drct', mnem => "adcd ", CPU => M_03, postbytes => 1}, 
	  "9a" => {opcode => hex "9a", opttype => 'am_drct', mnem => "ord  ", CPU => M_03, postbytes => 1}, 
	  "9b" => {opcode => hex "9b", opttype => 'am_drct', mnem => "addw ", CPU => M_03, postbytes => 1}, 

	  "9c" => {opcode => hex "9c", opttype => 'am_drct', mnem => "cmpy ", CPU => M_09, postbytes => 1}, 
	  "9e" => {opcode => hex "9e", opttype => 'am_drct', mnem => "ldy  ", CPU => M_09, postbytes => 1}, 
	  "9f" => {opcode => hex "9f", opttype => 'am_drct', mnem => "sty  ", CPU => M_09, postbytes => 1}, 

	  "a0" => {opcode => hex "a0", opttype => 'am_xidx', mnem => "subw ", CPU => M_03, postbytes => 1}, 
	  "a1" => {opcode => hex "a1", opttype => 'am_xidx', mnem => "cmpw ", CPU => M_03, postbytes => 1}, 
	  "a2" => {opcode => hex "a2", opttype => 'am_xidx', mnem => "sbcd ", CPU => M_03, postbytes => 1}, 
	
	  "a3" => {opcode => hex "a3", opttype => 'am_xidx', mnem => "cmpd ", CPU => M_09, postbytes => 1}, 

	  "a4" => {opcode => hex "a4", opttype => 'am_xidx', mnem => "andd ", CPU => M_03, postbytes => 1}, 
	  "a5" => {opcode => hex "a5", opttype => 'am_xidx', mnem => "bitd ", CPU => M_03, postbytes => 1}, 
	  "a6" => {opcode => hex "a6", opttype => 'am_xidx', mnem => "ldw  ", CPU => M_03, postbytes => 1}, 
	  "a7" => {opcode => hex "a7", opttype => 'am_xidx', mnem => "stw  ", CPU => M_03, postbytes => 1}, 
	  "a8" => {opcode => hex "a8", opttype => 'am_xidx', mnem => "eord ", CPU => M_03, postbytes => 1}, 
	  "a9" => {opcode => hex "a9", opttype => 'am_xidx', mnem => "adcd ", CPU => M_03, postbytes => 1}, 
	  "aa" => {opcode => hex "aa", opttype => 'am_xidx', mnem => "ord  ", CPU => M_03, postbytes => 1}, 
	  "ab" => {opcode => hex "ab", opttype => 'am_xidx', mnem => "addw ", CPU => M_03, postbytes => 1}, 
	
	  "ac" => {opcode => hex "ac", opttype => 'am_xidx', mnem => "cmpy ", CPU => M_09, postbytes => 1}, 
	  "ae" => {opcode => hex "ae", opttype => 'am_xidx', mnem => "ldy  ", CPU => M_09, postbytes => 1}, 
	  "af" => {opcode => hex "af", opttype => 'am_xidx', mnem => "sty  ", CPU => M_09, postbytes => 1}, 
	
	  "b0" => {opcode => hex "b0", opttype => 'am_ext', mnem => "subw ", CPU => M_03, postbytes => 2}, 
	  "b1" => {opcode => hex "b1", opttype => 'am_ext', mnem => "cmpw ", CPU => M_03, postbytes => 2}, 
	  "b2" => {opcode => hex "b2", opttype => 'am_ext', mnem => "sbcd ", CPU => M_03, postbytes => 2}, 
	
	  "b3" => {opcode => hex "b3", opttype => 'am_ext', mnem => "cmpd ", CPU => M_09, postbytes => 2}, 

	  "b4" => {opcode => hex "b4", opttype => 'am_ext', mnem => "andd ", CPU => M_03, postbytes => 2}, 
	  "b5" => {opcode => hex "b5", opttype => 'am_ext', mnem => "bitd ", CPU => M_03, postbytes => 2}, 
	  "b6" => {opcode => hex "b6", opttype => 'am_ext', mnem => "ldw  ", CPU => M_03, postbytes => 2}, 
	  "b7" => {opcode => hex "b7", opttype => 'am_ext', mnem => "stw  ", CPU => M_03, postbytes => 2}, 
	  "b8" => {opcode => hex "b8", opttype => 'am_ext', mnem => "eord ", CPU => M_03, postbytes => 2}, 
	  "b9" => {opcode => hex "b9", opttype => 'am_ext', mnem => "adcd ", CPU => M_03, postbytes => 2}, 
	  "ba" => {opcode => hex "ba", opttype => 'am_ext', mnem => "ord  ", CPU => M_03, postbytes => 2}, 
	  "bb" => {opcode => hex "bb", opttype => 'am_ext', mnem => "addw ", CPU => M_03, postbytes => 2}, 
	
	  "bc" => {opcode => hex "bc", opttype => 'am_ext', mnem => "cmpy ", CPU => M_09, postbytes => 2}, 
	  "be" => {opcode => hex "be", opttype => 'am_ext', mnem => "ldy  ", CPU => M_09, postbytes => 2}, 
	  "bf" => {opcode => hex "bf", opttype => 'am_ext', mnem => "sty  ", CPU => M_09, postbytes => 2}, 
	  "ce" => {opcode => hex "ce", opttype => 'am_simm', mnem => "lds  ", CPU => M_09, postbytes => 2}, 

	  "dc" => {opcode => hex "dc", opttype => 'am_drct', mnem => "ldq  ", CPU => M_03, postbytes => 1}, 
	  "dd" => {opcode => hex "dd", opttype => 'am_drct', mnem => "stq  ", CPU => M_03, postbytes => 1}, 
	
	  "de" => {opcode => hex "de", opttype => 'am_drct', mnem => "lds  ", CPU => M_09, postbytes => 1}, 
	  "df" => {opcode => hex "df", opttype => 'am_drct', mnem => "sts  ", CPU => M_09, postbytes => 1}, 

	  "ec" => {opcode => hex "ec", opttype => 'am_xidx', mnem => "ldq  ", CPU => M_03, postbytes => 1}, 
	  "ed" => {opcode => hex "ed", opttype => 'am_xidx', mnem => "stq  ", CPU => M_03, postbytes => 1}, 
	
	  "ee" => {opcode => hex "ee", opttype => 'am_xidx', mnem => "lds  ", CPU => M_09, postbytes => 1}, 
	  "ef" => {opcode => hex "ef", opttype => 'am_xidx', mnem => "sts  ", CPU => M_09, postbytes => 1}, 

	  "fc" => {opcode => hex "fc", opttype => 'am_ext', mnem => "ldq  ", CPU => M_03, postbytes => 2}, 
	  "fd" => {opcode => hex "fd", opttype => 'am_ext', mnem => "stq  ", CPU => M_03, postbytes => 2}, 
	
	  "fe" => {opcode => hex "fe", opttype => 'am_ext', mnem => "lds  ", CPU => M_09, postbytes => 2}, 
	  "ff" => {opcode => hex "ff", opttype => 'am_ext', mnem => "sts  ", CPU => M_09, postbytes => 2}
  );

my %Pre11 = (

#		* NOTE:  the following (0x30 - 0x3d) may need to have
#		* a different class.  This is a temporary kludge
	  "30" => {opcode => hex "30", opttype => 'am_bit', mnem => "band ", CPU => M_03, postbytes => 2}, 
	  "31" => {opcode => hex "31", opttype => 'am_bit', mnem => "biand", CPU => M_03, postbytes => 2}, 
	  "32" => {opcode => hex "32", opttype => 'am_bit', mnem => "bor  ", CPU => M_03, postbytes => 2}, 
	  "33" => {opcode => hex "33", opttype => 'am_bit', mnem => "bior ", CPU => M_03, postbytes => 2}, 
	  "34" => {opcode => hex "34", opttype => 'am_bit', mnem => "beor ", CPU => M_03, postbytes => 2}, 
	  "35" => {opcode => hex "35", opttype => 'am_bit', mnem => "bieor", CPU => M_03, postbytes => 2}, 
	  "36" => {opcode => hex "36", opttype => 'am_bit', mnem => "ldbt ", CPU => M_03, postbytes => 2}, 
	  "37" => {opcode => hex "37", opttype => 'am_bit', mnem => "stbt ", CPU => M_03, postbytes => 2}, 
	  "38" => {opcode => hex "38", opttype => 'am_tfm', mnem => "tfm  ", CPU => M_03, postbytes => 1}, 
	  "39" => {opcode => hex "39", opttype => 'am_tfm', mnem => "tfm  ", CPU => M_03, postbytes => 1}, 
	  "3a" => {opcode => hex "3a", opttype => 'am_tfm', mnem => "tfm  ", CPU => M_03, postbytes => 1}, 
	  "3b" => {opcode => hex "3b", opttype => 'am_tfm', mnem => "tfm  ", CPU => M_03, postbytes => 1}, 
	  "3c" => {opcode => hex "3c", opttype => 'am_bytei', mnem => "bitmd", CPU => M_03, postbytes => 1}, 
	  "3d" => {opcode => hex "3d", opttype => 'am_bytei', mnem => "ldmd ", CPU => M_03, postbytes => 1}, 

	  "3f" => {opcode => hex "3f", opttype => 'am_bytei', mnem => "swi3 ", CPU => M_09, postbytes => 1}, 	# This may have to be changed

	  "43" => {opcode => hex "43", opttype => 'am_inh', mnem => "come ", CPU => M_03, postbytes => 0}, 
	  "4a" => {opcode => hex "4a", opttype => 'am_inh', mnem => "dece ", CPU => M_03, postbytes => 0}, 
	  "4c" => {opcode => hex "4c", opttype => 'am_inh', mnem => "ince ", CPU => M_03, postbytes => 0}, 
	  "4d" => {opcode => hex "4d", opttype => 'am_inh', mnem => "tste ", CPU => M_03, postbytes => 0}, 
	  "4f" => {opcode => hex "4f", opttype => 'am_inh', mnem => "clre ", CPU => M_03, postbytes => 0}, 
	  "53" => {opcode => hex "53", opttype => 'am_inh', mnem => "comf ", CPU => M_03, postbytes => 0}, 
	  "5a" => {opcode => hex "5a", opttype => 'am_inh', mnem => "decf ", CPU => M_03, postbytes => 0}, 
	  "5c" => {opcode => hex "5c", opttype => 'am_inh', mnem => "incf ", CPU => M_03, postbytes => 0}, 
	  "5d" => {opcode => hex "5d", opttype => 'am_inh', mnem => "tstf ", CPU => M_03, postbytes => 0}, 
	  "5f" => {opcode => hex "5f", opttype => 'am_inh', mnem => "clrf ", CPU => M_03, postbytes => 0}, 
	  "80" => {opcode => hex "80", opttype => 'am_bytei', mnem => "sube ", CPU => M_03, postbytes => 1}, 
	  "81" => {opcode => hex "81", opttype => 'am_bytei', mnem => "cmpe ", CPU => M_03, postbytes => 1}, 
	
	  "83" => {opcode => hex "83", opttype => 'am_uimm', mnem => "cmpu ", CPU => M_09, postbytes => 2}, 

	  "86" => {opcode => hex "86", opttype => 'am_bytei', mnem => "lde  ", CPU => M_03, postbytes => 1}, 
	  "8b" => {opcode => hex "8b", opttype => 'am_bytei', mnem => "adde ", CPU => M_03, postbytes => 1}, 
	
	  "8c" => {opcode => hex "8c", opttype => 'am_simm', mnem => "cmps ", CPU => M_09, postbytes => 2}, 

	  "8d" => {opcode => hex "8d", opttype => 'am_dimm', mnem => "divd ", CPU => M_03, postbytes => 1}, 
	  "8e" => {opcode => hex "8e", opttype => 'am_dimm', mnem => "divq ", CPU => M_03, postbytes => 2}, 
	  "8f" => {opcode => hex "8f", opttype => 'am_dimm', mnem => "muld ", CPU => M_03, postbytes => 2}, 
	  "90" => {opcode => hex "90", opttype => 'am_drct', mnem => "sube ", CPU => M_03, postbytes => 1}, 
	  "91" => {opcode => hex "91", opttype => 'am_drct', mnem => "cmpe ", CPU => M_03, postbytes => 1}, 
	
	  "93" => {opcode => hex "93", opttype => 'am_drct', mnem => "cmpu ", CPU => M_09, postbytes => 1}, 

	  "96" => {opcode => hex "96", opttype => 'am_drct', mnem => "lde  ", CPU => M_03, postbytes => 1}, 
	  "97" => {opcode => hex "97", opttype => 'am_drct', mnem => "ste  ", CPU => M_03, postbytes => 1}, 
	  "9b" => {opcode => hex "9b", opttype => 'am_drct', mnem => "adde ", CPU => M_03, postbytes => 1}, 
	
	  "9c" => {opcode => hex "9c", opttype => 'am_drct', mnem => "cmps ", CPU => M_09, postbytes => 1}, 

	  "9d" => {opcode => hex "9d", opttype => 'am_drct', mnem => "divd ", CPU => M_03, postbytes => 1}, 
	  "9e" => {opcode => hex "9e", opttype => 'am_drct', mnem => "divq ", CPU => M_03, postbytes => 1}, 
	  "9f" => {opcode => hex "9f", opttype => 'am_drct', mnem => "muld ", CPU => M_03, postbytes => 1}, 
	  "a0" => {opcode => hex "a0", opttype => 'am_xidx', mnem => "sube ", CPU => M_03, postbytes => 1}, 
	  "a1" => {opcode => hex "a1", opttype => 'am_xidx', mnem => "cmpe ", CPU => M_03, postbytes => 1}, 
	
	  "a3" => {opcode => hex "a3", opttype => 'am_xidx', mnem => "cmpu ", CPU => M_09, postbytes => 1}, 

	  "a6" => {opcode => hex "a6", opttype => 'am_xidx', mnem => "lde  ", CPU => M_03, postbytes => 1}, 
	  "a7" => {opcode => hex "a7", opttype => 'am_xidx', mnem => "ste  ", CPU => M_03, postbytes => 1}, 
	  "ab" => {opcode => hex "ab", opttype => 'am_xidx', mnem => "adde ", CPU => M_03, postbytes => 1}, 
	
	  "ac" => {opcode => hex "ac", opttype => 'am_xidx', mnem => "cmps ", CPU => M_09, postbytes => 1}, 

	  "ad" => {opcode => hex "ad", opttype => 'am_xidx', mnem => "divd ", CPU => M_03, postbytes => 1}, 
	  "ae" => {opcode => hex "ae", opttype => 'am_xidx', mnem => "divq ", CPU => M_03, postbytes => 1}, 
	  "af" => {opcode => hex "af", opttype => 'am_xidx', mnem => "muld ", CPU => M_03, postbytes => 1}, 
	  "80" => {opcode => hex "80", opttype => 'am_ext', mnem => "sube ", CPU => M_03, postbytes => 2}, 
	  "81" => {opcode => hex "81", opttype => 'am_ext', mnem => "cmpe ", CPU => M_03, postbytes => 2}, 
	
	  "b3" => {opcode => hex "b3", opttype => 'am_ext', mnem => "cmpu ", CPU => M_09, postbytes => 2}, 

	  "b6" => {opcode => hex "b6", opttype => 'am_ext', mnem => "lde  ", CPU => M_03, postbytes => 2}, 
	  "b7" => {opcode => hex "b7", opttype => 'am_ext', mnem => "ste  ", CPU => M_03, postbytes => 2}, 
	  "bb" => {opcode => hex "bb", opttype => 'am_ext', mnem => "adde ", CPU => M_03, postbytes => 2}, 
	
	  "bc" => {opcode => hex "bc", opttype => 'am_ext', mnem => "cmps ", CPU => M_09, postbytes => 2}, 

	  "bd" => {opcode => hex "bd", opttype => 'am_ext', mnem => "divd ", CPU => M_03, postbytes => 2}, 
	  "be" => {opcode => hex "be", opttype => 'am_ext', mnem => "divq ", CPU => M_03, postbytes => 2}, 
	  "bf" => {opcode => hex "bf", opttype => 'am_ext', mnem => "muld ", CPU => M_03, postbytes => 2}, 
	  "c0" => {opcode => hex "c0", opttype => 'am_bytei', mnem => "subf ", CPU => M_03, postbytes => 1}, 
	  "c1" => {opcode => hex "c1", opttype => 'am_bytei', mnem => "cmpf ", CPU => M_03, postbytes => 1}, 
	  "c6" => {opcode => hex "c6", opttype => 'am_bytei', mnem => "ldf  ", CPU => M_03, postbytes => 1}, 
	  "cb" => {opcode => hex "cb", opttype => 'am_bytei', mnem => "addf ", CPU => M_03, postbytes => 1}, 
	  "d0" => {opcode => hex "d0", opttype => 'am_drct', mnem => "subf ", CPU => M_03, postbytes => 1}, 
	  "d1" => {opcode => hex "d1", opttype => 'am_drct', mnem => "cmpf ", CPU => M_03, postbytes => 1}, 
	  "d6" => {opcode => hex "d6", opttype => 'am_drct', mnem => "ldf  ", CPU => M_03, postbytes => 1}, 
	  "d7" => {opcode => hex "d7", opttype => 'am_drct', mnem => "stf  ", CPU => M_03, postbytes => 1}, 
	  "db" => {opcode => hex "db", opttype => 'am_drct', mnem => "addf ", CPU => M_03, postbytes => 1}, 
	  "e0" => {opcode => hex "e0", opttype => 'am_xidx', mnem => "subf ", CPU => M_03, postbytes => 1}, 
	  "e1" => {opcode => hex "e1", opttype => 'am_xidx', mnem => "cmpf ", CPU => M_03, postbytes => 1}, 
	  "e6" => {opcode => hex "e6", opttype => 'am_xidx', mnem => "ldf  ", CPU => M_03, postbytes => 1}, 
	  "e7" => {opcode => hex "e7", opttype => 'am_xidx', mnem => "stf  ", CPU => M_03, postbytes => 1}, 
	  "eb" => {opcode => hex "eb", opttype => 'am_xidx', mnem => "addf ", CPU => M_03, postbytes => 1}, 
	  "f0" => {opcode => hex "f0", opttype => 'am_ext', mnem => "subf ", CPU => M_03, postbytes => 2}, 
	  "f1" => {opcode => hex "f1", opttype => 'am_ext', mnem => "cmpf ", CPU => M_03, postbytes => 2}, 
	  "f6" => {opcode => hex "f6", opttype => 'am_ext', mnem => "ldf  ", CPU => M_03, postbytes => 2}, 
	  "f7" => {opcode => hex "f7", opttype => 'am_ext', mnem => "stf  ", CPU => M_03, postbytes => 2}, 
	  "fb" => {opcode => hex "fb", opttype => 'am_ext', mnem => "addf ", CPU => M_03, postbytes => 2}
);

my %Byte1 = (
	  "00" => {opcode => hex "00", opttype => 'am_drct', mnem => "neg  ", CPU => M_09, postbytes => 1}, 

	  "01" => {opcode => hex "01", opttype => 'am_drct', mnem => "oim  ", CPU => M_03, postbytes => 1}, 
	  "02" => {opcode => hex "02", opttype => 'am_drct', mnem => "aim  ", CPU => M_03, postbytes => 1}, 
	
	  "03" => {opcode => hex "03", opttype => 'am_drct', mnem => "com  ", CPU => M_09, postbytes => 1}, 
	  "04" => {opcode => hex "04", opttype => 'am_drct', mnem => "lsr  ", CPU => M_09, postbytes => 1}, 

	  "05" => {opcode => hex "05", opttype => 'am_drct', mnem => "eim  ", CPU => M_03, postbytes => 1}, 
	
	  "06" => {opcode => hex "06", opttype => 'am_drct', mnem => "ror  ", CPU => M_09, postbytes => 1}, 
	  "07" => {opcode => hex "07", opttype => 'am_drct', mnem => "asr  ", CPU => M_09, postbytes => 1}, 
	  "08" => {opcode => hex "08", opttype => 'am_drct', mnem => "asl  ", CPU => M_09, postbytes => 1}, 	# or LSL
	  "09" => {opcode => hex "09", opttype => 'am_drct', mnem => "rol  ", CPU => M_09, postbytes => 1}, 
	  "0a" => {opcode => hex "0a", opttype => 'am_drct', mnem => "dec  ", CPU => M_09, postbytes => 1}, 

	  "0b" => {opcode => hex "0b", opttype => 'am_drct', mnem => "tim  ", CPU => M_03, postbytes => 1}, 
	
	  "0c" => {opcode => hex "0c", opttype => 'am_drct', mnem => "inc  ", CPU => M_09, postbytes => 1}, 
	  "0d" => {opcode => hex "0d", opttype => 'am_drct', mnem => "tst  ", CPU => M_09, postbytes => 1}, 
	  "0e" => {opcode => hex "0e", opttype => 'am_drct', mnem => "jmp  ", CPU => M_09, postbytes => 1}, 
	  "0f" => {opcode => hex "0f", opttype => 'am_drct', mnem => "clr  ", CPU => M_09, postbytes => 1}, 
	  "12" => {opcode => hex "12", opttype => 'am_inh', mnem => "nop  ", CPU => M_09, postbytes => 0}, 
	  "13" => {opcode => hex "13", opttype => 'am_inh', mnem => "sync ", CPU => M_09, postbytes => 0}, 

	  "14" => {opcode => hex "14", opttype => 'am_inh', mnem => "sexw ", CPU => M_03, postbytes => 0}, 
	
	  "16" => {opcode => hex "16", opttype => 'am_rel', mnem => "lbra ", CPU => M_09, postbytes => 2}, 
	  "17" => {opcode => hex "17", opttype => 'am_rel', mnem => "lbsr ", CPU => M_09, postbytes => 2}, 
	  "19" => {opcode => hex "19", opttype => 'am_inh', mnem => "daa  ", CPU => M_09, postbytes => 0}, 
	  "1a" => {opcode => hex "1a", opttype => 'am_bytei', mnem => "orcc ", CPU => M_09, postbytes => 1}, 
	  "1c" => {opcode => hex "1c", opttype => 'am_bytei', mnem => "andcc", CPU => M_09, postbytes => 1}, 
	  "1d" => {opcode => hex "1d", opttype => 'am_inh', mnem => "sex  ", CPU => M_09, postbytes => 0}, 
	  "1e" => {opcode => hex "1e", opttype => 'am_reg', mnem => "exg  ", CPU => M_09, postbytes => 1}, 
	  "1f" => {opcode => hex "1f", opttype => 'am_reg', mnem => "tfr  ", CPU => M_09, postbytes => 1}, 
	  "20" => {opcode => hex "20", opttype => 'am_rel', mnem => "bra  ", CPU => M_09, postbytes => 1}, 
	 
	  "21" => {opcode => hex "21", opttype => 'am_rel', mnem => "brn  ", CPU => M_09, postbytes => 1}, 
	  "22" => {opcode => hex "22", opttype => 'am_rel', mnem => "bhi  ", CPU => M_09, postbytes => 1}, 
	  "23" => {opcode => hex "23", opttype => 'am_rel', mnem => "bls  ", CPU => M_09, postbytes => 1}, 
	  "24" => {opcode => hex "24", opttype => 'am_rel', mnem => "bcc  ", CPU => M_09, postbytes => 1}, 	# OR BHS
	  "25" => {opcode => hex "25", opttype => 'am_rel', mnem => "bcs  ", CPU => M_09, postbytes => 1}, 	# or BLO
	  "26" => {opcode => hex "26", opttype => 'am_rel', mnem => "bne  ", CPU => M_09, postbytes => 1}, 
	  "27" => {opcode => hex "27", opttype => 'am_rel', mnem => "beq  ", CPU => M_09, postbytes => 1}, 
	  "28" => {opcode => hex "28", opttype => 'am_rel', mnem => "bvc  ", CPU => M_09, postbytes => 1}, 
	  "29" => {opcode => hex "29", opttype => 'am_rel', mnem => "bvs  ", CPU => M_09, postbytes => 1}, 
	  "2a" => {opcode => hex "2a", opttype => 'am_rel', mnem => "bpl  ", CPU => M_09, postbytes => 1}, 
	  "2b" => {opcode => hex "2b", opttype => 'am_rel', mnem => "bmi  ", CPU => M_09, postbytes => 1}, 
	  "2c" => {opcode => hex "2c", opttype => 'am_rel', mnem => "bge  ", CPU => M_09, postbytes => 1}, 
	  "2d" => {opcode => hex "2d", opttype => 'am_rel', mnem => "blt  ", CPU => M_09, postbytes => 1}, 
	  "2e" => {opcode => hex "2e", opttype => 'am_rel', mnem => "bgt  ", CPU => M_09, postbytes => 1}, 
	  "2f" => {opcode => hex "2f", opttype => 'am_rel', mnem => "ble  ", CPU => M_09, postbytes => 1}, 
	
	  "30" => {opcode => hex "30", opttype => 'am_xidx', mnem => "leax ", CPU => M_09, postbytes => 1}, 
	  "31" => {opcode => hex "31", opttype => 'am_xidx', mnem => "leay ", CPU => M_09, postbytes => 1}, 
	  "32" => {opcode => hex "32", opttype => 'am_xidx', mnem => "leas ", CPU => M_09, postbytes => 1}, 
	  "33" => {opcode => hex "33", opttype => 'am_xidx', mnem => "leau ", CPU => M_09, postbytes => 1}, 
	  "34" => {opcode => hex "34", opttype => 'am_psh', mnem => "pshs ", CPU => M_09, postbytes => 1}, 
	  "35" => {opcode => hex "35", opttype => 'am_psh', mnem => "puls ", CPU => M_09, postbytes => 1}, 
	  "36" => {opcode => hex "36", opttype => 'am_psh', mnem => "pshu ", CPU => M_09, postbytes => 1}, 
	  "37" => {opcode => hex "37", opttype => 'am_psh', mnem => "pulu ", CPU => M_09, postbytes => 1}, 
	  "39" => {opcode => hex "39", opttype => 'am_inh', mnem => "rts  ", CPU => M_09, postbytes => 0}, 
	  "3a" => {opcode => hex "3a", opttype => 'am_inh', mnem => "abx  ", CPU => M_09, postbytes => 0}, 
	  "3b" => {opcode => hex "3b", opttype => 'am_inh', mnem => "rti  ", CPU => M_09, postbytes => 0}, 
	  "3c" => {opcode => hex "3c", opttype => 'am_bytei', mnem => "cwai ", CPU => M_09, postbytes => 1}, 
	  "3d" => {opcode => hex "3d", opttype => 'am_inh', mnem => "mul  ", CPU => M_09, postbytes => 0}, 
	  "3f" => {opcode => hex "3f", opttype => 'am_inh', mnem => "swi  ", CPU => M_09, postbytes => 0}, 
	  "40" => {opcode => hex "40", opttype => 'am_inh', mnem => "nega ", CPU => M_09, postbytes => 0}, 
	  "43" => {opcode => hex "43", opttype => 'am_inh', mnem => "coma ", CPU => M_09, postbytes => 0}, 
	  "44" => {opcode => hex "44", opttype => 'am_inh', mnem => "lsra ", CPU => M_09, postbytes => 0}, 
	  "46" => {opcode => hex "46", opttype => 'am_inh', mnem => "rora ", CPU => M_09, postbytes => 0}, 
	  "47" => {opcode => hex "47", opttype => 'am_inh', mnem => "asra ", CPU => M_09, postbytes => 0}, 
	  "48" => {opcode => hex "48", opttype => 'am_inh', mnem => "asla ", CPU => M_09, postbytes => 0}, 	# also LSLA
	  "49" => {opcode => hex "49", opttype => 'am_inh', mnem => "rola ", CPU => M_09, postbytes => 0}, 
	  "4a" => {opcode => hex "4a", opttype => 'am_inh', mnem => "deca ", CPU => M_09, postbytes => 0}, 
	  "4c" => {opcode => hex "4c", opttype => 'am_inh', mnem => "inca ", CPU => M_09, postbytes => 0}, 
	  "4d" => {opcode => hex "4d", opttype => 'am_inh', mnem => "tsta ", CPU => M_09, postbytes => 0}, 
	  "4f" => {opcode => hex "4f", opttype => 'am_inh', mnem => "clra ", CPU => M_09, postbytes => 0}, 
	  "50" => {opcode => hex "50", opttype => 'am_inh', mnem => "negb ", CPU => M_09, postbytes => 0}, 
	  "53" => {opcode => hex "53", opttype => 'am_inh', mnem => "comb ", CPU => M_09, postbytes => 0}, 
	  "54" => {opcode => hex "54", opttype => 'am_inh', mnem => "lsrb ", CPU => M_09, postbytes => 0}, 
	  "56" => {opcode => hex "56", opttype => 'am_inh', mnem => "rorb ", CPU => M_09, postbytes => 0}, 
	  "57" => {opcode => hex "57", opttype => 'am_inh', mnem => "asrb ", CPU => M_09, postbytes => 0}, 
	  "58" => {opcode => hex "58", opttype => 'am_inh', mnem => "lslb ", CPU => M_09, postbytes => 0}, 	# also LSLB
	  "59" => {opcode => hex "59", opttype => 'am_inh', mnem => "rolb ", CPU => M_09, postbytes => 0}, 
	  "5a" => {opcode => hex "5a", opttype => 'am_inh', mnem => "decb ", CPU => M_09, postbytes => 0}, 
	  "5c" => {opcode => hex "5c", opttype => 'am_inh', mnem => "incb ", CPU => M_09, postbytes => 0}, 
	  "5d" => {opcode => hex "5d", opttype => 'am_inh', mnem => "tstb ", CPU => M_09, postbytes => 0}, 
	  "5f" => {opcode => hex "5f", opttype => 'am_inh', mnem => "clrb ", CPU => M_09, postbytes => 0}, 

	  "60" => {opcode => hex "60", opttype => 'am_xidx', mnem => "neg  ", CPU => M_09, postbytes => 1}, 

	  "61" => {opcode => hex "61", opttype => 'am_xidx', mnem => "oim  ", CPU => M_03, postbytes => 1}, 
	  "62" => {opcode => hex "62", opttype => 'am_xidx', mnem => "aim  ", CPU => M_03, postbytes => 1}, 
	
	  "63" => {opcode => hex "63", opttype => 'am_xidx', mnem => "com  ", CPU => M_09, postbytes => 1}, 
	  "64" => {opcode => hex "64", opttype => 'am_xidx', mnem => "lsr  ", CPU => M_09, postbytes => 1}, 

	  "65" => {opcode => hex "65", opttype => 'am_xidx', mnem => "eim  ", CPU => M_03, postbytes => 1}, 
	
	  "66" => {opcode => hex "66", opttype => 'am_xidx', mnem => "ror  ", CPU => M_09, postbytes => 1}, 
	  "67" => {opcode => hex "67", opttype => 'am_xidx', mnem => "asr  ", CPU => M_09, postbytes => 1}, 
	  "68" => {opcode => hex "68", opttype => 'am_xidx', mnem => "asl  ", CPU => M_09, postbytes => 1}, 	# or LSL
	  "69" => {opcode => hex "69", opttype => 'am_xidx', mnem => "rol  ", CPU => M_09, postbytes => 1}, 
	  "6a" => {opcode => hex "6a", opttype => 'am_xidx', mnem => "dec  ", CPU => M_09, postbytes => 1}, 

	  "6b" => {opcode => hex "6b", opttype => 'am_xidx', mnem => "tim  ", CPU => M_03, postbytes => 2}, 
	
	  "6c" => {opcode => hex "6c", opttype => 'am_xidx', mnem => "inc  ", CPU => M_09, postbytes => 1}, 
	  "6d" => {opcode => hex "6d", opttype => 'am_xidx', mnem => "tst  ", CPU => M_09, postbytes => 1}, 
	  "6e" => {opcode => hex "6e", opttype => 'am_xidx', mnem => "jmp  ", CPU => M_09, postbytes => 1}, 
	  "6f" => {opcode => hex "6f", opttype => 'am_xidx', mnem => "clr  ", CPU => M_09, postbytes => 1}, 

	  "70" => {opcode => hex "70", opttype => 'am_ext', mnem => "neg  ", CPU => M_09, postbytes => 2}, 

	  "71" => {opcode => hex "71", opttype => 'am_ext', mnem => "oim  ", CPU => M_03, postbytes => 2}, 
	  "72" => {opcode => hex "72", opttype => 'am_ext', mnem => "aim  ", CPU => M_03, postbytes => 2}, 

	  "73" => {opcode => hex "73", opttype => 'am_ext', mnem => "com  ", CPU => M_09, postbytes => 2}, 
	  "74" => {opcode => hex "74", opttype => 'am_ext', mnem => "lsr  ", CPU => M_09, postbytes => 2}, 

	  "75" => {opcode => hex "75", opttype => 'am_ext', mnem => "eim  ", CPU => M_03, postbytes => 2}, 
	
	  "76" => {opcode => hex "76", opttype => 'am_ext', mnem => "ror  ", CPU => M_09, postbytes => 2}, 
	  "77" => {opcode => hex "77", opttype => 'am_ext', mnem => "asr  ", CPU => M_09, postbytes => 2}, 
	  "78" => {opcode => hex "78", opttype => 'am_ext', mnem => "asl  ", CPU => M_09, postbytes => 2}, 	# or LSL
	  "79" => {opcode => hex "79", opttype => 'am_ext', mnem => "rol  ", CPU => M_09, postbytes => 2}, 
	  "7a" => {opcode => hex "7a", opttype => 'am_ext', mnem => "dec  ", CPU => M_09, postbytes => 2}, 

	  "7b" => {opcode => hex "7b", opttype => 'am_ext', mnem => "tim  ", CPU => M_03, postbytes => 2}, 
	
	  "7c" => {opcode => hex "7c", opttype => 'am_ext', mnem => "inc  ", CPU => M_09, postbytes => 2}, 
	  "7d" => {opcode => hex "7d", opttype => 'am_ext', mnem => "tst  ", CPU => M_09, postbytes => 2}, 
	  "7e" => {opcode => hex "7e", opttype => 'am_ext', mnem => "jmp  ", CPU => M_09, postbytes => 2}, 
	  "7f" => {opcode => hex "7f", opttype => 'am_ext', mnem => "clr  ", CPU => M_09, postbytes => 2}, 
	
	  "80" => {opcode => hex "80", opttype => 'am_bytei', mnem => "suba ", CPU => M_09, postbytes => 1}, 
	  "81" => {opcode => hex "81", opttype => 'am_bytei', mnem => "cmpa ", CPU => M_09, postbytes => 1}, 
	  "82" => {opcode => hex "82", opttype => 'am_bytei', mnem => "sbca ", CPU => M_09, postbytes => 1}, 
	  "83" => {opcode => hex "83", opttype => 'am_dimm', mnem => "subd ", CPU => M_09, postbytes => 2}, 
	  "84" => {opcode => hex "84", opttype => 'am_bytei', mnem => "anda ", CPU => M_09, postbytes => 1}, 
	  "85" => {opcode => hex "85", opttype => 'am_bytei', mnem => "bita ", CPU => M_09, postbytes => 1}, 
	  "86" => {opcode => hex "86", opttype => 'am_bytei', mnem => "lda  ", CPU => M_09, postbytes => 1}, 
	  "88" => {opcode => hex "88", opttype => 'am_bytei', mnem => "eora ", CPU => M_09, postbytes => 1}, 
	  "89" => {opcode => hex "89", opttype => 'am_bytei', mnem => "adca ", CPU => M_09, postbytes => 1}, 
	  "8a" => {opcode => hex "8a", opttype => 'am_bytei', mnem => "ora  ", CPU => M_09, postbytes => 1}, 
	  "8b" => {opcode => hex "8b", opttype => 'am_bytei', mnem => "adda ", CPU => M_09, postbytes => 1}, 
	  "8c" => {opcode => hex "8c", opttype => 'am_ximm', mnem => "cmpx ", CPU => M_09, postbytes => 2}, 
	  "8d" => {opcode => hex "8d", opttype => 'am_rel', mnem => "bsr  ", CPU => M_09, postbytes => 1}, 
	  "8e" => {opcode => hex "8e", opttype => 'am_ximm', mnem => "ldx  ", CPU => M_09, postbytes => 2}, 
	
	  "90" => {opcode => hex "90", opttype => 'am_drct', mnem => "suba ", CPU => M_09, postbytes => 1}, 
	  "91" => {opcode => hex "91", opttype => 'am_drct', mnem => "cmpa ", CPU => M_09, postbytes => 1}, 
	  "92" => {opcode => hex "92", opttype => 'am_drct', mnem => "sbca ", CPU => M_09, postbytes => 1}, 
	  "93" => {opcode => hex "93", opttype => 'am_drct', mnem => "subd ", CPU => M_09, postbytes => 1}, 
	  "94" => {opcode => hex "94", opttype => 'am_drct', mnem => "anda ", CPU => M_09, postbytes => 1}, 
	  "95" => {opcode => hex "95", opttype => 'am_drct', mnem => "bita ", CPU => M_09, postbytes => 1}, 
	  "96" => {opcode => hex "96", opttype => 'am_drct', mnem => "lda  ", CPU => M_09, postbytes => 1}, 
	  "97" => {opcode => hex "97", opttype => 'am_drct', mnem => "sta  ", CPU => M_09, postbytes => 1}, 
	  "98" => {opcode => hex "98", opttype => 'am_drct', mnem => "eora ", CPU => M_09, postbytes => 1}, 
	  "99" => {opcode => hex "99", opttype => 'am_drct', mnem => "adca ", CPU => M_09, postbytes => 1}, 
	  "9a" => {opcode => hex "9a", opttype => 'am_drct', mnem => "ora  ", CPU => M_09, postbytes => 1}, 
	  "9b" => {opcode => hex "9b", opttype => 'am_drct', mnem => "adda ", CPU => M_09, postbytes => 1}, 
	  "9c" => {opcode => hex "9c", opttype => 'am_drct', mnem => "cmpx ", CPU => M_09, postbytes => 1}, 
	  "9d" => {opcode => hex "9d", opttype => 'am_drct', mnem => "jsr  ", CPU => M_09, postbytes => 1}, 
	  "9e" => {opcode => hex "9e", opttype => 'am_drct', mnem => "ldx  ", CPU => M_09, postbytes => 1}, 
	  "9f" => {opcode => hex "9f", opttype => 'am_drct', mnem => "stx  ", CPU => M_09, postbytes => 1}, 
	
	  "a0" => {opcode => hex "a0", opttype => 'am_xidx', mnem => "suba ", CPU => M_09, postbytes => 1}, 
	  "a1" => {opcode => hex "a1", opttype => 'am_xidx', mnem => "cmpa ", CPU => M_09, postbytes => 1}, 
	  "a2" => {opcode => hex "a2", opttype => 'am_xidx', mnem => "sbca ", CPU => M_09, postbytes => 1}, 
	  "a3" => {opcode => hex "a3", opttype => 'am_xidx', mnem => "subd ", CPU => M_09, postbytes => 1}, 
	  "a4" => {opcode => hex "a4", opttype => 'am_xidx', mnem => "anda ", CPU => M_09, postbytes => 1}, 
	  "a5" => {opcode => hex "a5", opttype => 'am_xidx', mnem => "bita ", CPU => M_09, postbytes => 1}, 
	  "a6" => {opcode => hex "a6", opttype => 'am_xidx', mnem => "lda  ", CPU => M_09, postbytes => 1}, 
	  "a7" => {opcode => hex "a7", opttype => 'am_xidx', mnem => "sta  ", CPU => M_09, postbytes => 1}, 
	  "a8" => {opcode => hex "a8", opttype => 'am_xidx', mnem => "eora ", CPU => M_09, postbytes => 1}, 
	  "a9" => {opcode => hex "a9", opttype => 'am_xidx', mnem => "adca ", CPU => M_09, postbytes => 1}, 
	  "aa" => {opcode => hex "aa", opttype => 'am_xidx', mnem => "ora  ", CPU => M_09, postbytes => 1}, 
	  "ab" => {opcode => hex "ab", opttype => 'am_xidx', mnem => "adda ", CPU => M_09, postbytes => 1}, 
	  "ac" => {opcode => hex "ac", opttype => 'am_xidx', mnem => "cmpx ", CPU => M_09, postbytes => 1}, 
	  "ad" => {opcode => hex "ad", opttype => 'am_xidx', mnem => "jsr  ", CPU => M_09, postbytes => 1}, 
	  "ae" => {opcode => hex "ae", opttype => 'am_xidx', mnem => "ldx  ", CPU => M_09, postbytes => 1}, 
	  "af" => {opcode => hex "af", opttype => 'am_xidx', mnem => "stx  ", CPU => M_09, postbytes => 1}, 
	
	  "b0" => {opcode => hex "b0", opttype => 'am_ext', mnem => "suba ", CPU => M_09, postbytes => 2}, 
	  "b1" => {opcode => hex "b1", opttype => 'am_ext', mnem => "cmpa ", CPU => M_09, postbytes => 2}, 
	  "b2" => {opcode => hex "b2", opttype => 'am_ext', mnem => "sbca ", CPU => M_09, postbytes => 2}, 
	  "b3" => {opcode => hex "b3", opttype => 'am_ext', mnem => "subd ", CPU => M_09, postbytes => 2}, 
	  "b4" => {opcode => hex "b4", opttype => 'am_ext', mnem => "anda ", CPU => M_09, postbytes => 2}, 
	  "b5" => {opcode => hex "b5", opttype => 'am_ext', mnem => "bita ", CPU => M_09, postbytes => 2}, 
	  "b6" => {opcode => hex "b6", opttype => 'am_ext', mnem => "lda  ", CPU => M_09, postbytes => 2}, 
	  "b7" => {opcode => hex "b7", opttype => 'am_ext', mnem => "sta  ", CPU => M_09, postbytes => 2}, 
	  "b8" => {opcode => hex "b8", opttype => 'am_ext', mnem => "eora ", CPU => M_09, postbytes => 2}, 
	  "b9" => {opcode => hex "b9", opttype => 'am_ext', mnem => "adca ", CPU => M_09, postbytes => 2}, 
	  "ba" => {opcode => hex "ba", opttype => 'am_ext', mnem => "ora  ", CPU => M_09, postbytes => 2}, 
	  "bb" => {opcode => hex "bb", opttype => 'am_ext', mnem => "adda ", CPU => M_09, postbytes => 2}, 
	  "bc" => {opcode => hex "bc", opttype => 'am_ext', mnem => "cmpx ", CPU => M_09, postbytes => 2}, 
	  "bd" => {opcode => hex "bd", opttype => 'am_ext', mnem => "jsr  ", CPU => M_09, postbytes => 2}, 
	  "be" => {opcode => hex "be", opttype => 'am_ext', mnem => "ldx  ", CPU => M_09, postbytes => 2}, 
	  "bf" => {opcode => hex "bf", opttype => 'am_ext', mnem => "stx  ", CPU => M_09, postbytes => 2}, 
	
	  "c0" => {opcode => hex "c0", opttype => 'am_bytei', mnem => "subb ", CPU => M_09, postbytes => 1}, 
	  "c1" => {opcode => hex "c1", opttype => 'am_bytei', mnem => "cmpb ", CPU => M_09, postbytes => 1}, 
	  "c2" => {opcode => hex "c2", opttype => 'am_bytei', mnem => "sbcb ", CPU => M_09, postbytes => 1}, 
	  "c3" => {opcode => hex "c3", opttype => 'am_dimm', mnem => "addd ", CPU => M_09, postbytes => 2}, 
	  "c4" => {opcode => hex "c4", opttype => 'am_bytei', mnem => "andb ", CPU => M_09, postbytes => 1}, 
	  "c5" => {opcode => hex "c5", opttype => 'am_bytei', mnem => "bitb ", CPU => M_09, postbytes => 1}, 
	  "c6" => {opcode => hex "c6", opttype => 'am_bytei', mnem => "ldb  ", CPU => M_09, postbytes => 1}, 
#	  "c7" => {opcode => hex "c7", opttype => 'am_bytei', mnem => "stb  "}, CPU => , postbytes => 
	  "c8" => {opcode => hex "c8", opttype => 'am_bytei', mnem => "eorb ", CPU => M_09, postbytes => 1}, 
	  "c9" => {opcode => hex "c9", opttype => 'am_bytei', mnem => "adcb ", CPU => M_09, postbytes => 1}, 
	  "ca" => {opcode => hex "ca", opttype => 'am_bytei', mnem => "orb  ", CPU => M_09, postbytes => 1}, 
	  "cb" => {opcode => hex "cb", opttype => 'am_bytei', mnem => "addb ", CPU => M_09, postbytes => 1}, 
	  "cc" => {opcode => hex "cc", opttype => 'am_dimm', mnem => "ldd  ", CPU => M_09, postbytes => 2}, 
	
	  "cd" => {opcode => hex "cd", opttype => 'am_dimm', mnem => "ldq  ", CPU => M_03, postbytes => 4}, 
	
	  "ce" => {opcode => hex "ce", opttype => 'am_uimm', mnem => "ldu  ", CPU => M_09, postbytes => 2}, 
#	  "cf" => {opcode => hex "cf", opttype => 'am_bytei', mnem => "stu  "}, CPU =, postbytes => 
	
	  "d0" => {opcode => hex "d0", opttype => 'am_drct', mnem => "subb ", CPU => M_09, postbytes => 1}, 
	  "d1" => {opcode => hex "d1", opttype => 'am_drct', mnem => "cmpb ", CPU => M_09, postbytes => 1}, 
	  "d2" => {opcode => hex "d2", opttype => 'am_drct', mnem => "sbcb ", CPU => M_09, postbytes => 1}, 
	  "d3" => {opcode => hex "d3", opttype => 'am_drct', mnem => "addd ", CPU => M_09, postbytes => 1}, 
	  "d4" => {opcode => hex "d4", opttype => 'am_drct', mnem => "andb ", CPU => M_09, postbytes => 1}, 
	  "d5" => {opcode => hex "d5", opttype => 'am_drct', mnem => "bitb ", CPU => M_09, postbytes => 1}, 
	  "d6" => {opcode => hex "d6", opttype => 'am_drct', mnem => "ldb  ", CPU => M_09, postbytes => 1}, 
	  "d7" => {opcode => hex "d7", opttype => 'am_drct', mnem => "stb  ", CPU => M_09, postbytes => 1}, 
	  "d8" => {opcode => hex "d8", opttype => 'am_drct', mnem => "eorb ", CPU => M_09, postbytes => 1}, 
	  "d9" => {opcode => hex "d9", opttype => 'am_drct', mnem => "adcb ", CPU => M_09, postbytes => 1}, 
	  "da" => {opcode => hex "da", opttype => 'am_drct', mnem => "orb  ", CPU => M_09, postbytes => 1}, 
	  "db" => {opcode => hex "db", opttype => 'am_drct', mnem => "addb ", CPU => M_09, postbytes => 1}, 
	  "dc" => {opcode => hex "dc", opttype => 'am_drct', mnem => "ldd  ", CPU => M_09, postbytes => 1}, 
	  "dd" => {opcode => hex "dd", opttype => 'am_drct', mnem => "std  ", CPU => M_09, postbytes => 1}, 
	  "de" => {opcode => hex "de", opttype => 'am_drct', mnem => "ldu  ", CPU => M_09, postbytes => 1}, 
	  "df" => {opcode => hex "df", opttype => 'am_drct', mnem => "stu  ", CPU => M_09, postbytes => 1}, 
	
	  "e0" => {opcode => hex "e0", opttype => 'am_xidx', mnem => "subb ", CPU => M_09, postbytes => 1}, 
	  "e1" => {opcode => hex "e1", opttype => 'am_xidx', mnem => "cmpb ", CPU => M_09, postbytes => 1}, 
	  "e2" => {opcode => hex "e2", opttype => 'am_xidx', mnem => "sbcb ", CPU => M_09, postbytes => 1}, 
	  "e3" => {opcode => hex "e3", opttype => 'am_xidx', mnem => "addd ", CPU => M_09, postbytes => 1}, 
	  "e4" => {opcode => hex "e4", opttype => 'am_xidx', mnem => "andb ", CPU => M_09, postbytes => 1}, 
	  "e5" => {opcode => hex "e5", opttype => 'am_xidx', mnem => "bitb ", CPU => M_09, postbytes => 1}, 
	  "e6" => {opcode => hex "e6", opttype => 'am_xidx', mnem => "ldb  ", CPU => M_09, postbytes => 1}, 
	  "e7" => {opcode => hex "e7", opttype => 'am_xidx', mnem => "stb  ", CPU => M_09, postbytes => 1}, 
	  "e8" => {opcode => hex "e8", opttype => 'am_xidx', mnem => "eorb ", CPU => M_09, postbytes => 1}, 
	  "e9" => {opcode => hex "e9", opttype => 'am_xidx', mnem => "adcb ", CPU => M_09, postbytes => 1}, 
	  "ea" => {opcode => hex "ea", opttype => 'am_xidx', mnem => "orb  ", CPU => M_09, postbytes => 1}, 
	  "eb" => {opcode => hex "eb", opttype => 'am_xidx', mnem => "addb ", CPU => M_09, postbytes => 1}, 
	  "ec" => {opcode => hex "ec", opttype => 'am_xidx', mnem => "ldd  ", CPU => M_09, postbytes => 1}, 
	  "ed" => {opcode => hex "ed", opttype => 'am_xidx', mnem => "std  ", CPU => M_09, postbytes => 1}, 
	  "ee" => {opcode => hex "ee", opttype => 'am_xidx', mnem => "ldu  ", CPU => M_09, postbytes => 1}, 
	  "ef" => {opcode => hex "ef", opttype => 'am_xidx', mnem => "stu  ", CPU => M_09, postbytes => 1}, 
	
	  "f0" => {opcode => hex "f0", opttype => 'am_ext', mnem => "subb ", CPU => M_09, postbytes => 2}, 
	  "f1" => {opcode => hex "f1", opttype => 'am_ext', mnem => "cmpb ", CPU => M_09, postbytes => 2}, 
	  "f2" => {opcode => hex "f2", opttype => 'am_ext', mnem => "sbcb ", CPU => M_09, postbytes => 2}, 
	  "f3" => {opcode => hex "f3", opttype => 'am_ext', mnem => "addd ", CPU => M_09, postbytes => 2}, 
	  "f4" => {opcode => hex "f4", opttype => 'am_ext', mnem => "andb ", CPU => M_09, postbytes => 2}, 
	  "f5" => {opcode => hex "f5", opttype => 'am_ext', mnem => "bitb ", CPU => M_09, postbytes => 2}, 
	  "f6" => {opcode => hex "f6", opttype => 'am_ext', mnem => "ldb  ", CPU => M_09, postbytes => 2}, 
	  "f7" => {opcode => hex "f7", opttype => 'am_ext', mnem => "stb  ", CPU => M_09, postbytes => 2}, 
	  "f8" => {opcode => hex "f8", opttype => 'am_ext', mnem => "eorb ", CPU => M_09, postbytes => 2}, 
	  "f9" => {opcode => hex "f9", opttype => 'am_ext', mnem => "adcb ", CPU => M_09, postbytes => 2}, 
	  "fa" => {opcode => hex "fa", opttype => 'am_ext', mnem => "orb  ", CPU => M_09, postbytes => 2}, 
	  "fb" => {opcode => hex "fb", opttype => 'am_ext', mnem => "addb ", CPU => M_09, postbytes => 2}, 
	  "fc" => {opcode => hex "fc", opttype => 'am_ext', mnem => "ldd  ", CPU => M_09, postbytes => 2}, 
	  "fd" => {opcode => hex "fd", opttype => 'am_ext', mnem => "std  ", CPU => M_09, postbytes => 2}, 
	  "fe" => {opcode => hex "fe", opttype => 'am_ext', mnem => "ldu  ", CPU => M_09, postbytes => 2}, 
	  "ff" => {opcode => hex "ff", opttype => 'am_ext', mnem => "stu  ", CPU => M_09, postbytes => 2}
);

# #################################################### #
# iscmd() - Checks to see if code pointed to by p is   #
#           valid code.                                #
# Passed:  $fbyte = ref tofirst byte of code to check  #
#          $csiz = ref to bump size????                #
#          $cputyp                                     #
# Returns: Pointer to valid lkup tbl entry or          #
#           undef on fail                              #
#          $fbyte, $csiz (in calling func are modified #
# #################################################### #

sub iscmd {
    my ($fbyte, $csiz, $cputyp, $fh) = @_;    # BOTH are REFERENCES !!!

    my ($tbl, $c);

    $$fbyte = $c = main::o9_fgetbyte($fh);
    $$csiz = 2;

    my $typ;

    if ($$fbyte == hex '10') {
        $typ = "Pre10";
        $tbl = \%Pre10;
        $c = main::o9_fgetbyte($fh);
        $$fbyte = ($$fbyte << 8) + $c;
    }
    elsif ($$fbyte == hex '11') {
        $typ = "Pre11";
        $tbl = \%Pre11;
        $c = main::o9_fgetbyte($fh);
        $$fbyte = ($$fbyte << 8) + $c;
    }
    else {
        $typ = "Byte1";
        $$csiz = 1;
        $tbl = \%Byte1;
    }

    my $key = sprintf "%02x", $c;

    unless ((exists ($tbl->{$key})) &&
                    ($tbl->{$key}->{CPU} <= $cputyp)) {
        return undef;
    }

    return $tbl->{$key};
}

1;
