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

/* ************************************************************************ *
 *                                                                          $
 * os9disasm - a project to disassemble Os9-coco modules into source code   $
 *             following the example of Dynamite+                           $
 *                                                                          $
 * ************************************************************************ $
 *                                                                          $
 *  Edition History:                                                        $
 *  #  Date       Comments                                              by  $
 *  -- ---------- -------------------------------------------------     --- $
 *  01 2003/01/31 First began project                                   dlb $
 ************************************************************************** $
 * File:  o9dis.c                                                           $
 * Purpose: mainline for program                                            $
 *                                                                          *
 *  $Id::                                                                   $
 * ************************************************************************ */

#define MAIN

#include "odis.h"

#ifndef HAVE_DIRNAME
char *dirname(char *path);
#endif

#ifdef HAVE_LIBGEN_H
#   include <libgen.h>
#endif

	/* ascii names for control characters */
static const char *CtrlCod[] =
{
	"nul", "soh", "stx", "etx",
	"eot", "enq", "ack", "bel",
	"bs", "ht", "lf", "vt",
	"ff", "cr", "so", "si",
	"dle", "dcl", "dc2", "dc3",
	"dc4", "nak", "syn", "etb",
	"can", "em", "sub", "esc",
	"fs", "gs", "rs", "us",
	"space"
};

static int doingcmds = 0;              /* Flag: set if doing cmd file */
static char *DefDir;
static char *myhome;            /* pointer to HOME environment name */
char rdbuf[500];

static void
usage ()
{
    printf ("\nos9disasm V %s\n", VERSION);
    printf ("    Disassembles 6809/6309 code, sending a formatted listing to stdout\n");
    printf ("\nSyntax: os9dis [opts] <module filename> [opts]\n");
    printf ("\n Options:\n");
    printf ("   PATHS\n");
    printf ("    -c[=]<command file>\n");
    printf ("    -d  -  deine path to defs files  (default=$HOME/coco/defs)\n");
    printf ("    -s[=]<label file> - up to %d allowed\n", MAX_LBFIL);
    printf ("    -o[=]<source (.asm) filename\n");
    printf ("\n   OUTPUT OPTIONS\n");
    printf ("    -ls -  short labels (6-char) default=8-char\n");
    printf ("    -ll[=]<length> - Specify label length\n");
    printf ("\n    -u  -  fold to uppercase\n");
    printf ("    -a    Specify all 8-bit indexes with \"<\"\n");
    printf ("    -g  -  Output listing in tabbed format suitable for g09dis to interpret\n");
    printf ("    -z  -  Print zero register ofset. (Default is \"no\"\n");
    printf ("\n   PAGE SPECS\n");
    printf ("    -pw[=]<page width>     default=80\n");
    printf ("    -pd[=]<page depth>     default=66    0=disable pagination\n");
    printf ("\n   TARGET CPU\n");
    printf ("\n    -x[=]<type> - Target OS\n");
    printf ("           C=Coco (default = OS9)\n");
    printf ("           -3  -  target CPU is 6309 (accept 6309 opcodes)\n");
    printf ("\n   SOURCE FILE TYPE\n");
    printf ("      -r  File is ROF\n");
    printf ("      -w  Module built with rma/rlink\n");
    return;
}

/* **************************************************** *
 * pass_eq() - move pointer past equal sign, if present *
 * **************************************************** */

static char *
pass_eq (char *pr)
{
    return (*pr == '=' ? ++pr : pr);
}

/* ************************************************************************ *
 * build_path() - Try to build a valid pathname for a filename provided.    *
 *                Several attempts are made.  First, if the raw name is     *
 *                accessible, this name is returned.                        *
 *                If not, successive attempts to substute for "~/", trying  *
 *                for the same dir  in which the cmdfile is located,        *
 *                and finally  DefDir - where the defs are located.         *
 *                On all these, a string is dup-ed, and this string is      *
 *                returned.                                                 *
 * Passed:    the filename                                                  *
 * Returned:  return ptr to full pathname string. 0 on failure.             *
 * ************************************************************************ */

static char *
build_path (char *fname)
{
    char *realname = NULL;

    /* If it's accessible in its raw state, then return that name */

    if ( ! (access (fname, R_OK)))    /* Try in current directory */
    {
        return strdup(fname);   /* So we can free() it */
    }

    /* Try for "~/" */

    if ((fname[0] == '~') && (fname[1] == '/'))
    {
        if ( ! (realname = malloc (strlen(myhome) + strlen(fname) + 1)))
        {
            fprintf (stderr, "Cannot allocate memory for \"%s\"\n", fname);
            return 0;
        }
        
        strcpy (realname, myhome);

        if ((realname[strlen(realname) - 1]) != '/')
        {
            strcat (realname, "/");
        }

        strcat (realname, &fname[1]);
        return realname;
    }

    /* Now try for the same directory as the cmdfile, if it's been set */
    if (cmdfilename)
    {
        char *cmdf = dirname(strdup (cmdfilename));

        if ( ! (realname = malloc (strlen(cmdf) + strlen(fname) + 2)))
        {
            fprintf (stderr, "Cannot allocate memory for %s\n", fname);
            return 0;
        }

        sprintf (realname, "%s/%s", cmdf, fname);

        if ( ! access (realname, R_OK))
        {
            return realname;
        }
        else
        {
            free (realname);
        }
    }

    if ( DefDir)
    {
        int lngth = 0;
        int istilde = 0;

        if ( ! strncmp (DefDir, "~/", 2))
        {
            lngth = strlen (myhome);
            istilde = 1;
        }

        lngth += (strlen (DefDir) + strlen (fname) + 2);

        if ( ! (realname = malloc (lngth)))
        {
            fprintf (stderr, "Cannot allocate memory for \"%s\"\n", fname);
            return 0;
        }

        if (istilde)
        {
            sprintf (realname, "%s%s", myhome, &(DefDir[1]));
        }
        else
        {
            strcpy (realname, DefDir);
        }

        if (realname[strlen(realname) - 1] != '/')
        {
            strcat (realname, "/");
        }

        strcat (realname, fname);

        if ( ! access (realname, R_OK))
        {
            return realname;
        }
        else
        {
            free (realname);
        }
    }

    fprintf (stderr, "Could not find a path to \"%s\" ... skipping\n", fname);

    return 0;
}

/* **************************************************************** *
 * do_opt() - Searches for match of char pointed to by c and does   *
 *      whatever setup processing is needed.                        *
 *      This is used for both command-line opts and  opts found in  *
 *      the command file.                                           *
 * **************************************************************** */

void
do_opt (char *c)
{
    char *pt = c;

    switch (tolower (*(pt++)))
    {
    case 'a':
        Show8bit = 1;
        break;
    case 'o':                  /* output asm src file */
        asmfile = pass_eq (pt);

        if ( ! (outpath = fopen (asmfile, "wb")))
        {
            nerrexit ("Cannot open output file\n");
        }
        
        WrtSrc = 1;
        break;
    case 'x':
        pt = pass_eq (pt);

        switch (toupper(*pt))
        {
            case 'C':
                OSType = OS_Coco;
                DfltLbls = CocoDflt;
                fprintf(stderr, "You are disassembling for coco\n");
                break;
            default:
                fprintf (stderr, "Error, undefined OS type: %s\n", pt);
                exit (1);
        }

        break;
    case 'r':           /* File is an ROF file */
        IsROF = 1;
        break;
    case 's':                  /* Label file name       */
        if (LblFilz < MAX_LBFIL)
        {
            pt = pass_eq (pt);

            if ((LblFNam[LblFilz] = build_path (pt)))
            {
                ++LblFilz;
            }
        }
        else
        {
            fprintf (stderr, "Max label files allotted -- Ignoring \'%s\'\n",
                     pass_eq (pt));
        }
        break;
    case 'c':                  /* Specify Command file */
        if (cmdfilename)
        {
            fprintf (stderr, "Command file already defined\n");
            fprintf (stderr, "Ignoring %s\n", pass_eq (pt));
        }
        else
        {
            cmdfilename = build_path (pass_eq (pt));
        }

        break;
    case 'u':                  /* Translate to upper-case */
        UpCase = 1;
        break;
    case 'g':                  /* tabs (g-print-capable) */
        tabinit ();
        break;
    case 'p':                  /* Page width/depth */
        switch (tolower (*(pt++)))
        {
        case 'w':
            PgWidth = atoi (pass_eq (pt));
            break;
        case 'd':
            PgDepth = atoi (pass_eq (pt));
            break;
        default:
            usage ();
            exit (1);
        }
        break;
    case 'l':
        switch (tolower (*(pt++)))
        {
        case 's':
            ShortLbl = 6;
            break;
        case 'l':
            NamLen = atoi(pass_eq (pt));

            /* Make sure NamLen doesn't exceed max length */

            if (NamLen > NLMAX)
            {
                NamLen = NLMAX;
            }

            adjpscmd ();        /* Adjust the printout formats */

            break;
        default:
            usage ();
            exit (1);
        }

        break;
    case 'w':
        CSrc = 1;
        break;
    case 'd':
        if ( ! doingcmds)
        {
            DefDir = pass_eq (pt);
        }
        else
        {
            pt = pass_eq (pt);

            if ( ! (DefDir = strdup (pt)))
            {
                fprintf (stderr, "Cannot allocate memory for Defs dirname\n");
                exit (1);
            }
        }

        break;
    case 'z':
        dozeros = 1;
        break;
    case '3':
        CpuTyp = M_03;
        break;
    default:                   /* Unknown Option */
        usage ();
        exit (1);
    }
}

/* ************************************************************************ *
 * pass1() - Do setup procedures.  Processes header, depending upon OSType. *
 *          For OS9 type (default), determines if it's a program module or  *
 *          ROF and then processes header.                                  *
 *          Process command dfile (if we have one).                         *
 * ************************************************************************ */

static void
pass1 ()
{

    Pass2 = 0;

    /* do os9hdr here so that header values will be known */

    /* OSType = OS_9;*/

    switch (OSType)
    {
        case OS_Coco:
            rsdoshdr ();
            break;
        default:   /* default is OS_9 */
            UseFCC = 1;

            M_ID = o9_fgetword (progpath);

            if (M_ID == 0x62cd)    /* First half or ROF sync bytes? */
            {
                /* If so, get second 16 bytes */

                M_ID = M_ID << 16 | o9_fgetword (progpath);

                if ( M_ID == 0x62cd2387)
                {
                    IsROF = 1;
                }
                else
                {
                    fprintf (stderr, "Not an ROF Header - starts with %08x\n",
                                     M_ID);
                    exit (1);
                }
            }
            else
            {
                if (M_ID != 0x87cd)
                {
                    fprintf (stderr, "Not a valid header - starts with %04x\n",
                                     M_ID);
                    exit (1);
                }

            }

            rewind (progpath);

            if (IsROF)
            {
                rofhdr ();
            }
            else
            {
                os9hdr ();
            }
    }

    if (cmdfilename)
    {
        doingcmds = 1;
        do_cmd_file ();
        doingcmds = 0;
    }
}

/* ******************************************************************** *
 * RdLblFile() - Reads a label file and stores label values into label  *
 *      tree if value (or address) is present in tree                   *
 *      Path to file has already been opened and is stored in "inpath"  *
 * ******************************************************************** */

static void
RdLblFile ()
{
    char labelname[30],
         clas,
         eq[10],
         strval[8],
        *lbegin;
    int address;
    struct nlist *nl;

    while ( ! feof (inpath))
    {
        fgets (rdbuf, sizeof (rdbuf), inpath);

        if ( ! (lbegin = skipblank (rdbuf)) || (*lbegin == '*'))
        {
            continue;
        }

        if (sscanf (rdbuf, "%s %s %s %c", labelname, eq, strval, &clas) == 4)
        {
            clas = toupper (clas);

            if ( ! strcasecmp (eq, "equ"))
            {
                /* Store address in proper place */

                if (strval[0] == '$')   /* if represented in hex */
                {
                    sscanf (&strval[1], "%x", &address);
                }
                else
                {
                    address = atoi (strval);
                }

                if ((OSType == OS_9) && (clas == 'D') && (address <= ModData))
                {
                    nl = FindLbl (ListRoot ('D'), address);
                }
                else
                {
                    nl = FindLbl (ListRoot (clas), address);
                }

                if (nl)
                {
                    strncpy (nl->sname, labelname, NamLen);
                    nl->sname[NamLen] = '\0';
                    nl->stdnam = 1;
                }
            }
        }
    }
}

/* ******************************************************** *
 * GetLabels() - Set up all label definitions               *
 *      Reads in all default label files and then reads     *
 *      any files specified by the '-s' option.             *
 * ******************************************************** */

static void
GetLabels ()                    /* Read the labelfiles */
{
    register int x;
    register struct nlist *nl;
    char  filename[500],
         *stdlbl = "dynalbl",
         *tmpnam;

    /* First, get built-in ascii definitions.  Later, if desired,
     * they can be redefined */

    for (x = 0; x < 33; x++)
    {
        if ((nl = FindLbl (ListRoot ('^'), x)))
        {
            strcpy (nl->sname, CtrlCod[x]);
        }
    }

    if ((nl = FindLbl (ListRoot ('^'), 0x7f)))
    {
        strcpy (nl->sname, "del");
    }

    /* Next read in the Standard systems names file */

    if ((OSType == OS_9) || (OSType == OS_Moto))
    {
        tmpnam = build_path ("sysnames");

        if ( ! (inpath = fopen (tmpnam, "rb")))
            fprintf (stderr, "Error in opening Sysnames file..%s\n",
                              filename);
        else
        {
            RdLblFile ();
            fclose (inpath);
        }
    }

    /* and now the standard label file */
    switch (OSType)
    {
    case OS_Flex:
        strcat (strcpy (rdbuf, myhome), "/coco/defs/flex9lbl");
        break;
    case OS_Moto:              /* none.. create a dummy */
        strcat (strcpy (rdbuf, myhome), "/defs/donthavenone");
        break;
    case OS_Coco:
        stdlbl = "cocolbl";
        break;
    default:                   /* our favorite -- OS9  */
        stdlbl = "dynalbl";
        break;
    }

    tmpnam = build_path (stdlbl);

    if ( ! (inpath = fopen (tmpnam, "rb")))
        fprintf (stderr, "Error in opening Sysnames file..%s\n", filename);
    else
    {
        RdLblFile ();
        fclose (inpath);
    }

    /* Now read in label files specified on the command line */

    for (x = 0; x < LblFilz; x++)
    {
        tmpnam = build_path (LblFNam[x]);

        if ((inpath = fopen (tmpnam, "rb")))
        {
            RdLblFile ();
            fclose (inpath);
        }
        else
        {
            fprintf (stderr, "ERROR! cannot open Label File %s for read\n",
                     tmpnam);
        }
    }
}

/* ************************************************************ *
 * InitDefaults() - Null out all SymLst entries at tree base    *
 * ************************************************************ */

static void
InitDefaults ()
{
    register int x;

    doingcmds = 0;

    if ( ! (myhome = getenv ("HOME")))
    {
        fprintf (stderr, "Cannot determine HOME directory.. aborting..\n");
        exit (1);
    }

    DefDir = "~/coco/defs/";    /* Default defs location */

    /* Initialize all SymLst tree bases to NULL */

    for (x = 1; x < sizeof (SymLst) / sizeof (SymLst[1]); x++)
    {
        SymLst[x] = 0;
    }

}

/* **************************** *
 *                              *
 * program main entry point     *
 *                              *
 * **************************** */

int
main (int argc, char **argv)
{
    register int count;         /* generic integer      */

    if (argc < 2)
    {
        usage ();
        exit (1);
    }

    InitDefaults ();            /* Set switch defaults  */

    DfltLbls = OS9Dflt;         /* Default is OS9 */

    /* Parse command line */

    for (count = 1; count < argc;)
    {
        char *pt;

        pt = argv[count];

        if (*pt == '-')
        {
            ++pt;
            do_opt (pt);
        }
        else
        {
            if ( ! modfile)
                modfile = pt;
            else
            {
                fprintf (stderr, "More than one input file specified\n");
                fprintf (stderr, "Ignoring \'%s\n\'", pt);
                fprintf (stderr, "\n");
            }
        }
        ++count;
    }

    /* todo later: (maybe)let it recurse more than one file */

    if ( ! modfile)
    {
        /* todo later: input filename from keyboard */

        fprintf (stderr, "No input file specified!\n");
        usage ();
        exit (1);
    }

    {
        struct stat statbf;

        if (stat (modfile, &statbf) == -1)
        {
            fprintf (stderr, "Can't stat file %s\n", modfile);
            exit (errno);
        }

        FileSize = (int) statbf.st_size;
    }

    if ( ! (progpath = fopen (modfile, "rb")))
    {
        fprintf (stderr, "Cannot open infile (%s) to read\n", modfile);
        exit (errno);
    }

    /* Do Pass 1 */

    pass1 ();
    progdis ();

    GetLabels ();       /* Read in Label files before doing second pass */
    Pass2 = 1;
    rewind (progpath);

   /* If we're doing RSDOS, reset header pointers to first header */

    if (OSType == OS_Coco)
    {
        rsdoshdr();
    }

    progdis ();

    exit (0);
}

