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
# File:  o9dis.c                                                             #
# Purpose: mainline for program                                              #
############################################################################*/

#define MAIN

#include "odis.h"

int doingcmds=0; /* Flag: set if doing cmd file */
char *DefDir;

static char *myhome;	/* pointer to HOME environment name */
char	rdbuf[500];

void usage()
{
	fprintf(stderr,"Syntax: os9dis [opts] <module filename> [opts]\n");
	fprintf(stderr," Options:\n");
	fprintf(stderr,"	-c[=]<command file>\n");
	fprintf(stderr,"	-s[=]<label file> - up to %d allowed\n",
			MAX_LBFIL);
	fprintf(stderr,"	-o[=]<source (.asm) filename\n");
	fprintf(stderr,"	-ls -  short labels (6-char) default=8-char\n");
	fprintf(stderr,"\n	-u  -  fold to uppercase\n");
	fprintf(stderr,"	-pw[=]<page width>     default=80\n");
	fprintf(stderr,"	-pd[=]<pagd depth>     default=66\n");
	fprintf(stderr,"	-d  -  define path to defs files  (default=$HOME/coco/defs)\n");
	fprintf(stderr,"	-3  -  target CPU is 6309 (accept 6309 opcodes)\n");
	return;
}

int main(int argc,char **argv)
{
	register int count;	/* generic integer	*/

	if( argc<2 ) {
		usage();
		exit(1);
	}

	InitDefaults();		/* Set switch defaults	*/

	DfltLbls = OS9Dflt;	/* Default is OS9 */

	/* Parse command line */
	
	for(count=1;count<argc;  ) {
		char *pt;

		pt = argv[count];

		if( *pt == '-') {
			++pt;
			do_opt(pt);
		}
		else {
			if( !modfile )
				modfile=pt;
			else {
				fprintf(stderr,"More than one input file specified\n");
				fprintf(stderr,"Ignoring \'%s\n\'",pt);
				fprintf(stderr,"\n");
			}
		}
		++count;
	}
	
		/* todo later: (maybe)let it recurse more than one file */ 
	if( !modfile ) {
		/* todo later: input filename from keyboard */
		fprintf(stderr,"No input file specified!\n");
		usage();
		exit(1);
	}

	{
		struct stat statbf;

		if( stat(modfile,&statbf) == -1 ) {
			fprintf(stderr,"Can't stat file %s\n",modfile);
			exit(errno);
		}

		FileSize = (int)statbf.st_size;
	}
	
	if( !(inpath = fopen(modfile,"rb")) ) {
		fprintf(stderr,"Cannot open infile (%s) to read\n",modfile);
		exit(errno);
	}
	
	/* We'll load the whole module into memory rather than
	 * trying to read from the file
	 */
	
	if( !(ModBegin=malloc(FileSize+100)) ) {
		fprintf(stderr,"Error!!  Cannot malloc memory for infile!\n");
		exit(errno);
	}
	if(fread(ModBegin,FileSize,1,inpath) < 1) {
		fprintf(stderr,"Error!! Didn't read all of file %s\n",
				modfile);
		exit(errno);
	}
	fclose(inpath);	/* Don't need to read file anymore	*/

	pass1();
	progdis();
	GetLabels();	/* Read in Label files */
	Pass2=1;
	progdis();
	exit(0);
}

void do_opt(char *c)
{
	char *pt = c;
	
	switch(tolower(*(pt++))) {
		case 'o':	/* output asm src	*/
			asmfile=pass_eq(pt);
			if( !(outpath = fopen(asmfile,"wb")) )
				nerrexit("Cannot open output file\n");
			WrtSrc=1;
			break;
		case 's':	/* Label filename	*/
			if(LblFilz < MAX_LBFIL) {
				if(!doingcmds)
					LblFNam[LblFilz] = pass_eq(pt);
				else {
					LblFNam[LblFilz]=malloc(strlen(pt)+2);
					pt=pass_eq(pt);
					if( !LblFNam[LblFilz] ) {
						fprintf(stderr,
                                "Cannot allocate memory for Label filename\n");
						exit(1);
					}
					strcpy(LblFNam[LblFilz],pt);
				}
				++LblFilz;
			}
			else {
				fprintf(stderr,"Max label files allotted -- Ignoring \'%s\'\n",
								pass_eq(pt));
			}
			break;
		case 'c':	/* Specify Command file	*/
			if(cmdfilename) {
				fprintf(stderr,"Command file already defined\n");
				fprintf(stderr, "Ignoring %s\n",pass_eq(pt));
			}
			else {
				cmdfilename = pass_eq(pt);
			}
			break;
		case 'u':	/* Translate to upper-case */
				UpCase = 1;
				break;
        case 'g':  /* tabs (g-print-capable) */
                tabinit();
                break;
		case 'p':   /* Page width/depth */
			switch(tolower(*(pt++))) {
				case 'w':
					PgWidth=atoi(pass_eq(pt));
                    break;
				case 'd':
					PgDepth=atoi(pass_eq(pt));
                    break;
				default:
					usage();
					exit(1);
			}
			break;
		case 'l':
			switch(tolower(*(pt++))) {
				case 's':
					ShortLbl = 6;
					break;
				default:
					usage();
					exit(1);
			}
		case 'd':
			if( ! doingcmds )
			{
				DefDir = pass_eq(pt);
			}
			else {
				DefDir = malloc(strlen(pt)+2);
				pt=pass_eq(pt);
				if( !DefDir )
				{
					fprintf(stderr,"Cannot allocate memory for Defs Dir\n");
					exit(1);
				}
				strcpy(DefDir,pt);
			}
			break;
		case '3':
			CpuTyp = M_03;
			break;
		default:	/* Unknown Option */
			usage();
			exit(1);
	}
}

char *pass_eq(char *pr)
{
	return (*pr == '=' ? ++pr : pr);
}

void pass1()
{
	
	Pass2 = 0;

	/* do os9hdr here so that header values will be known */

	OSType = OS_9;  /*DEBUG*/
	UseFCC = 1;     /*DEBUG*/ /*Temporary fix */

	if( OSType == OS_9 )
		os9hdr();
				
	if(cmdfilename) {
		doingcmds=1;
		do_cmd_file();
		doingcmds=0;
	}
}

/* mkpath() : concatenate parts of a pathname */

void mkpath(char *dest,char *defpth,char *basename)
{
	dest[0] = '\0';
	
	if( defpth[0] == '~' )
	{
		strcpy(dest,myhome);
		strcat(dest,&defpth[1]);
	}
	else {
		strcpy(dest,defpth);
	}

	/* changed -- rindex() not in mingw??? */
/*	if( (rindex(dest,'/'))+1 != rindex(dest,'\0') )*/
	if( dest[strlen(dest)-1] != '/')
	{
		strcat(dest,"/");
	}

	strcat(dest,basename);
}
	

void GetLabels() /* Read the labelfiles */
{
	register int x;
	register struct nlist *nl;
	
	/* First, get built-in ascii definitions.  Later, if desired,
	 * they can be redefined */
	for(x=0;x<33;x++) {
		if( (nl=FindLbl(ListRoot('^'),x)) ) {
			strcpy(nl->sname,CtrlCod[x]);
			if(UpCase)
				UpString(nl->sname);
		}
	}
	if( (nl=FindLbl(ListRoot('^'),0x7f)) ) {
		strcpy(nl->sname,"del");
		if(UpCase)
			UpString(nl->sname);
	}
	
	/* Next read in the Standard systems names file */
	if( (OSType == OS_9) || (OSType == OS_Moto) ) {
		mkpath(rdbuf,DefDir,"sysnames");
		if( !(inpath=fopen(rdbuf,"rb")) )
			fprintf(stderr,"Error in opening Sysnames file..%s\n",
					rdbuf);
		else {
			RdLblFile();
			fclose(inpath);
		}
	}
	/* and now the standard label file */
	switch (OSType) {
		case OS_Flex:
			strcat(strcpy(rdbuf,myhome),"/coco/defs/flex9lbl");
			break;
		case OS_Moto:  /* none.. create a dummy */
			strcat(strcpy(rdbuf,myhome),"/defs/donthavenone");
			break;
		case OS_Coco:
			mkpath(rdbuf,DefDir,"cocolbl");
			break;
		default:	/* our favorite -- OS9  */
			mkpath(rdbuf,DefDir,"dynalbl");
			break;
	}
	if( !(inpath=fopen(rdbuf,"rb")) )
		fprintf(stderr,"Error in opening Sysnames file..%s\n",rdbuf);
	else {
		RdLblFile();
		fclose(inpath);
	}

		/* Now read in label files specified on the command line */
	for( x=0; x < LblFilz; x++ ) {
		if( !(inpath=fopen(LblFNam[x],"rb")) )
			fprintf(stderr,"Cannot open Label File: %s\n",
					LblFNam[x]);
		else {
			RdLblFile();
			fclose(inpath);
		}
	}
}

void RdLblFile()
{
	char labelname[30],
	     clas,
	     eq[10],
	     strval[8];
	int address;
	struct nlist *nl;
	char *lbegin;

	while( !feof(inpath) ) {
		fgets(rdbuf, sizeof(rdbuf), inpath);
		if(!(lbegin=skipblank(rdbuf)) || (*lbegin=='*'))
			continue;
		if(sscanf(rdbuf,"%s %s %s %c",labelname,eq,strval,
					&clas) == 4 ) {
			clas=toupper(clas);
			
			if( !strcasecmp(eq,"equ") ) {
				if(strval[0] == '$') {
					sscanf(&strval[1],"%x",&address);
				}
				else
					address=atoi(strval);
				if((OSType==OS_9)&& (clas=='D') &&
						(address<=ModData))
					nl=FindLbl(SymLst[0],address);
				else
					nl=FindLbl(ListRoot(clas),address);
				
				if( nl ) {
					strncpy(nl->sname,labelname,ShortLbl);
					nl->sname[ShortLbl]='\0';
					nl->stdnam = 1;
					if(UpCase)
						UpString(nl->sname);
				}
			}
		}
	}
}

void InitDefaults()
{
	register int x;
	
	doingcmds=0;

	if( !(myhome=getenv("HOME")) ) {
		fprintf(stderr,"Cannot determine HOME directory.. aborting..\n");
		exit(1);
	}
	
	DefDir = "~/coco/defs/";  /* Default defs location */
	
	for( x=0; x <= sizeof(SymLst)/sizeof(SymLst[0]); x++ ) {
		SymLst[x] = 0;
	}

}

