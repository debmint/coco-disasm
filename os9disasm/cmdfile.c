/*############################################################################
#
# os9disasm - a project to disassemble Os9-coco modules into source code
#             following the example of Dynamite+
#             
# # ######################################################################## #
#
#  $Id$
#                                                                            #
#  Edition History:                                                          #
#  #  Date       Comments                                              by    #
#  -- ---------- -------------------------------------------------     ---   #
#  01 2003/01/31 First began project                                   dlb   #
##############################################################################
# File:  cmdfile.c                                                           #
# Purpose: process command file                                              #
############################################################################*/

#include "odis.h"
#include "amodes.h"

int NoEnd;	/* Flag that no end on last bound (probably can go static) */
int TypeGet;	/* Flag 1=getting Addressing mode 0=Data Boundary	*/

void do_cmd_file()
{
	FILE *cmdfp;
	char miscbuf[240];
	char *mbf = miscbuf;
	
	NxtBnd = 0;	/* init Next boundary pointer */
	if( !(cmdfp=fopen(cmdfilename,"rb")) ) {
		fprintf(stderr,"Erorr # %d: Cannot open cmd file %s for read\n",
				errno,cmdfilename);
		exit(1);
	}

	while(fgets(miscbuf,sizeof(miscbuf),cmdfp)) {
		char *th;
		++LinNum;
		if( (th=(char *)strchr(miscbuf,'\n')) )
			*th = '\0';
		mbf = skipblank(miscbuf);
		if(!strlen(mbf) )	/* blank line? */
			continue;
		if(*mbf == '*')		/* Comment?	*/
			continue;	/*yes, ignore	*/
		if(*mbf == '+') {
			if(optincmd(++mbf)==-1)
				cmderr();
			continue;
		}
		if(*mbf == '>') {
			cmdamode(skipblank(++mbf));
			continue;
		}
		if( strchr(BoundsNames,toupper(*mbf)) || isdigit(*mbf) ) {
			boundsline(mbf);
		}
		else {
			fprintf(stderr,"Illegal cmd or switch in line %d\n",
					LinNum);
			exit(1);
		}
	}
}

int cmderr()
{
	fprintf(stderr,"Error processing cmd line #%d\n",LinNum);
	exit(1);
}

/* ****************************************** *
 * Process options found in command file line *
 * ****************************************** */

int optincmd(char *lpos)
{
	char st[80], *spt = st;
	
	while( *(lpos=skipblank(lpos)) ) {
		if(sscanf(lpos,"%80s",spt)!=1)
			return(-1);
		lpos+=strlen(spt);
		if(*spt == '-')
			++spt;
		do_opt(spt);
	}
	return(0);
}

/* ************************************************************** *
 * cmdsplit() :  split cmdline..  i.e. transfer characters up to  *
 * ';' or end of line - returns ptr to next char in cmdline buffer*
 * ************************************************************** */

char *cmdsplit(char *dest, char *src)
{
	char c;

	src=skipblank(src);
	if(!(c=*src) || (c=='\n') )
		return 0;

	if(strchr(src,';')) {
		while((c=*(src++)) != ';')
			*(dest++) = c;
		*dest = '\0';
	}
	else {
		strcpy(dest,src);
		src += strlen(src);
	}
	return src;
}


/* *************************************************** *
 * Process addressing modes found in command file line *
 * *************************************************** */

void cmdamode(char *pt)
{
	char buf[80];

	TypeGet=1;
	while( (pt=cmdsplit(buf,pt)) ) {
		DoMode(buf);
	}
}


int DoMode(char *lpos)
{
	struct databndaries *mptr;
	register int class;	/* addressing mode */
	register int notimm=5;
	char c;
	int lo,hi,bndty;
	register struct databndaries *lp;
	struct ofsetree *otreept = 0;

	if( *(lpos=skipblank(lpos)) == '#' ) {
		notimm=0;
		lpos=skipblank(++lpos);
	}
	switch (c=toupper(*(lpos++))) {
		case 'D':
/*			if(notimm)
				AMode=AM_DRCT;
			else
				AMode=AM_DIMM;
			break;*/
		case 'X':
		case 'Y':
		case 'U':
		case 'S':
				AMode = (int)strpos("DXYUS",c)+notimm+1; /*+2*/
				if( ! AMode )
				{
					nerrexit("Class \'%c\' not found");
				}
			break;
		case '1':
			AMode=AM_BYTI;
			break;
		case 'E':
			AMode=AM_EXT;
			break;
		case 'R':
			AMode=AM_REL;
			break;
		default:
			nerrexit("Illegal addressing mode");
			exit(1);	/* not needed but just to be safe */
	}

	lpos=skipblank(lpos);
	if( !(class=(int)strpos(lblorder,*(lpos++))) )
		nerrexit("Illegal class definition");
	
		/* Offset spec (if any) */
	
		/* check for default reset (no address) */
	if( !(lpos=skipblank(lpos)) || !(*lpos) || (*lpos==';') ) {
		DfltLbls[AMode-1] = lblorder[class-1];
		return 1;
	}
	
	if( *(lpos) == '(') {
		otreept = calloc (1, sizeof(struct ofsetree) );
		if( !otreept)
			nerrexit("Cannot allocate memory for offset!");
		lpos = setoffset(++lpos,otreept);
	}
	
	lpos=skipblank(lpos);
	getrange(lpos,&lo,&hi,bndty);
	
		/* Now insert new range into tree */
	if( !(mptr=calloc (1, sizeof(struct databndaries))) )
		nerrexit("Cannot allocate memory for data definition");

	mptr->b_lo=lo;mptr->b_hi=hi;
	mptr->DLeft=mptr->DRight=0;mptr->b_typ=class; /*a_mode;*/
	mptr->dofst=otreept;

	if(!LAdds[AMode]) {
		LAdds[AMode]=mptr; mptr->dabove=0;
	}
	else {
		lp=LAdds[AMode];
		while(1) {
			if(hi<lp->b_lo) {
				if(lp->DLeft) {
					lp=lp->DLeft;
					continue;
				}
				else {
					lp->DLeft=mptr;
					mptr->dabove=lp;
					return 1;
				}
			}
			else {
				if(lo>lp->b_hi) {
					if(lp->DRight) {
						lp=lp->DRight;
						continue;
					}
					else {
						lp->DRight=mptr;
						mptr->dabove=lp;
						return 1;
					}
				}
				else {
					nerrexit("Addressing mode segments overlap");
				}
			}
		}
	}
	return 1;
}

#define ILLBDRYNAM "Illegal boundary name in line %d\n"

/* ********************************************* *
 * process boundaries found in command file line *
 * ********************************************* */

void boundsline(char *mypos)
{
	char tmpbuf[80];
	register char *hold;
	/*char *stophere=&mypos[strlen(mypos)];*/
	register int count=1;

	TypeGet=0;
	
	if(isdigit(*mypos)) {	/*	Repeat count for line	*/
		mypos=movdigit(tmpbuf,mypos);
		count=atoi(tmpbuf);
		mypos=skipblank(mypos);
	}

	hold=mypos;
	
	while(count--) {
		char *nextpos;
		mypos=hold;
		while( (nextpos=cmdsplit(tmpbuf,mypos)) ) {
			setupbounds(tmpbuf);
			mypos=nextpos;
		}
	}
}

/* ************************************************* *
 * set up offset (if there) for offset specification *
 * ************************************************* */

char *setoffset(char *p, struct ofsetree *oft)
{
	char c,bufr[80];
	
	oft->oclas_maj = oft->of_maj = oft->add_to = 0;
	
	p=skipblank(p);
	if( !strchr(p,')') )
		nerrexit("\"(\" in command with no \")\"");
	if( (c=toupper(*(p++))) == '*') {
		/* Hope this works - flag * addressing like this  */
		oft->incl_pc++;
		p=skipblank(p); /* used this char, position for next */
		c=toupper(*(p++));
	}
	switch(c) {
		case '+':
			++oft->add_to;
			break;
		case '-':
			break;
		case ')':
			if( !oft->incl_pc)
				nerrexit("Blank offset spec!!!");
			return p;
		default:
			nerrexit("No '+', '-', or '*' in offset specification");
	}
	
	p=skipblank(p); c=toupper(*(p++));
	if(!(oft->oclas_maj=(int)strpos(lblorder,c)))
		nerrexit("No offset specified !!");

	p=skipblank(p);
	if( !isxdigit(*p) )
		nerrexit("Non-Hex number in offset value spec");
	/* NOTE: need to be sure string is lowercase and following
	* value needs to go into the structure */
	p=movxnum(bufr,p);
	sscanf(bufr,"%x",&oft->of_maj);
	/*if(add_to)
		oft->of_maj = -(oft->of_maj);*/

	if(*(p=skipblank(p)) == ')' ) {
		addlbl(oft->of_maj,strpos(lblorder,c));
	}
	else
		nerrexit("Illegal character.. offset must end with \")\"");
	return ++p;
}

void setupbounds(char *lpos)
{
	struct  databndaries *bdry,*prevbnd;
	register int bdtyp,lclass=0;
	int rglo,rghi;
	char c;
	struct ofsetree *otreept = 0;

	TypeGet=0;
	PBytSiz=1;	/* Default to single byte */

	/* First character should be boundary type */

	switch(c=toupper( *(lpos=skipblank(lpos)))) {
		case 'L':
			PBytSiz=2;
		case 'S':
			lpos=skipblank(++lpos);
			if(!(lclass=(int)strpos(lblorder,*lpos)))
				nerrexit("Illegal Label Class");
			break;
		case 'W':
			PBytSiz=2;
		case 'B':
			lclass=strpos(lblorder,'$');
			break;
		case 'A':
			lclass=strpos(lblorder,'^');
			break;
		default:
			nerrexit("Illegal boundary name");
	}

	bdtyp = (int)strpos(BoundsNames,c);
	/* Offset spec (if any) */
	lpos=skipblank(++lpos);
	if( *(lpos) == '(') {
		otreept = calloc (1, sizeof(struct ofsetree) );
		if( !otreept)
			nerrexit("Cannot allocate memory for offset!");
		lpos = setoffset(++lpos,otreept);
	}

	getrange(lpos,&rglo,&rghi,bdtyp);
		/* Now create the addition to the list */
	if( !(bdry = calloc (1, sizeof( struct databndaries)) ) ) {
		fprintf(stderr,"Cannot allocate memory for boundary\n");
		exit(1);
	}

	bdry->b_lo=rglo; bdry->b_hi=rghi; bdry->b_class=lclass;
	bdry->b_typ=bdtyp;bdry->DLeft=bdry->DRight=0;
	bdry->dofst=otreept; 
	
	if(NoEnd) {
		switch (NoEnd) {
			case 1:	/* NoEnd from this pass	*/
				++NoEnd; /* flag for next pass */
				break;
			default:
				prevbnd->b_hi = bdry->b_lo - 1;
				NoEnd = 0;	/* flag empty end fixed	*/
		}
	}
	prevbnd = bdry;	/* save this for open-ended bound */
	
	if( !dbounds ) {	/* First entry	*/
		bdry->dabove=0;
		dbounds = bdry;
	}
	else {
		bdinsert(bdry);
	}
}

void bndoverlap()
{
	fprintf(stderr,"Data segment overlap in line %d\n",LinNum);
	exit(1);
}

/* ***************************************************** *
 * bdinsert() - inserts an entry into the boundary table *
 * ***************************************************** */

void bdinsert(struct databndaries *bb)
{
	register struct databndaries *npt;
	register int mylo=bb->b_lo, myhi=bb->b_hi;
	
	npt = dbounds;	/*  Start at base	*/

	while(1) {
		if( myhi<npt->b_lo ) {
			if(npt->DLeft) {
				npt = npt->DLeft;
				continue;
			}
			else {
				bb->dabove=npt;
				npt->DLeft=bb;
				return;
				}
		}
		else {
			if(mylo>npt->b_hi) {
				if(npt->DRight) {
					npt = npt->DRight;
					continue;
				}
				else {
					bb->dabove=npt;
					npt->DRight=bb;
					return;
				}
			}
			else {
				bndoverlap();
			}
		}
	}
}

void getrange(char *pt,int *lo,int *hi,int bndtyp)
{
	char tmpdat[50], *dpt, c;
	
	dpt=tmpdat;	/* just to be sure */
	
	/* see if it's just a single byte/word */
	if( !(isxdigit(*(pt=skipblank(pt)))) ) {
		if( (*pt=='-') || ( (*pt=='/') && !TypeGet) ) {
			if(NoEnd) {
				nerrexit("No start address/no end address in prev line %d"); 
			}
			*lo = NxtBnd;
		}
		else {	/* no range specified */
			if(*pt && (*pt != '\n'))	/* non-digit gargabe */
				nerrexit("Illegal Address specified");
			else	/* provide data to process */
				sprintf(pt,"%x",NxtBnd);
		}
	}
	
	pt=movxnum(tmpdat,pt);
	sscanf(tmpdat,"%x",lo);
	
	/* Scan for second number/range/etc */
	switch(c=*(pt=skipblank(pt))) {
		case '/':
			if(TypeGet == 1)
				nerrexit("Cannot specify \"/\" in this mode!");
			pt=movdigit(tmpdat,++pt);
			NxtBnd = *lo+atoi(tmpdat);
			*hi = NxtBnd-1;
			break;
		case '-':
			switch( *(pt=skipblank(++pt))) {
				case ';':
				case '\0':
					NoEnd = 1; /* Flag need end address*/
					*hi = *lo;/* tmp value*/
					break;
				default:
					if(isxdigit(*pt)) {
						tellme(pt);
						pt=movxnum(tmpdat,pt);
						sscanf(tmpdat,"%x",hi);
						NxtBnd = *hi+1;
					}
			}
			break;
		default:
			if( !(*pt) ) {
				register int bsize = 1;
				
				if( (bndtyp == (int)strpos(BoundsNames,'L')) ||
							(bndtyp ==(int)strpos(BoundsNames,'W')) ) {
					bsize = 2;
				}
				NxtBnd = *lo+bsize;
				*hi = NxtBnd-1;
			}
	}
	if( *pt )
		nerrexit("Extra data..");

}
void tellme(char *pt)
{
	return;
}

char *movxnum(char *dst,char *src)
{
	while(isxdigit(*(src)))
		*(dst++) = tolower(*(src++));
	*dst = '\0';
	return src;
}

char *movdigit(char *dst,char *src)
{
	while(isdigit(*(src)))
		*(dst++) = *(src++);
	*dst = '\0';
	return src;
}

int endofcmd(char *pp)
{
	return(1 ? ((*pp=='\n') || (*pp==';') || (!(*pp))) : 0);
}
void nerrexit(char *l)
{
	fprintf(stderr,l);
	fprintf(stderr,"Line #%d\n",LinNum);
	exit(1);
}	
