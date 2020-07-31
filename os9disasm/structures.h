/*
 * This file is part of the coco-disasm project.
 *
 * Copyright (c) 2017 by David Breeding
 *
 * This project is licensed under the Gnu GPL v3 license. 
 * You should have received a copy of the licence for this program
 * in the file "COPYING".  If not, the license is available at
 *       https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 */

/* **************************************************************** *
 * structures.h - definitions for structures used in os9disasm      $
 * Note:  Coco Integers in most cases are represented as an array   $
 * of two chars due to big-endian  translations                     $
 *                                                                  $
 * $Id$
 * **************************************************************** */

/* *************************************************************** *
 * OS9 Module header defs                                          *
 * *************************************************************** */

//struct modhead {
//	unsigned char M_ID[2],	    /* Sync bytes ($87CD)	*/
//	     M_Size[2],	            /* Module Size		*/
//	     M_Name[2];	            /* Ptr to Module Name	*/
//	unsigned char M_Type,	    /* Type / Language	*/
//	     M_Revs,	            /* Attributes/Rev Level	*/
//	     M_Parity,	            /* Header Parity	*/
//	     M_Exec[2],	            /*Execution Entry Offset*/
//	     M_Mem[2];	            /*Stack Requirement	*/
//};

/* This structure dflts contains the list of defaults/options
 * under which the program is running	*/

struct dflts {
	char ShLbl;	        /* 0 = 8-char labels; 1 = 6-char labels */
	char OSType;	    /* See OS_* defines */
};

/* Symbol Table tree structure */

#define NLMAX 12

struct nlist {
	char sname[NLMAX+1];	    /* symbol name	*/
	int myaddr;		            /* address of symbol */
	int stdnam;		            /* flag that it's a std named label */
    int global;                 /* For ROF use... flags it's global */
	struct nlist *LNext;	    /* left (smaller) next	*/
	struct nlist *RNext;	    /* right (larger) next	*/
	struct nlist *parent;	    /* parent	*/
};

/* Offset [ (-L0053) - types stuff ] */

struct ofsetree {
	char oclas_maj;	    /* Class to use in offset addressing             */
	int  of_maj;	    /* The main offset value		                 */
	char incl_pc;	    /* Flag to include PC offset mode	             */
	int add_to;	        /* Flag: if set, means add offset, else subtract */
	/*int  of_sec;	 Secondary offset (0 if none)                        */
	/*char oclas_sec;	 Class of secondary offset                       */
};

/* Data areas/Label Addressing Modes tree structure */

struct databndaries {
	int b_lo;               /* lower (beginning) boundary address            */
	int b_hi;               /* high (ending) boundary address                */
	char b_typ;	            /* Boundary type for DA's, Lbl Class for A-Modes */
	char b_class;	        /* class def for type L and S Data boundaries    */
	struct ofsetree *dofst;
	struct databndaries *DLeft,
	                    *DRight,
			            *dabove;
};

/* Addressing mode default */
/* we may not need it, but just define a string */

struct amodedef {
	char byte_imm,
	     wrd_imm,
	     d_imm,
	     x_imm,
	     y_imm,
	     u_imm,
	     s_imm,
	     x_idx,
	     y_idx,
	     u_idx,
	     s_idx,
	     m_dp,
	     ext_adr,
	     relatv;
};

/* buffer to hold print strings	*/

struct printbuf {
	char instr[16],
	     opcod[20],
	     lbnm[NLMAX+1],
	     mnem[6],
	     operand[100],
         *comment;
};

/* ******************************************** *
 * xtndcmnt structures                          *
 * These are comments that are appended to the  *
 * end of the assembly line                     *
 * ******************************************** */

struct apndcmnt {
    int adrs;
    struct apndcmnt *apLeft;
    struct apndcmnt *apRight;
    char *CmPtr;
};

/* ************************************ *
 * Stand-alone Comment structures       *
 * ************************************ */

/* Single line of a comment */
struct cmntline {
    struct cmntline *nextline;
    char *ctxt;
};

/* Main tree */

struct commenttree {
    int adrs;
    struct commenttree *cmtUp;
    struct commenttree *cmtLeft;
    struct commenttree *cmtRight;
    struct cmntline *commts;
};

/* ***************************************** *
 * Structures for ROF files                  *
 * ***************************************** */

/* ROF header structure */

typedef struct rof_hdr {
            int   sync[2],
                  ty_lan;
            char  valid,
                  rdate[5],
                  edition,
                  version;
            int   udatsz,
                  udpsz,
                  idatsz,
                  idpsz,
                  codsz,
                  stksz,
                  modent;
            char  rname[20];
} ROFHD;

/* Global definitions */

struct rof_glbl {
            char  name[30];
            char  Type;
            int   Ofst;
};

/* External references */

struct rof_extrn {
            char  name[20];        /* External name                  */
            char  Type;             /* Type Flag                      */
            int   Ofst;             /* Offset into code               */
            int   Extrn;            /* Flag that it's an external ref */
            struct rof_extrn *up,
                             *LNext,
                             *RNext;
};

/* ************************************************************** *
 * Bitmasks for Type (all reference types)                        *
 *                                                                *
 * bit 7: Ref stored relative to the location of the reference    *
 * bit 6: Ref should be negated after use (?)                     *
 * bit 5: Ref is located in the code section                      *
 * bit 4: If date, ref is DP                                      *
 * bit 3: Ref is only 1 byte long                                 *
 * bit 2: Ref refers to code                                      *
 * bit 1: Ref refers to DP data                                   *
 * bit 0: Ref refers to init data                                 *
 *                                                                *
 * Special case: if both bits 1 & 2 are set, ref is constant val  *
 *                                                                *
 * Type is bit-mapped in the lower 3 bits                         *
 * bit 2,1:                                                       *
 *    110 = constant                                              *
 *    100 = code                                                  *
 *    010 = DP data                                               *
 *    000 = non-DP data                                           *
 * bit 000: Set if this is a ref to init data                     *
 *                                                                *
 * Global variable Table                                          *
 *  (04) 00000100 = to code                                       *
 *  (02) 00000010 = to DP bss                                     *
 *  (01) 00000001 = to non-DP data                                *
 *  (00) 00000000 = to non-DP bss                                 *
 *                                                                *
 * External Reference Table                                       *
 *  (a0) 10100000 = in code/word pcr                              *
 *  (20) 00100000 = in code/word                                  *
 *                                                                *
 * Local Variable Table                                           *
 *  (2a) 00101010 = in code/byte - to non-DP bss                  *
 *  (22) 00100010 = in code/word - to DP bss                      *
 *  (21) 00100001 = in code/word - to non-dp data                 *
 *  (20) 00100000 = in code/word - to non-DP bss                  *
 *  (04) 00000100 = in non-DP data/word - to code                 *
 *  (02) 00000010 = in non-DP data/word - to non-DP data          *
 *  (00) 00000000 = in non-DP data/word - to non-DP bss           *
 * ************************************************************** */

