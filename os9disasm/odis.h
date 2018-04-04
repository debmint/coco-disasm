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

/* *********************************************************************** *
*       OS9 Cross Disassembler                                             $
*                                                                          $
* File: odis.h                                                             $
* Purpose: general header file for os9disasm                               $
*                                                                          $
* $Id::                                                                    $
***************************************************************************/

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef MAIN
#define GLOBAL extern
#else
#define GLOBAL
#endif

#define LC_HEX '$'
#define LC_DEC '&'
#define LC_PIK '@'
#define LC_ASCII '^'
#define DEFAULTCLASS strpos(lblorder,DfltLbls[AMode-1])


/* CPU TYPE DEFS */
#define M_09 0
#define M_03 1

/* OS types   */
#define OS_9 0
#define OS_Flex 1
#define OS_Moto 2
#define OS_Coco 3
#define MAX_LBFIL 16	/* Max Label file allowed	*/


#include "structures.h"  /* All structure definitions */

/* File and filename pointers */
GLOBAL FILE *progpath,  /* The program module (or file) we're disassembling */
            *inpath,    /* generic input stream */
	        *outpath;	/* .asm file if -o specified		*/
GLOBAL char *modfile;   /* Filename of module    */
GLOBAL char *asmfile;	/* output asm source file	*/
GLOBAL char *cmdfilename;
GLOBAL int LblFilz; 	/* Label file count		*/
GLOBAL char *LblFNam[MAX_LBFIL];    /* List of label files to process */
GLOBAL int NxtBnd;	    /* Next available data bound	*/
GLOBAL int FileSize;	/* overall size of program file */
GLOBAL char BoundsNames[]
#ifdef MAIN
= "ABCLSW"
#endif
;

	/* General options and settings for program disassembly */
GLOBAL char OSType;	    /* OS for module - OS9, Flex, CoCo, or Motorola */
GLOBAL int ShortLbl	    /* Flag (if set) to only allow 6-character labels */
#ifdef MAIN
= 8
#endif
;

GLOBAL int CpuTyp	        /* Target cpu (6809 or 6309 */
#ifdef MAIN
=M_09		/* Default to 6809 */
#endif
;

GLOBAL int Show8bit;        /* 1 = Always specify short index offsets "<" */
GLOBAL int dozeros;         /* 1 = print zero offsets */
GLOBAL int UseTabs;         /* Use Tabs in output */
GLOBAL int UseFCC;	        /* Flag (if set) to enable "fcc" statement
			   default for OSTYpe=OS_9 */
GLOBAL int UpCase
#ifdef MAIN
= 0			                /* default to off (lower case */
#endif
;
GLOBAL int IsROF;           /* ROF flag.  if true, we're doing a ROF */
GLOBAL char *DfltLbls;	    /* Ptr to default label class table	*/
GLOBAL char OS9Dflt[]	    /* Default addressing modes for diff. OS's */
#ifdef MAIN
= "@@@@@X&&D&DXL"
#endif
;GLOBAL char FlexDflt[]
#ifdef MAIN
= "@@LLLL&&&&DLL"
#endif
;GLOBAL char MotoDflt[]
#ifdef MAIN
= "@@@@@X&&D&DXL"
#endif
;GLOBAL char CocoDflt[]
#ifdef MAIN
= "@@LLLL&&&&DLL"
#endif
;


GLOBAL unsigned int ModLoad;    /* Load address for RS-Dos module */

	/* Module info from OS9 module header */
GLOBAL unsigned int ModSiz,
                ModNam,
                ModExe,
                ModTyp,     /* only one byte, but probably saves processing */
                ModData,
                ModRev,
                ModEdit,
                HdrLen;	    /* (probably useful for other OS's, too) */

/* The following are variables used by the disassembly process */

	/* EndAdr is a ptr to the end of the executable code in our buffer
	 * ... less 3-byte CRC value of OS9 */
/*GLOBAL int EndAdr;*/
GLOBAL char *ModBegin;	    /* Ptr to in-memory prog we're reading	*/
GLOBAL int Pass2;	        /* Flag as to which pass we're in	*/
GLOBAL int Pc;		        /* The "Program counter"..		*/
GLOBAL int AMode;	        /* Label Class for current command	*/
GLOBAL int CmdLen;	        /* # bytes in current cmd		*/
GLOBAL int CmdEnt;	        /* Entry point for current command	*/

	/* Modes,etc for current address being processed or set up	*/
GLOBAL int	NowAmode,
		NowClass,
		NowOfst,
		PBytSiz;	        /* # bytes in operand	*/

GLOBAL int CSrc;
GLOBAL unsigned int M_ID,
                    M_SIZE,
                    M_NAME,
                    M_TYPE,
                    M_REVS,
                    M_PARITY,
                    M_EXEC,
                    M_MEM;

		/* printer/output specific stuff */
GLOBAL int HavUnknown;
GLOBAL int LinNum;
GLOBAL int PgLin;
GLOBAL struct printbuf PBuf, Unknown;
GLOBAL int WrtSrc	        /* Flag to write source file - 0 = no */
#ifdef MAIN
= 0
#endif
;
GLOBAL int PgDepth
#ifdef MAIN
= 66
#endif
;
GLOBAL int PgWidth
#ifdef MAIN
= 80
#endif
;

GLOBAL int NamLen
#ifdef MAIN
= 8
#endif
;

GLOBAL char *lblorder
#ifdef MAIN
= "!^$&@%ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#endif
;

/* **************************************************** *
 * roots for the 31 symbol list trees (1 for each class *
 * **************************************************** */

GLOBAL struct nlist *SymLst[33]; /* [0] is empty - to avoid adjusting strpos */

/* roots for the 31 classes of data boundaries*/

GLOBAL struct databndaries *LAdds[33];

/* (the single) root to data boundaries tree */

GLOBAL struct databndaries *dbounds;

/* Comments tree */

GLOBAL struct commenttree *Comments[33];
GLOBAL struct apndcmnt *CmntApnd[33];

#include "proto.h"	/* Place last so all structs will be defined */
