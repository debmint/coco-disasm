/*############################################################################
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
#     2004/04/24 Added 6309 mnemonics                                  dlb   #
#  01 2003/01/31 First began project                                   dlb   #
##############################################################################
# File:  dtble.h                                                             #
# Purpose: lookup tables for disassembly                                     #
############################################################################*/

/* get Addressing mode definitions */
#include "amodes.h"

/* Lookup table to find mnemonic names	*/

struct lkuptbl {
	int cod;	/* Opcode for command	*/
	char amode;	/* Addressing mode	*/
	char mnem[6];	/* Mnemonic for asm	*/
	int t_cpu;
	int adbyt;	/* number of bytes to add */
};

/* CPU TYPE DEFS */
#ifndef M_09
#   define M_09 0
#   define M_03 1
#endif

/*  opcode      TYPE        mnem   CPU-spec  postbytes
 *  ------      ----        ----   --------  ---------
 */
struct lkuptbl Pre10[] = {
	{0x21,     AM_REL,     "lbrn ",     M_09,    2},
	{0x22,     AM_REL,     "lbhi ",     M_09,    2},
	{0x23,     AM_REL,     "lbls ",     M_09,    2},
	{0x24,     AM_REL,     "lbhs ",     M_09,    2},	/* also LBCC */
	{0x25,     AM_REL,     "lblo ",     M_09,    2},	/* also LBCS */
	{0x26,     AM_REL,     "lbne ",     M_09,    2},
	{0x27,     AM_REL,     "lbeq ",     M_09,    2},
	{0x28,     AM_REL,     "lbvc ",     M_09,    2},
	{0x29,     AM_REL,     "lbvs ",     M_09,    2},
	{0x2a,     AM_REL,     "lbpl ",     M_09,    2},
	{0x2b,     AM_REL,     "lbmi ",     M_09,    2},
	{0x2c,     AM_REL,     "lbge ",     M_09,    2},
	{0x2d,     AM_REL,     "lblt ",     M_09,    2},
	{0x2e,     AM_REL,     "lbgt ",     M_09,    2},
	{0x2f,     AM_REL,     "lble ",     M_09,    2},

	{0x30,     AM_REG,     "addr ",     M_03,    1},
	{0x31,     AM_REG,     "adcr ",     M_03,    1},
	{0x32,     AM_REG,     "subr ",     M_03,    1},
	{0x33,     AM_REG,     "subcr",     M_03,    1},
	{0x34,     AM_REG,     "andr ",     M_03,    1},
	{0x35,     AM_REG,     "orr  ",     M_03,    1},
	{0x36,     AM_REG,     "eorr ",     M_03,    1},
	{0x37,     AM_REG,     "cmpr ",     M_03,    1},
	{0x38,     AM_INH,     "pshsw",     M_03,    0},
	{0x39,     AM_INH,     "pulsw",     M_03,    0},
	{0x3a,     AM_INH,     "pshuw",     M_03,    0},
	{0x3b,     AM_INH,     "puluw",     M_03,    0},

	{0x3f,     AM_BYTI,    "swi2 ",     M_09,    1},	/* may have to be changed */

    {0x40,     AM_INH,     "negd ",     M_03,    0},
	{0x43,     AM_INH,     "comd ",     M_03,    0},
	{0x44,     AM_INH,     "lsrd ",     M_03,    0},
	{0x46,     AM_INH,     "rord ",     M_03,    0},
	{0x47,     AM_INH,     "asrd ",     M_03,    0},
	{0x48,     AM_INH,     "asld ",     M_03,    0},
	{0x49,     AM_INH,     "rold ",     M_03,    0},
	{0x4a,     AM_INH,     "decd ",     M_03,    0},
	{0x4c,     AM_INH,     "incd ",     M_03,    0},
	{0x4d,     AM_INH,     "tstd ",     M_03,    0},
	{0x4f,     AM_INH,     "clrd ",     M_03,    0},
	{0x53,     AM_INH,     "comw ",     M_03,    0},
	{0x54,     AM_INH,     "lsrw ",     M_03,    0},
	{0x56,     AM_INH,     "rorw ",     M_03,    0},
	{0x59,     AM_INH,     "rolw ",     M_03,    0},
	{0x5a,     AM_INH,     "decw ",     M_03,    0},
	{0x5c,     AM_INH,     "incw ",     M_03,    0},
	{0x5d,     AM_INH,     "tstw ",     M_03,    0},
	{0x80,     AM_WIMM,    "subw ",     M_03,    2},
	{0x81,     AM_WIMM,    "cmpw ",     M_03,    2},
	{0x82,     AM_DIMM,    "sbcd ",     M_03,    2},
	
	{0x83,     AM_DIMM,    "cmpd ",     M_09,    2},

	{0x84,     AM_DIMM,    "andd ",     M_03,    2},
	{0x85,     AM_DIMM,    "bitd ",     M_03,    2},
	{0x86,     AM_WIMM,    "ldw  ",     M_03,    2},
	{0x88,     AM_DIMM,    "eord ",     M_03,    2},
	{0x89,     AM_DIMM,    "adcd ",     M_03,    2},
	{0x8a,     AM_DIMM,    "ord  ",     M_03,    2},
	{0x8b,     AM_WIMM,    "addw ",     M_03,    2},
	
	{0x8c,     AM_YIMM,    "cmpy ",     M_09,    2},
	{0x8e,     AM_YIMM,    "ldy  ",     M_09,    2},

	{0x90,     AM_DRCT,    "subw ",     M_03,    1},
	{0x91,     AM_DRCT,    "cmpw ",     M_03,    1},
	{0x92,     AM_DRCT,    "sbcd ",     M_03,    1},
	
	{0x93,     AM_DRCT,    "cmpd ",     M_09,    1},

	{0x94,     AM_DRCT,    "andd ",     M_03,    1},
	{0x95,     AM_DRCT,    "bitd ",     M_03,    1},
	{0x96,     AM_DRCT,    "ldw  ",     M_03,    1},
	{0x97,     AM_DRCT,    "stw  ",     M_03,    1},
	{0x98,     AM_DRCT,    "eord ",     M_03,    1},
	{0x99,     AM_DRCT,    "adcd ",     M_03,    1},
	{0x9a,     AM_DRCT,    "ord  ",     M_03,    1},
	{0x9b,     AM_DRCT,    "addw ",     M_03,    1},

	{0x9c,     AM_DRCT,    "cmpy ",     M_09,    1},
	{0x9e,     AM_DRCT,    "ldy  ",     M_09,    1},
	{0x9f,     AM_DRCT,    "sty  ",     M_09,    1},

	{0xa0,     AM_XIDX,    "subw ",     M_03,    1},
	{0xa1,     AM_XIDX,    "cmpw ",     M_03,    1},
	{0xa2,     AM_XIDX,    "sbcd ",     M_03,    1},
	
	{0xa3,     AM_XIDX,    "cmpd ",     M_09,    1},

	{0xa4,     AM_XIDX,    "andd ",     M_03,    1},
	{0xa5,     AM_XIDX,    "bitd ",     M_03,    1},
	{0xa6,     AM_XIDX,    "ldw  ",     M_03,    1},
	{0xa7,     AM_XIDX,    "stw  ",     M_03,    1},
	{0xa8,     AM_XIDX,    "eord ",     M_03,    1},
	{0xa9,     AM_XIDX,    "adcd ",     M_03,    1},
	{0xaa,     AM_XIDX,    "ord  ",     M_03,    1},
	{0xab,     AM_XIDX,    "addw ",     M_03,    1},
	
	{0xac,     AM_XIDX,    "cmpy ",     M_09,    1},
	{0xae,     AM_XIDX,    "ldy  ",     M_09,    1},
	{0xaf,     AM_XIDX,    "sty  ",     M_09,    1},
	
	{0xb0,     AM_EXT,     "subw ",     M_03,    2},
	{0xb1,     AM_EXT,     "cmpw ",     M_03,    2},
	{0xb2,     AM_EXT,     "sbcd ",     M_03,    2},
	
	{0xb3,     AM_EXT,     "cmpd ",     M_09,    2},

	{0xb4,     AM_EXT,     "andd ",     M_03,    2},
	{0xb5,     AM_EXT,     "bitd ",     M_03,    2},
	{0xb6,     AM_EXT,     "ldw  ",     M_03,    2},
	{0xb7,     AM_EXT,     "stw  ",     M_03,    2},
	{0xb8,     AM_EXT,     "eord ",     M_03,    2},
	{0xb9,     AM_EXT,     "adcd ",     M_03,    2},
	{0xba,     AM_EXT,     "ord  ",     M_03,    2},
	{0xbb,     AM_EXT,     "addw ",     M_03,    2},
	
	{0xbc,     AM_EXT,     "cmpy ",     M_09,    2},
	{0xbe,     AM_EXT,     "ldy  ",     M_09,    2},
	{0xbf,     AM_EXT,     "sty  ",     M_09,    2},
	{0xce,     AM_SIMM,    "lds  ",     M_09,    2},

	{0xdc,     AM_DRCT,    "ldq  ",     M_03,    1},
	{0xdd,     AM_DRCT,    "stq  ",     M_03,    1},
	
	{0xde,     AM_DRCT,    "lds  ",     M_09,    1},
	{0xdf,     AM_DRCT,    "sts  ",     M_09,    1},

	{0xec,     AM_XIDX,    "ldq  ",     M_03,    1},
	{0xed,     AM_XIDX,    "stq  ",     M_03,    1},
	
	{0xee,     AM_XIDX,    "lds  ",     M_09,    1},
	{0xef,     AM_XIDX,    "sts  ",     M_09,    1},

	{0xfc,     AM_EXT,     "ldq  ",     M_03,    2},
	{0xfd,     AM_EXT,     "stq  ",     M_03,    2},
	
	{0xfe,     AM_EXT,     "lds  ",     M_09,    2},
	{0xff,     AM_EXT,     "sts  ",     M_09,    2}
};

struct lkuptbl Pre11[] = {

		/* NOTE:  the following (0x30 - 0x3d) may need to have
		 * a different class.  This is a temporary kludge   */
	{0x30,     AM_BIT,     "band ",     M_03,    2},
	{0x31,     AM_BIT,     "biand",     M_03,    2},
	{0x32,     AM_BIT,     "bor  ",     M_03,    2},
	{0x33,     AM_BIT,     "bior ",     M_03,    2},
	{0x34,     AM_BIT,     "beor ",     M_03,    2},
	{0x35,     AM_BIT,     "bieor",     M_03,    2},
	{0x36,     AM_BIT,     "ldbt ",     M_03,    2},
	{0x37,     AM_BIT,     "stbt ",     M_03,    2},
	{0x38,     AM_TFM,     "tfm  ",     M_03,    1},
	{0x39,     AM_TFM,     "tfm  ",     M_03,    1},
	{0x3a,     AM_TFM,     "tfm  ",     M_03,    1},
	{0x3b,     AM_TFM,     "tfm  ",     M_03,    1},
	{0x3c,     AM_BYTI,    "bitmd",     M_03,    1},
	{0x3d,     AM_BYTI,    "ldmd ",     M_03,    1},

	{0x3f,     AM_BYTI,    "swi3 ",     M_09,    1},	/* This may have to be changed */

	{0x43,     AM_INH,     "come ",     M_03,    0},
	{0x4a,     AM_INH,     "dece ",     M_03,    0},
	{0x4c,     AM_INH,     "ince ",     M_03,    0},
	{0x4d,     AM_INH,     "tste ",     M_03,    0},
	{0x4f,     AM_INH,     "clre ",     M_03,    0},
	{0x53,     AM_INH,     "comf ",     M_03,    0},
	{0x5a,     AM_INH,     "decf ",     M_03,    0},
	{0x5c,     AM_INH,     "incf ",     M_03,    0},
	{0x5d,     AM_INH,     "tstf ",     M_03,    0},
	{0x5f,     AM_INH,     "clrf ",     M_03,    0},
	{0x80,     AM_BYTI,    "sube ",     M_03,    1},
	{0x81,     AM_BYTI,    "cmpe ",     M_03,    1},
	
	{0x83,     AM_UIMM,    "cmpu ",     M_09,    2},

	{0x86,     AM_BYTI,    "lde  ",     M_03,    1},
	{0x8b,     AM_BYTI,    "adde ",     M_03,    1},
	
	{0x8c,     AM_SIMM,    "cmps ",     M_09,    2},

	{0x8d,     AM_DIMM,    "divd ",     M_03,    1},
	{0x8e,     AM_DIMM,    "divq ",     M_03,    2},
	{0x8f,     AM_DIMM,    "muld ",     M_03,    2},
	{0x90,     AM_DRCT,    "sube ",     M_03,    1},
	{0x91,     AM_DRCT,    "cmpe ",     M_03,    1},
	
	{0x93,     AM_DRCT,    "cmpu ",     M_09,    1},

	{0x96,     AM_DRCT,    "lde  ",     M_03,    1},
	{0x97,     AM_DRCT,    "ste  ",     M_03,    1},
	{0x9b,     AM_DRCT,    "adde ",     M_03,    1},
	
	{0x9c,     AM_DRCT,    "cmps ",     M_09,    1},

	{0x9d,     AM_DRCT,    "divd ",     M_03,    1},
	{0x9e,     AM_DRCT,    "divq ",     M_03,    1},
	{0x9f,     AM_DRCT,    "muld ",     M_03,    1},
	{0xa0,     AM_XIDX,    "sube ",     M_03,    1},
	{0xa1,     AM_XIDX,    "cmpe ",     M_03,    1},
	
	{0xa3,     AM_XIDX,    "cmpu ",     M_09,    1},

	{0xa6,     AM_XIDX,    "lde  ",     M_03,    1},
	{0xa7,     AM_XIDX,    "ste  ",     M_03,    1},
	{0xab,     AM_XIDX,    "adde ",     M_03,    1},
	
	{0xac,     AM_XIDX,    "cmps ",     M_09,    1},

	{0xad,     AM_XIDX,    "divd ",     M_03,    1},
	{0xae,     AM_XIDX,    "divq ",     M_03,    1},
	{0xaf,     AM_XIDX,    "muld ",     M_03,    1},
	{0x80,     AM_EXT,     "sube ",     M_03,    2},
	{0x81,     AM_EXT,     "cmpe ",     M_03,    2},
	
	{0xb3,     AM_EXT,     "cmpu ",     M_09,    2},

	{0xb6,     AM_EXT,     "lde  ",     M_03,    2},
	{0xb7,     AM_EXT,     "ste  ",     M_03,    2},
	{0xbb,     AM_EXT,     "adde ",     M_03,    2},
	
	{0xbc,     AM_EXT,     "cmps ",     M_09,    2},

	{0xbd,     AM_EXT,     "divd ",     M_03,    2},
	{0xbe,     AM_EXT,     "divq ",     M_03,    2},
	{0xbf,     AM_EXT,     "muld ",     M_03,    2},
	{0xc0,     AM_BYTI,    "subf ",     M_03,    1},
	{0xc1,     AM_BYTI,    "cmpf ",     M_03,    1},
	{0xc6,     AM_BYTI,    "ldf  ",     M_03,    1},
	{0xcb,     AM_BYTI,    "addf ",     M_03,    1},
	{0xd0,     AM_DRCT,    "subf ",     M_03,    1},
	{0xd1,     AM_DRCT,    "cmpf ",     M_03,    1},
	{0xd6,     AM_DRCT,    "ldf  ",     M_03,    1},
	{0xd7,     AM_DRCT,    "stf  ",     M_03,    1},
	{0xdb,     AM_DRCT,    "addf ",     M_03,    1},
	{0xe0,     AM_XIDX,    "subf ",     M_03,    1},
	{0xe1,     AM_XIDX,    "cmpf ",     M_03,    1},
	{0xe6,     AM_XIDX,    "ldf  ",     M_03,    1},
	{0xe7,     AM_XIDX,    "stf  ",     M_03,    1},
	{0xeb,     AM_XIDX,    "addf ",     M_03,    1},
	{0xf0,     AM_EXT,     "subf ",     M_03,    2},
	{0xf1,     AM_EXT,     "cmpf ",     M_03,    2},
	{0xf6,     AM_EXT,     "ldf  ",     M_03,    2},
	{0xf7,     AM_EXT,     "stf  ",     M_03,    2},
	{0xfb,     AM_EXT,     "addf ",     M_03,    2}
};

struct lkuptbl Byte1[] = {
	{0x00,     AM_DRCT,    "neg  ",     M_09,    1},

	{0x01,     AM_DRCT,    "oim  ",     M_03,    1},
	{0x02,     AM_DRCT,    "aim  ",     M_03,    1},
	
	{0x03,     AM_DRCT,    "com  ",     M_09,    1},
	{0x04,     AM_DRCT,    "lsr  ",     M_09,    1},

	{0x05,     AM_DRCT,    "eim  ",     M_03,    1},
	
	{0x06,     AM_DRCT,    "ror  ",     M_09,    1},
	{0x07,     AM_DRCT,    "asr  ",     M_09,    1},
	{0x08,     AM_DRCT,    "asl  ",     M_09,    1},	/* or LSL */
	{0x09,     AM_DRCT,    "rol  ",     M_09,    1},
	{0x0a,     AM_DRCT,    "dec  ",     M_09,    1},

	{0x0b,     AM_DRCT,    "tim  ",     M_03,    1},
	
	{0x0c,     AM_DRCT,    "inc  ",     M_09,    1},
	{0x0d,     AM_DRCT,    "tst  ",     M_09,    1},
	{0x0e,     AM_DRCT,    "jmp  ",     M_09,    1},
	{0x0f,     AM_DRCT,    "clr  ",     M_09,    1},
	{0x12,     AM_INH,     "nop  ",     M_09,    0},
	{0x13,     AM_INH,     "sync ",     M_09,    0},

	{0x14,     AM_INH,     "sexw ",     M_03,    0},
	
	{0x16,     AM_REL,     "lbra ",     M_09,    2},
	{0x17,     AM_REL,     "lbsr ",     M_09,    2},
	{0x19,     AM_INH,     "daa  ",     M_09,    0},
	{0x1a,     AM_BYTI,    "orcc ",     M_09,    1},
	{0x1c,     AM_BYTI,    "andcc",     M_09,    1},
	{0x1d,     AM_INH,     "sex  ",     M_09,    0},
	{0x1e,     AM_REG,     "exg  ",     M_09,    1},
	{0x1f,     AM_REG,     "tfr  ",     M_09,    1},
	{0x20,     AM_REL,     "bra  ",     M_09,    1},
	 
	{0x21,     AM_REL,     "brn  ",     M_09,    1},
	{0x22,     AM_REL,     "bhi  ",     M_09,    1},
	{0x23,     AM_REL,     "bls  ",     M_09,    1},
	{0x24,     AM_REL,     "bcc  ",     M_09,    1},	/* OR BHS */
	{0x25,     AM_REL,     "bcs  ",     M_09,    1},	/* or BLO */
	{0x26,     AM_REL,     "bne  ",     M_09,    1},
	{0x27,     AM_REL,     "beq  ",     M_09,    1},
	{0x28,     AM_REL,     "bvc  ",     M_09,    1},
	{0x29,     AM_REL,     "bvs  ",     M_09,    1},
	{0x2a,     AM_REL,     "bpl  ",     M_09,    1},
	{0x2b,     AM_REL,     "bmi  ",     M_09,    1},
	{0x2c,     AM_REL,     "bge  ",     M_09,    1},
	{0x2d,     AM_REL,     "blt  ",     M_09,    1},
	{0x2e,     AM_REL,     "bgt  ",     M_09,    1},
	{0x2f,     AM_REL,     "ble  ",     M_09,    1},
	
	{0x30,     AM_XIDX,     "leax ",     M_09,    1},
	{0x31,     AM_XIDX,     "leay ",     M_09,    1},
	{0x32,     AM_XIDX,     "leas ",     M_09,    1},
	{0x33,     AM_XIDX,     "leau ",     M_09,    1},
	{0x34,     AM_PSH,      "pshs ",     M_09,    1},
	{0x35,     AM_PSH,      "puls ",     M_09,    1},
	{0x36,     AM_PSH,      "pshu ",     M_09,    1},
	{0x37,     AM_PSH,      "pulu ",     M_09,    1},
	{0x39,     AM_INH,      "rts  ",     M_09,    0},
	{0x3a,     AM_INH,      "abx  ",     M_09,    0},
	{0x3b,     AM_INH,      "rti  ",     M_09,    0},
	{0x3c,     AM_BYTI,     "cwai ",     M_09,    1},
	{0x3d,     AM_INH,      "mul  ",     M_09,    0},
	{0x3f,     AM_INH,      "swi  ",     M_09,    0},
	{0x40,     AM_INH,      "nega ",     M_09,    0},
	{0x43,     AM_INH,      "coma ",     M_09,    0},
	{0x44,     AM_INH,      "lsra ",     M_09,    0},
	{0x46,     AM_INH,      "rora ",     M_09,    0},
	{0x47,     AM_INH,      "asra ",     M_09,    0},
	{0x48,     AM_INH,      "asla ",     M_09,    0},	/* also LSLA */
	{0x49,     AM_INH,      "rola ",     M_09,    0},
	{0x4a,     AM_INH,      "deca ",     M_09,    0},
	{0x4c,     AM_INH,      "inca ",     M_09,    0},
	{0x4d,     AM_INH,      "tsta ",     M_09,    0},
	{0x4f,     AM_INH,      "clra ",     M_09,    0},
	{0x50,     AM_INH,      "negb ",     M_09,    0},
	{0x53,     AM_INH,      "comb ",     M_09,    0},
	{0x54,     AM_INH,      "lsrb ",     M_09,    0},
	{0x56,     AM_INH,      "rorb ",     M_09,    0},
	{0x57,     AM_INH,      "asrb ",     M_09,    0},
	{0x58,     AM_INH,      "lslb ",     M_09,    0},	/* also LSLB */
	{0x59,     AM_INH,      "rolb ",     M_09,    0},
	{0x5a,     AM_INH,      "decb ",     M_09,    0},
	{0x5c,     AM_INH,      "incb ",     M_09,    0},
	{0x5d,     AM_INH,      "tstb ",     M_09,    0},
	{0x5f,     AM_INH,      "clrb ",     M_09,    0},

	{0x60,     AM_XIDX,     "neg  ",     M_09,    1},

	{0x61,     AM_XIDX,     "oim  ",     M_03,    1},
	{0x62,     AM_XIDX,     "aim  ",     M_03,    1},
	
	{0x63,     AM_XIDX,     "com  ",     M_09,    1},
	{0x64,     AM_XIDX,     "lsr  ",     M_09,    1},

	{0x65,     AM_XIDX,     "eim  ",     M_03,    1},
	
	{0x66,     AM_XIDX,     "ror  ",     M_09,    1},
	{0x67,     AM_XIDX,     "asr  ",     M_09,    1},
	{0x68,     AM_XIDX,     "asl  ",     M_09,    1},	/* or LSL */
	{0x69,     AM_XIDX,     "rol  ",     M_09,    1},
	{0x6a,     AM_XIDX,     "dec  ",     M_09,    1},

	{0x6b,     AM_XIDX,     "tim  ",     M_03,    2},
	
	{0x6c,     AM_XIDX,     "inc  ",     M_09,    1},
	{0x6d,     AM_XIDX,     "tst  ",     M_09,    1},
	{0x6e,     AM_XIDX,     "jmp  ",     M_09,    1},
	{0x6f,     AM_XIDX,     "clr  ",     M_09,    1},

	{0x70,     AM_EXT,      "neg  ",     M_09,    2},

	{0x71,     AM_EXT,      "oim  ",     M_03,    2},
	{0x72,     AM_EXT,      "aim  ",     M_03,    2},

	{0x73,     AM_EXT,      "com  ",     M_09,    2},
	{0x74,     AM_EXT,      "lsr  ",     M_09,    2},

	{0x75,     AM_EXT,      "eim  ",     M_03,    2},
	
	{0x76,     AM_EXT,      "ror  ",     M_09,    2},
	{0x77,     AM_EXT,      "asr  ",     M_09,    2},
	{0x78,     AM_EXT,      "asl  ",     M_09,    2},	/* or LSL */
	{0x79,     AM_EXT,      "rol  ",     M_09,    2},
	{0x7a,     AM_EXT,      "dec  ",     M_09,    2},

	{0x7b,     AM_EXT,      "tim  ",     M_03,    2},
	
	{0x7c,     AM_EXT,      "inc  ",     M_09,    2},
	{0x7d,     AM_EXT,      "tst  ",     M_09,    2},
	{0x7e,     AM_EXT,      "jmp  ",     M_09,    2},
	{0x7f,     AM_EXT,      "clr  ",     M_09,    2},
	
	{0x80,     AM_BYTI,     "suba ",     M_09,    1},
	{0x81,     AM_BYTI,     "cmpa ",     M_09,    1},
	{0x82,     AM_BYTI,     "sbca ",     M_09,    1},
	{0x83,     AM_DIMM,     "subd ",     M_09,    2},
	{0x84,     AM_BYTI,     "anda ",     M_09,    1},
	{0x85,     AM_BYTI,     "bita ",     M_09,    1},
	{0x86,     AM_BYTI,     "lda  ",     M_09,    1},
	{0x88,     AM_BYTI,     "eora ",     M_09,    1},
	{0x89,     AM_BYTI,     "adca ",     M_09,    1},
	{0x8a,     AM_BYTI,     "ora  ",     M_09,    1},
	{0x8b,     AM_BYTI,     "adda ",     M_09,    1},
	{0x8c,     AM_XIMM,     "cmpx ",     M_09,    2},
	{0x8d,     AM_REL,      "bsr  ",     M_09,    1},
	{0x8e,     AM_XIMM,     "ldx  ",     M_09,    2},
	
	{0x90,     AM_DRCT,     "suba ",     M_09,    1},
	{0x91,     AM_DRCT,     "cmpa ",     M_09,    1},
	{0x92,     AM_DRCT,     "sbca ",     M_09,    1},
	{0x93,     AM_DRCT,     "subd ",     M_09,    1},
	{0x94,     AM_DRCT,     "anda ",     M_09,    1},
	{0x95,     AM_DRCT,     "bita ",     M_09,    1},
	{0x96,     AM_DRCT,     "lda  ",     M_09,    1},
	{0x97,     AM_DRCT,     "sta  ",     M_09,    1},
	{0x98,     AM_DRCT,     "eora ",     M_09,    1},
	{0x99,     AM_DRCT,     "adca ",     M_09,    1},
	{0x9a,     AM_DRCT,     "ora  ",     M_09,    1},
	{0x9b,     AM_DRCT,     "adda ",     M_09,    1},
	{0x9c,     AM_DRCT,     "cmpx ",     M_09,    1},
	{0x9d,     AM_DRCT,     "jsr  ",     M_09,    1},
	{0x9e,     AM_DRCT,     "ldx  ",     M_09,    1},
	{0x9f,     AM_DRCT,     "stx  ",     M_09,    1},
	
	{0xa0,     AM_XIDX,     "suba ",     M_09,    1},
	{0xa1,     AM_XIDX,     "cmpa ",     M_09,    1},
	{0xa2,     AM_XIDX,     "sbca ",     M_09,    1},
	{0xa3,     AM_XIDX,     "subd ",     M_09,    1},
	{0xa4,     AM_XIDX,     "anda ",     M_09,    1},
	{0xa5,     AM_XIDX,     "bita ",     M_09,    1},
	{0xa6,     AM_XIDX,     "lda  ",     M_09,    1},
	{0xa7,     AM_XIDX,     "sta  ",     M_09,    1},
	{0xa8,     AM_XIDX,     "eora ",     M_09,    1},
	{0xa9,     AM_XIDX,     "adca ",     M_09,    1},
	{0xaa,     AM_XIDX,     "ora  ",     M_09,    1},
	{0xab,     AM_XIDX,     "adda ",     M_09,    1},
	{0xac,     AM_XIDX,     "cmpx ",     M_09,    1},
	{0xad,     AM_XIDX,     "jsr  ",     M_09,    1},
	{0xae,     AM_XIDX,     "ldx  ",     M_09,    1},
	{0xaf,     AM_XIDX,     "stx  ",     M_09,    1},
	
	{0xb0,     AM_EXT,      "suba ",     M_09,    2},
	{0xb1,     AM_EXT,      "cmpa ",     M_09,    2},
	{0xb2,     AM_EXT,      "sbca ",     M_09,    2},
	{0xb3,     AM_EXT,      "subd ",     M_09,    2},
	{0xb4,     AM_EXT,      "anda ",     M_09,    2},
	{0xb5,     AM_EXT,      "bita ",     M_09,    2},
	{0xb6,     AM_EXT,      "lda  ",     M_09,    2},
	{0xb7,     AM_EXT,      "sta  ",     M_09,    2},
	{0xb8,     AM_EXT,      "eora ",     M_09,    2},
	{0xb9,     AM_EXT,      "adca ",     M_09,    2},
	{0xba,     AM_EXT,      "ora  ",     M_09,    2},
	{0xbb,     AM_EXT,      "adda ",     M_09,    2},
	{0xbc,     AM_EXT,      "cmpx ",     M_09,    2},
	{0xbd,     AM_EXT,      "jsr  ",     M_09,    2},
	{0xbe,     AM_EXT,      "ldx  ",     M_09,    2},
	{0xbf,     AM_EXT,      "stx  ",     M_09,    2},
	
	{0xc0,     AM_BYTI,     "subb ",     M_09,    1},
	{0xc1,     AM_BYTI,     "cmpb ",     M_09,    1},
	{0xc2,     AM_BYTI,     "sbcb ",     M_09,    1},
	{0xc3,     AM_DIMM,     "addd ",     M_09,    2},
	{0xc4,     AM_BYTI,     "andb ",     M_09,    1},
	{0xc5,     AM_BYTI,     "bitb ",     M_09,    1},
	{0xc6,     AM_BYTI,     "ldb  ",     M_09,    1},
/*	{0xc7,     AM_BYTI,     "stb  "},*/
	{0xc8,     AM_BYTI,     "eorb ",     M_09,    1},
	{0xc9,     AM_BYTI,     "adcb ",     M_09,    1},
	{0xca,     AM_BYTI,     "orb  ",     M_09,    1},
	{0xcb,     AM_BYTI,     "addb ",     M_09,    1},
	{0xcc,     AM_DIMM,     "ldd  ",     M_09,    2},
	
	{0xcd,     AM_DIMM,     "ldq  ",     M_03,    4},
	
	{0xce,     AM_UIMM,     "ldu  ",     M_09,    2},
/*	{0xcf,     AM_BYTI,     "stu  "},*/
	
	{0xd0,     AM_DRCT,     "subb ",     M_09,    1},
	{0xd1,     AM_DRCT,     "cmpb ",     M_09,    1},
	{0xd2,     AM_DRCT,     "sbcb ",     M_09,    1},
	{0xd3,     AM_DRCT,     "addd ",     M_09,    1},
	{0xd4,     AM_DRCT,     "andb ",     M_09,    1},
	{0xd5,     AM_DRCT,     "bitb ",     M_09,    1},
	{0xd6,     AM_DRCT,     "ldb  ",     M_09,    1},
	{0xd7,     AM_DRCT,     "stb  ",     M_09,    1},
	{0xd8,     AM_DRCT,     "eorb ",     M_09,    1},
	{0xd9,     AM_DRCT,     "adcb ",     M_09,    1},
	{0xda,     AM_DRCT,     "orb  ",     M_09,    1},
	{0xdb,     AM_DRCT,     "addb ",     M_09,    1},
	{0xdc,     AM_DRCT,     "ldd  ",     M_09,    1},
	{0xdd,     AM_DRCT,     "std  ",     M_09,    1},
	{0xde,     AM_DRCT,     "ldu  ",     M_09,    1},
	{0xdf,     AM_DRCT,     "stu  ",     M_09,    1},
	
	{0xe0,     AM_XIDX,     "subb ",     M_09,    1},
	{0xe1,     AM_XIDX,     "cmpb ",     M_09,    1},
	{0xe2,     AM_XIDX,     "sbcb ",     M_09,    1},
	{0xe3,     AM_XIDX,     "addd ",     M_09,    1},
	{0xe4,     AM_XIDX,     "andb ",     M_09,    1},
	{0xe5,     AM_XIDX,     "bitb ",     M_09,    1},
	{0xe6,     AM_XIDX,     "ldb  ",     M_09,    1},
	{0xe7,     AM_XIDX,     "stb  ",     M_09,    1},
	{0xe8,     AM_XIDX,     "eorb ",     M_09,    1},
	{0xe9,     AM_XIDX,     "adcb ",     M_09,    1},
	{0xea,     AM_XIDX,     "orb  ",     M_09,    1},
	{0xeb,     AM_XIDX,     "addb ",     M_09,    1},
	{0xec,     AM_XIDX,     "ldd  ",     M_09,    1},
	{0xed,     AM_XIDX,     "std  ",     M_09,    1},
	{0xee,     AM_XIDX,     "ldu  ",     M_09,    1},
	{0xef,     AM_XIDX,     "stu  ",     M_09,    1},
	
	{0xf0,     AM_EXT,      "subb ",     M_09,    2},
	{0xf1,     AM_EXT,      "cmpb ",     M_09,    2},
	{0xf2,     AM_EXT,      "sbcb ",     M_09,    2},
	{0xf3,     AM_EXT,      "addd ",     M_09,    2},
	{0xf4,     AM_EXT,      "andb ",     M_09,    2},
	{0xf5,     AM_EXT,      "bitb ",     M_09,    2},
	{0xf6,     AM_EXT,      "ldb  ",     M_09,    2},
	{0xf7,     AM_EXT,      "stb  ",     M_09,    2},
	{0xf8,     AM_EXT,      "eorb ",     M_09,    2},
	{0xf9,     AM_EXT,      "adcb ",     M_09,    2},
	{0xfa,     AM_EXT,      "orb  ",     M_09,    2},
	{0xfb,     AM_EXT,      "addb ",     M_09,    2},
	{0xfc,     AM_EXT,      "ldd  ",     M_09,    2},
	{0xfd,     AM_EXT,      "std  ",     M_09,    2},
	{0xfe,     AM_EXT,      "ldu  ",     M_09,    2},
	{0xff,     AM_EXT,      "stu  ",     M_09,    2}
};

