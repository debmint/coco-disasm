/*############################################################################
#                                                                            #
#  os9disasm - OS9-6809 CROSS DISASSEMBLER                                   #
#             following the example of Dynamite+                             #
#                                                                            #
# # ######################################################################## #
#                                                                            #
#  $Id$
#                                                                            #
#  Edition History:                                                          #
#  #  Date       Comments                                              by    #
#  -- ---------- -------------------------------------------------     ---   #
#  01 2003/01/31 First began project                                   dlb   #
##############################################################################
#    File:  progdis.c                                                        #
# Purpose:  do disassembly of program file                                   #
#############################################################################*/

#include "odis.h"
#include "dtble.h"

/* Flag whether Databoundary or Addressing Mode */
#define DBOUND 1
#define ADRMOD 2
#define NOIDXINDIR if(c&0x20) return 0

char *RegReg[] = {"d","x","y","u","s","pc","","","a","b","cc","dp"};
char *RegR03[] = {"d","x","y","u","s","pc","w","v","a","b","cc","dp",
							"0","","e","f"};
char *PshPuls[] = {"cc","a","b","dp","x","y","u","pc"};
char *PshPulu[] = {"cc","a","b","dp","x","y","s","pc"};
char RegOrdr[] = "xyus";

/*extern struct lkuptbl *Pre10, *Pre11, *Byte1;*/
extern char *pseudcmd,*realcmd;

char CmdBuf[10];	/* buffer to hold bytes of cmd code */
int UnkCount;		/* Count of unknowns now being held */

struct databndaries *curbnd;
struct databndaries *amodes[sizeof(lblorder)];
struct lkuptbl *tabl;
struct printbuf *pbuf=&PBuf;

int CodEnd;	/* End of executable code (ModSize-3 for OS9 */

/* progdis(): mainline disassembly routine	*/
void progdis()
{
	/* Work done by pass 1:
	 * 	read code char by char.. determine addresses
	 * 	and set up all labels by type.  The user-defined
	 * 	mnemonic label names are not read at this time
	 */

	LinNum=PgLin=0;

	/* Be sure we start with a clear pbuf */
	memset(pbuf,0,sizeof(struct printbuf));
	
	if( Pass2 ) {
		if( OSType == OS_9 )
			OS9Modline();
		WrtEquates(1);  /* now do standard named labels */
		WrtEquates(0);  /* write non-standard labels */
		if(OSType == OS_9)
			OS9DataPrint();
	}

	/* Now begin parsing through program */


	Pc=HdrLen; /* Entry point for executable code */
	while( Pc < (CodEnd) ) {
		register struct databndaries *bp;

		CmdLen=0; strcpy(CmdBuf,"");
			/* Try this to see if it avoids buffer overflow */
		memset(pbuf,0,sizeof(struct printbuf));
		CmdEnt = Pc;
			/* check if in data boundary */
		if( (bp=ClasHere(dbounds,Pc)) ) {
			NsrtBnds(bp);
		}
		else {
			/* CodEnd added so code won't be generated past
			 * CRC bytes for OS9 Module	*/
			if( GetCmd() && (Pc<=CodEnd) ) {
				if(Pass2) {
					PrintLine(realcmd,pbuf);
				}
					
					/* allocate byte */
			}	/* if ! GetCmd	*/
			else {
				if(Pass2) {
					memset(pbuf,0,sizeof(struct printbuf));
					sprintf(pbuf->instr,"%02x",
						ModBegin[CmdEnt] & 0xff);
					strcpy(pbuf->mnem,"fcb");
					strcpy(pbuf->operand,"$");
					PBFcat(pbuf->operand,"%02x",
						&ModBegin[CmdEnt],1);
					PrintLine(pseudcmd,pbuf);
				}
				Pc = ++CmdEnt;
			}
		}
	}
	if(Pass2) {
		if(OSType == OS_9)
			WrtEmod();
	}
}

/*  NsertBnds():	Insert boundary area */

void NsrtBnds(struct databndaries *bp)
{
	memset(pbuf,0,sizeof(struct printbuf));
	AMode = 0;	/* To prevent LblCalc from defining class */
	NowClass = bp->b_class;
	PBytSiz = 1;	/* Default to one byte length */

	switch(bp->b_typ) {
		case 1: /* Ascii */
			MovASC( (bp->b_hi) -(bp->b_lo)+1);
			break;	/* bump PC  */
		case 6: /* Word */
		case 4: /* Long */
			PBytSiz = 2;	/* Takes care of both Word & Long */
		case 2:	/* Byte */
		case 5: /* Short */
			/*if( (bp->b_typ==1) || (bp->b_typ==5) )
				PBytSiz=1;
			else
				PBytSiz=2;*/
			MovBytes(bp);
			break;
		case 3: /* "C"ode .. not implememted yet */
			break;
		default:
			break;
	}
	NowClass = 0;
	/*Pc=(bp->b_hi)+PBytSiz;*/	/* Not needed for Pass2, but no harm */
}

void MovBytes(struct databndaries *db)
{
	char tmps[20];
	int x;

	CmdEnt=Pc;
	while( Pc <= db->b_hi ) {
		tmps[0] = '\0';
		x= (PBytSiz==1 ? (unsigned char)ModBegin[Pc] :
				o9_int(&ModBegin[Pc])&0xffff);
		LblCalc(tmps,x,AMode);
		if(Pass2) {
			if(strlen(pbuf->instr) < 12) {
				PBFcat(pbuf->instr,"%02x ",&ModBegin[Pc],1);
				if( (PBytSiz==2) && (strlen(pbuf->instr)<12) )
					PBFcat(pbuf->instr,"%02x ",
							&ModBegin[Pc+1],1);
			}
			if(strlen(pbuf->operand))
				strcat(pbuf->operand,",");
			strcat(pbuf->operand,tmps);
			if(strlen(pbuf->operand) > 22) {
				strcpy(pbuf->mnem, PBytSiz==1 ? "fcb" : "fdb");
				PrintLine(pseudcmd,pbuf);
				CmdEnt = Pc + PBytSiz;
			}
		}
		Pc += PBytSiz;
	}
	if(strlen(pbuf->operand)) {
		strcpy(pbuf->mnem, PBytSiz==1 ? "fcb" : "fdb");
		PrintLine(pseudcmd,pbuf);
	}
}

/* MovAsc() - Move nb byes int fcc (or fcs) statement */
void MovASC(int nb)
{
	register unsigned char *h=&ModBegin[Pc];
	struct nlist *nlp;
	char oper_tmp[30];

	memset(pbuf,0,sizeof(pbuf));
	strcpy(pbuf->mnem,"fcc");	/* Default mnemonic to "fcc" */
	CmdEnt=Pc;

	*oper_tmp = '\0';
	while(nb--) {
		register int x;
		char c[6];

		if( (isprint(x = *h)) || (x&0x80 && UseFCC && isprint(x&0x7f)) ) {
			if(Pass2) {
				if(strlen(pbuf->instr) < 12)
					PBFcat(pbuf->instr,"%02x ",h,1);
				x &= 0x7f;
				sprintf(c,"%c",x);
				strcat( oper_tmp, c);
				if( Pass2 && (*(h) &0x80) ) {
					strcpy(pbuf->mnem, "fcs");
					AddDelims(pbuf->operand,oper_tmp);
					PrintLine(pseudcmd,pbuf);
					*oper_tmp = '\0';
					CmdEnt=Pc+1;
					strcpy(pbuf->mnem, "fcc");
				}
				if( (strlen(oper_tmp) > 24) ||
						(strlen(oper_tmp) &&
						FindLbl(ListRoot('L'),Pc+1))) {
					AddDelims(pbuf->operand,oper_tmp);
					PrintLine(pseudcmd,pbuf);
					*oper_tmp = '\0';
					CmdEnt=Pc+1;
					strcpy(pbuf->mnem,"fcc");
				}
			}
		}
		else {	/* it's a control character */
			if( Pass2 && (strlen(oper_tmp)) ) {
				AddDelims(pbuf->operand,oper_tmp);
				PrintLine(pseudcmd,pbuf);
				*oper_tmp = '\0';
				CmdEnt=Pc;
			}
			if(!Pass2) {
				if( (x&0x7f) < 33 ) {
					addlbl(x&0x7f,strpos(lblorder,'^'));
				}
			}
			else {
				strcpy(pbuf->mnem,"fcb");
				PrintLbl(pbuf->operand,strpos(lblorder,'^'),
						x,nlp);
				sprintf(pbuf->instr,"%02x",x & 0xff);
				PrintLine(pseudcmd,pbuf);
				strcpy(pbuf->mnem,"fcc");
			}
			CmdEnt=Pc+1;
		}
		++Pc; ++h;
	}
	if(strlen(oper_tmp)) {	/* Clear out any pending string */
		AddDelims(pbuf->operand,oper_tmp);
		PrintLine(pseudcmd,pbuf);
		*oper_tmp = '\0';
	}
}

	/* AddDelims() - Add delimiters for fcc/fcs operand -
	 * 		checks string for nonexistand delimiter
	 * 		and copies string with delims to destination
	 */
void AddDelims( char *dest,char *src )
{
	char delim;

	if( !strchr(src,'"') )
		delim = '"';
	else
		if( !strchr(src, '/') )
			delim = '/';
		else
			if( !strchr(src,'#') )
				delim = '#';
			else
				if( !strchr(src,'\\') )
					delim = '\\';

	sprintf(dest,"%c%s%c",delim,src,delim);
}


	/* IsCode() - Checks to see if code pointed to by p is valid code.
	 * Returns: pointer to valid lkuptable entry or 0 on fail
	 */

struct lkuptbl *IsCmd(char *p)
{
	struct lkuptbl *T;	/* pointer to appropriate tbl	*/
	register int sz;	/* # entries in this table	*/
	char c;
	
	switch( c = *p ) {
		case '\x10':
			T=Pre10; c = *(++p);
			sz=sizeof(Pre10)/sizeof(Pre10[0]);
			break;
		case '\x11':
			T=Pre11; c = *(++p);
			sz=sizeof(Pre11)/sizeof(Pre11[0]);
			break;
		default:
			T=Byte1;
			sz=sizeof(Byte1)/sizeof(struct lkuptbl);
			break;
	}

	while( (T->cod != c) ) {
		if( --sz == 0 )
			return 0;
		++T;
	}
	AMode=T->amode;
	return ( (T->cod == c) && (T->t_cpu <= CpuTyp) ) ? T : 0;
}

/*	GetCmd():	parse through data and collect bytes of cmd */
int GetCmd()
	
{
	char *p,
	     tmp[25];
	struct lkuptbl *tbl;
	register int offset = 0,
		     noncode = 0;
	unsigned char pbyte;
	char **cptr;

	*tmp = '\0';
	CmdLen=0;
	CmdEnt = Pc;	/* Save this entry point for case of bad code */
	
	p = &(ModBegin[Pc]);	/* Start at first byte */
	
	/*while( !(tbl=IsCmd( &(ModBegin[Pc]) )) ) {*/
	if( !(tbl=IsCmd( &(ModBegin[Pc]) )) ) {
	/*	++Pc;
		++noncode;*/
		/* Need to provide for cleanup of noncode */
		return 0;
	}
	
	/* At this point
	 *   |  |  |  |  |  |  | ...
	 *   ^           ^
	 *   p    Pc=first executable code	*/
	
	/* If illegal code is present, then print it out if pass 2 */
	if(noncode && Pass2) {
		/* Pc hasn't moved, data from Pc to
		 * p - 1 is noncode	*/
		return 0;
	}
	p = &ModBegin[Pc];  /* Now set ptr to current code begin */
	/* Now move stuff to printer buffer */
	switch(ModBegin[CmdEnt]) {
		case '\x10':
		case '\x11':
			offset=2;
			break;
		default:
			offset=1;
	}
	if(offset==2)
		PBFcp(pbuf->instr,"%04x",&ModBegin[CmdEnt],2);
	else
		PBFcp(pbuf->instr,"%02x",&ModBegin[CmdEnt],1);
		
	Pc+=offset;
	offset=0;	/* reset offset - not sure if needed */
	strcpy(pbuf->mnem,tbl->mnem);
	AMode = tbl->amode; PBytSiz=tbl->adbyt;
	
	/* Special case for OS9 */
	if( (OSType == OS_9) && !(strncasecmp(tbl->mnem,"swi2",4)) ) {
		register unsigned char ch;
		register struct nlist *nl;

		strcpy(pbuf->mnem,"os9");

		ch=ModBegin[Pc++];
		sprintf(pbuf->opcod,"%02x",ch);
		if(!Pass2) {
			addlbl(ch,strpos(lblorder,'!'));
		}
		else {
			if( (nl=FindLbl(SymLst[strpos(lblorder,'!')],ch)) ) {
				strcat(pbuf->operand,nl->sname);
			}
			else
				sprintf(pbuf->operand,"%02x",ch);
		}
		return 1;
	}

	/* take care of aim/oim/tim immediate # */
	/* TODO  add addressing mode for this!!! */
	if( strstr(tbl->mnem,"im") )
	{
		unsigned char im = ModBegin[Pc++];
		sprintf(pbuf->opcod,"%02x",im);
		sprintf(pbuf->operand,"#$%02x,",im);
	}

	switch(AMode) {
		register int ct;
		register char **regpt;
		char tmpbyt,
		     tmpbuf[15];
			
		
		case AM_INH:
			return 1; /*Nothing else to do */
		case AM_XIDX:
			if(!TxIdx()){	/* Process Indexed mode */
				return 0; /* ?????? */
			}
			return 1;
			break;
		case AM_PSH:
			pbyte = ModBegin[Pc++];
			sprintf(pbuf->opcod,"%02x",pbyte&0xff);
			cptr = PshPuls;
			if( (pbyte== 0x36) || (pbyte==0x37) )
				cptr = PshPulu;
			for(ct=0;ct<8;ct++) {
				if( pbyte & 1 ) {
					if(strlen(pbuf->operand)) {
						strcat(pbuf->operand,",");
						strcat(pbuf->operand,cptr[ct]);
					}
					else
						strcpy(pbuf->operand,cptr[ct]);
				}
				pbyte = pbyte>>1;
			}
			break;
		case AM_TFM:
			pbyte = ModBegin[Pc++];
			sprintf(pbuf->opcod,"%02x",pbyte&0xff);
			ct=(pbyte>>4)&0x0f; pbyte&=0x0f;
			
			if( (ct > 4) || (pbyte >4) )
			{
				return 0;	/* Illegal */
			}
				/* do r0 */
			strcpy(pbuf->operand,RegReg[ct]);

			if( !((tbl->cod) & 1) )
			{
				strcat(pbuf->operand,"+");
			}
			else {
				if(tbl->cod == 0x39)
				{
					strcat(pbuf->operand,"-");
				}
			}
				/* now "r1" */	
			strcat(pbuf->operand,",");
			strcat(pbuf->operand,RegReg[(pbyte) &0x0f]);
			switch(tbl->cod) {
				case 0x38:
				case 0x3b:
					strcat(pbuf->operand,"+");
					break;
				case 0x39:
					strcat(pbuf->operand,"-");
					break;
				default:
					break;
			}
			break;
		case AM_REG:
			if( CpuTyp == M_09 )
			{
				regpt = RegReg;
			}
			else {
				regpt = RegR03;
			}
			
			pbyte = ModBegin[Pc++];
			sprintf(pbuf->opcod,"%02x",pbyte&0xff);
			ct=(pbyte>>4)&0x0f; pbyte&=0x0f;

			/* abort if 6809 codes are out of range */
			if( CpuTyp == M_09 )
			{
				if( (ct > 0x0b) || (pbyte > 0x0b) )
				{
					return 0;	/* Illegal */
				}
			}
			
			if(!strlen(regpt[ct]) || !strlen(regpt[pbyte]) )
				return 0;
			
			/* Can't tfr between different-sized registers */
			if( ct != 0x0c )  /* "0" register for 6309 */
			{
				if( (ct&0x08) != (pbyte&0x08) )
				{
					return 0;	/* mismatched register size */
				}
			}
			
			strcpy(pbuf->operand,regpt[ct]);
			strcat(pbuf->operand,",");
			strcat(pbuf->operand,regpt[(pbyte) &0x0f]);
			break;
		case AM_BYTI:
		case AM_DIMM:
		case AM_XIMM:
		case AM_YIMM:
		case AM_UIMM:
		case AM_WIMM:
			strcpy(pbuf->operand,"#");
		case AM_DRCT:
		case AM_EXT:
		case AM_REL:
			switch ( tbl->adbyt ) {
				case 4:   /* only one instance, "ldq (immediate mode)" */
					offset=o9_int(&ModBegin[Pc]);
					sprintf(pbuf->opcod,"%04x",offset&0xffff);
					strcat(pbuf->operand,"$");
					strcat(pbuf->operand,pbuf->opcod);
					Pc+=2;
					offset=o9_int(&ModBegin[Pc]);
					sprintf(tmpbuf,"%04x",offset&0xffff);
					strcat(pbuf->opcod,tmpbuf);
					strcat(pbuf->operand,tmpbuf);
					Pc+=2;
					return 1;  /* done */
				case 2:
					offset=o9_int(&ModBegin[Pc]);
					sprintf(tmpbuf,"%04x",offset&0xffff);
					strcat(pbuf->opcod,tmpbuf);
					Pc+=2;
					break;
				default:
					offset = ModBegin[Pc++];
					if( (AMode == AM_DRCT) || (AMode == AM_BYTI) )
						offset &= 0xff;
					sprintf(tmpbuf,"%02x",offset&0xff);
					strcat(pbuf->opcod,tmpbuf);
					break;
			}
	
			LblCalc(tmp,offset,AMode);
			strcat(pbuf->operand,tmp);
			break;
			
		case AM_BIT:
			pbyte = ModBegin[Pc++];
			sprintf(pbuf->opcod,"%02x",pbyte);
			pbyte &=0xff;
			tmpbyt=pbyte>>6;
			
			if( tmpbyt > 3 )
				return 0;
			
			sprintf(pbuf->operand,"%s.",PshPuls[tmpbyt]);
			tmpbyt = (pbyte>>3) & 0x7;
			sprintf(tmpbuf,"%d,",tmpbyt); strcat(pbuf->operand,tmpbuf);
			sprintf(tmpbuf,"$%02x",ModBegin[Pc++]);
			strcat(pbuf->operand,tmpbuf);strcat(pbuf->operand,".");
			strcat(pbuf->opcod,&tmpbuf[1]);
			tmpbyt = pbyte & 0x7;
			sprintf(tmpbuf,"%d",tmpbyt); strcat(pbuf->operand,tmpbuf);
			break;
			
	}
	
	if(UpCase)
		UpString(pbuf->operand);
	return 1;
}

int TxIdx()
{
	unsigned char c = ModBegin[Pc];
	struct databndaries *kls;	/* Class for this address */
	short offset;
	char myclass;
	char oper1[25];
	char oper2[5];
	char regNam;

	*oper1 = *oper2 = '\0';
	PBFcat(pbuf->opcod,"%02x ",&ModBegin[Pc++],1);

	/* Extended indirect    [mmnn]*/
	if(c==0x9f) {
		register unsigned short da;
		
		AMode = AM_EXT;
		da=o9_int(&ModBegin[Pc]);
		PBFcat(pbuf->opcod,"%04x",&ModBegin[Pc],2); Pc+=2;
		
		LblCalc(oper1,da,AMode);
		/*sprintf(pbuf->operand,"[%s]",oper1);*/
		strcat(pbuf->operand,"[");
		strcat(pbuf->operand,oper1);
		strcat(pbuf->operand,"]");
		return 1;
	}
	else {	
		regNam = RegOrdr[((c >> 5) & 3)]; /* Current register offset */
		if(UpCase)
			regNam = toupper(regNam);
		AMode += (c>>5) & 3;
	
		if( !(c & 0x80) ) {	/* 0RRx xxxx = 5-bit	*/
			int sbit;	/*the offset portion of the postbyte*/

			if( !(c & 0x1f) )	/* 0,r not valid asm mode? */
				return 0;
			sbit=c&0x0f;

			if(c&0x10) {	/* sign bit */
				/* Test the following */
			/*	sbit = 0x10-sbit;
				strcpy(pbuf->operand,"-");*/
				sbit -= 0x10;
			}
			LblCalc(oper1,sbit,AMode);
			sprintf(oper2,"%s,%c",oper1,regNam);
			strcat(pbuf->operand,oper2);
			return 1;
		}
		else {
			if( (kls = ClasHere(LAdds[AMode],Pc)) ) {
				myclass = kls->b_typ;
				/* set up offset if present */
			}
			else {
				myclass = DEFAULTCLASS;
			}
			/* Set up for n,R or n,PC later if other
			 * cases don't apply */
			switch( c & 1) { /* postbyte size*/
				case 0:	/* single byte */
					offset = ModBegin[Pc];
					break;
				default:   /* 16-bit (only other option */
					offset = o9_int(&ModBegin[Pc]);
					break;
			}
			PBytSiz=(c&1) + 1;
			
			switch( c & 0x0f ) {
				case 0:
					sprintf(oper1,",%c+",regNam);
					break;
				case 1:
					sprintf(oper1,",%c++",regNam);
					break;
				case 2:
					sprintf(oper1,",-%c",regNam);
					break;
				case 3:
					sprintf(oper1,",--%c",regNam);
					break;
				case 4:
					sprintf(oper1,",%c",regNam);
					break;
				case 5:
					sprintf(oper1,"b,%c",regNam);
					if(UpCase)
						UpString(oper1);
					break;
				case 6:
					sprintf(oper1,"a,%c",regNam);
					if(UpCase)
						UpString(oper1);
					break;
				case 0x0b:
					sprintf(oper1,"d,%c",regNam);
					if(UpCase)
						UpString(oper1);
					break;
				case 0x08: /* <n,R */
				case 0x09: /* >nn,R */
					if(offset<0x127 && offset>-128 && c&1)
						strcpy(oper1,">");
					LblCalc(oper1,offset,AMode);
					sprintf(oper2,",%c",regNam);
					PBFcat(pbuf->opcod,c&1?"%04x":"%02x",
							&ModBegin[Pc],(c&1)+1);
					Pc+=(c&1)+1;
					break;
				case 0x0c:	/* n,PC (8-bit) */
				case 0x0d:	/*nn,PC (16 bit) */
					AMode = AM_REL;
					/* below is a temporary fix */
					myclass=DEFAULTCLASS;
					PBFcat(pbuf->opcod,c&1?"%04x":"%02x",
							&ModBegin[Pc],(c&1)+1);
					Pc+=(c&1)+1;
					
					if(offset<0x127 && offset >-128 && c&1)
						strcpy(oper1,">");
					else
						if( !(c&1))
							strcpy(oper1,"<");
					
					LblCalc(oper1,offset,AMode);
					sprintf(oper2,",%s","pcr");
					break;
				default: /* Illegal Code */
					return 0;
			}
		}
	}
	if(UpCase)
		UpString(oper2);
	
	if( c & 0x10 )
	{
		/*sprintf( pbuf->operand, "[%s%s]",oper1,oper2);*/
		strcat( pbuf->operand,"[" );
		strcat( pbuf->operand, strcat(oper1,oper2) );
		strcat( pbuf->operand,"]" );
	}
	else {
		/*sprintf( pbuf->operand, "%s%s",oper1,oper2);*/
		strcat( pbuf->operand, strcat(oper1,oper2) );
	}

	return 1;
}

/* WrtOfst - write operand data to tmp buffer */

int WrtOfst(int adr,char clas,char *dst1,char *dst2,char *RgN,int bytlen)
{
	struct nlist *nl;
	
	if(!Pass2) {
		addlbl(adr,clas);
	}
	else {
		if(!(nl=FindLbl(SymLst[(int)clas],adr)))
			return 0;
		PBFcat(pbuf->opcod,(bytlen==2 ? "%04x" : "%02x"),
					&ModBegin[Pc],bytlen);
		strcpy(dst1,nl->sname); strcat(dst1,",");
		strcat(dst2,RgN);
	}
	Pc+=bytlen;
	return 1;
}

/*	This that follows may not be exactly what we will use
 *	but save it.. it's the pattern for pasing the tree
 */


char prfmt[] = "%10s %04x\n";

void DoPrt(struct nlist *nl) {
	if( !nl ) {
		fprintf(stderr,"Error, ptr is null!\n");
		return;
	}
	if(nl->LNext) {
		DoPrt(nl->LNext);
	}
	printf(prfmt,nl->sname,nl->myaddr);
	if(nl->RNext) {
		DoPrt(nl->RNext);
	}
}

void listlbls(){
	struct nlist *nl;
	int count;

	for( count=1;count<strlen(lblorder); count++) {
		nl=SymLst[count];
		if(nl) {
			printf("Labels of class: %c\n",lblorder[count]);
			DoPrt(nl);
		}
	}
}

void os9hdr(void)
{
	/* Check to be sure it _is_ a 6809 Memory Module */
	if((*ModBegin != '\x87') || (ModBegin[1] != '\xCD')) {
		fprintf(stderr,"The file %s is NOT a 6809 Memory Module!!!\n",
				modfile);
		exit(1);
	}

	/* Translate Header information in Header to big-endian format */
	{
		register struct modhead *mhd = (struct modhead *)ModBegin;

		ModSiz = o9_int(mhd->M_Size);
		addlbl(ModSiz,strpos(lblorder,'L'));
		ModNam = o9_int(mhd->M_Name);
		addlbl(ModNam,strpos(lblorder,'L'));
		ModExe = o9_int(mhd->M_Exec);
		addlbl(ModExe,strpos(lblorder,'L'));
		ModTyp = (unsigned int)mhd->M_Type;
		if((ModTyp < 3) || (ModTyp == 0x0c) || (ModTyp > 0x0b)) {
			ModData= o9_int(mhd->M_Mem);
			PBytSiz=2;	/* Kludge??? */
			addlbl(ModData,strpos(lblorder,'D'));
			HdrLen = 13;
		}
		else {
			ModData = -1; /* Flag as not used */
			HdrLen = 9;
		}
		CodEnd=ModSiz-3;

		/* EndAdr: Ptr to end of code (less OS9 CRC bytes
		 * .. actually, next byte past last executable code
		 * .. (first byte of CRC
		 */
		EndAdr = (int)ModBegin + ModSiz -3;
	}
	
}

