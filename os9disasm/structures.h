
/* OS9 Module header defs */
/* Note:  Coco Integers are represented as chars due to big-endian
 * translations
 */

struct modhead {
	unsigned char M_ID[2],	/* Sync bytes ($87CD)	*/
	     M_Size[2],	/* Module Size		*/
	     M_Name[2];	/* Ptr to Module Name	*/
	unsigned char M_Type,	/* Type / Language	*/
	     M_Revs,	/* Attributes/Rev Level	*/
	     M_Parity,	/* Header Parity	*/
	     M_Exec[2],	/*Execution Entry Offset*/
	     M_Mem[2];	/*Stack Requirement	*/
};

/* This structure dflts contains the list of defaults/options
 * under which the program is running	*/

struct dflts {
	char ShLbl;	/* 0 = 8-char labels; 1 = 6-char labels */
	char OSType;	/* See OS_* defines */
};

/* Symbol Table tree structure */

#define NLMAX 8

struct nlist {
	char sname[NLMAX+1];	/* symbol name	*/
	int myaddr;		/* address of symbol */
	int stdnam;		/* flag that it's a std named label */
	struct nlist *LNext;	/* left next	*/
	struct nlist *RNext;	/* right next	*/
	struct nlist *parent;	/* parent	*/
};

/* Offset [ (-L0053) - types stuff ] */

struct ofsetree {
	char oclas_maj;	/* Class to use in offset addressing	*/
	int  of_maj;	/* The main offset value		*/
	char incl_pc;	/* Flag to include PC offset mode	*/
	int add_to;	/* Flag: if set, means add offset, else subtract */
	/*int  of_sec;	 Secondary offset (0 if none)		*/
	/*char oclas_sec;	 Class of secondary offset		*/
};

/* Data areas/Label Addressing Modes tree structure */

struct databndaries {
	int b_lo;
	int b_hi;
	char b_typ;	/* Boundary type for DA's, Lbl Class for A-Modes */
	char b_class;	/* class def for type L and S Data boundaries    */
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
	     lbnm[10],
	     mnem[6],
	     operand[50];
};
	     
