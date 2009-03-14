/* ************************************************************************ *
 *                                                                          *
 * os9disasm - a project to disassemble Os9-coco modules into source code   *
 *             following the example of Dynamite+                           *
 *                                                                          *
 * ************************************************************************ *
 *                                                                          *
 *  $Id$                         *
 *                                                                          *
 *  Edition History:                                                        *
 *  #  Date       Comments                                              by  *
 *  -- ---------- -------------------------------------------------     --- *
 *  01 2003/01/31 First began project                                   dlb *
 ************************************************************************** *
 * File:  o9dis.c                                                           *
 * Purpose: mainline for program                                            *
 * ************************************************************************ */

#define MAIN

#include "odis.h"

int doingcmds = 0;              /* Flag: set if doing cmd file */
static char *DefDir;

static char *myhome;            /* pointer to HOME environment name */
char rdbuf[500];

static void
usage ()
{
    fprintf (stderr, "\nos9disasm V %s\n", VERSION);
    fprintf (stderr, "    Disassembles 6809/6309 code, sending a formatted listing to stdout\n");
    fprintf (stderr, "\nSyntax: os9dis [opts] <module filename> [opts]\n");
    fprintf (stderr, "\n Options:\n");
    fprintf (stderr, "   PATHS\n");
    fprintf (stderr, "    -c[=]<command file>\n");
    fprintf (stderr,
             "    -d  -  define path to defs files  (default=$HOME/coco/defs)\n");
    fprintf (stderr, "    -s[=]<label file> - up to %d allowed\n", MAX_LBFIL);
    fprintf (stderr, "    -o[=]<source (.asm) filename\n");
    fprintf (stderr, "\n   OUTPUT OPTIONS\n");
    fprintf (stderr, "    -ls -  short labels (6-char) default=8-char\n");
    fprintf (stderr, "    -ll[=]<length> - Specify label length\n");
    fprintf (stderr, "\n    -u  -  fold to uppercase\n");
    fprintf (stderr, "    -a    Specify all 8-bit indexes with \"<\"\n");
    fprintf (stderr, "                C=Coco (default = OS9)\n");
    fprintf (stderr, "    -g  -  Output listing in tabbed format suitable for g09dis to interpret\n");
    fprintf (stderr,
             "    -z  -  Print zero register offset. (Default is \"no\"\n");
    fprintf (stderr, "\n   PAGE SPECS\n");
    fprintf (stderr, "    -pw[=]<page width>     default=80\n");
    fprintf (stderr, "    -pd[=]<pagd depth>     default=66\n");
    fprintf (stderr, "\n   TARGET CPU\n");
    fprintf (stderr, "\n    -x[=]<type> - Target OS\n");
    fprintf (stderr, "    -3  -  target CPU is 6309 (accept 6309 opcodes)\n");
    return;
}

static char *
pass_eq (char *pr)
{
    return (*pr == '=' ? ++pr : pr);
}

/* **************************************************** *
 * build_path()  - locate a filename and build a full   *
 *                 pathlist.                            *
 * Passed:    the filename                              *
 * Returned:  return ptr to full pathname               *
 * Note:  fullname is only a temporary buffer,          *
 *        Unless fname is unaltered, the returned       *
 *        pointer is one to a newly created string      *
 * **************************************************** */

static char *
build_path (char *fname, char *fullname, int nsize)
{
    char *abslut[] = { "/", "./", "../", "" };
    char **pt;
    char *realname = NULL;

    pt = &abslut[0];

    /* is it an absolute pathame ? */

    while (**pt)
    {
        if ( ! strncmp (*pt, fname, strlen (*pt)))
        {
            realname = fname;
            break;
        }
        ++pt;
    }

    /* is it "~/xxx" ? */

    if ( ! strncmp (fname, "~/", 2))
    {
        strncpy (fullname, myhome, nsize);
        nsize -= strlen (fullname);
        strncat (fullname, &fname[1], nsize);
        //realname = strdup (fullname);
        realname = fullname;
    }

    if ( ! realname)
    {
        /* If we get here, fname is simply a basename.. */
        if ( ! (access (fname, R_OK)))    /* Try in current directory */
        {
            realname = fname;
        }
        else
        {              /* OK.. one last shot..  assume it's in Defdir */
            if ( ! strncmp (DefDir, "~/", 2))
            {
                int tmpsize = nsize;

                strncpy (fullname, myhome, nsize);
                tmpsize -= strlen (fullname);

                strncat (fullname, &DefDir[1], tmpsize);
                nsize -= strlen (fullname);
            }
            else
            {
                strncpy (fullname, DefDir, nsize);
                nsize -= strlen (fullname);
            }

            strncat (fullname, fname, nsize);
            realname = fullname;
        }
    }

    return realname;
}

/* ************************************** *
 * do_opt() - Searches for match of char  *
 *      pointed to by c and does whatever *
 *      setup processing is needed        *
 * ************************************** */

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
            nerrexit ("Cannot open output file\n");
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
            if ( ! doingcmds)
                LblFNam[LblFilz] = pass_eq (pt);
            else
            {
                /*LblFNam[LblFilz]=malloc(strlen(pt)+2);
                   pt=pass_eq(pt);
                   if( !LblFNam[LblFilz] ) {
                   fprintf(stderr,
                   "Cannot allocate memory for Label filename\n");
                   exit(1);
                   }
                   strcpy(LblFNam[LblFilz],pt); */
                pt = pass_eq (pt);

                if ( ! (LblFNam[LblFilz] = strdup (pt)))
                {
                    fprintf (stderr, "Cannot strdup() Label filename %s\n",
                                      pt);
                }
            }

            ++LblFilz;
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
            char tmpstr[500];

            cmdfilename = build_path (pass_eq (pt), tmpstr, sizeof (tmpstr));
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
    case 'd':
        if ( ! doingcmds)
        {
            DefDir = pass_eq (pt);
        }
        else
        {
            /*DefDir = malloc(strlen(pt)+2); */
            pt = pass_eq (pt);

            if ( ! (DefDir = strdup (pt)))
            {
                fprintf (stderr, "Cannot allocate memory for Defs dirname\n");
                exit (1);
            }
            /*strcpy(DefDir,pt); */
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

static void
pass1 ()
{

    Pass2 = 0;

    /* do os9hdr here so that header values will be known */

    /* OSType = OS_9;*/

    switch (OSType)
    {
        int firstword;

        case OS_Coco:
            rsdoshdr ();
            break;
        default:   /* default is OS_9 */
            UseFCC = 1;

            firstword = o9_fgetword (progpath);

            if (firstword == 0x62cd)
            {
                firstword = firstword << 16 | o9_fgetword (progpath);

                if ( firstword == 0x62cd2387)
                {
                    IsROF = 1;
                }
                else
                {
                    fprintf (stderr, "Not an ROF Header - starts with %08x\n",
                                     firstword);
                    exit (1);
                }
            }
            else
            {
                if (firstword != 0x87cd)
                {
                    fprintf (stderr, "Not a valid header - starts with %04x\n",
                                     firstword);
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

static void
RdLblFile ()
{
    char labelname[30], clas, eq[10], strval[8];
    int address;
    struct nlist *nl;
    char *lbegin;

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

/* ********************************************* *
 * GetLabels() - Set up label definitions        *
 *      Set up defaults and read all label files *
 * ********************************************* */

static void
GetLabels ()                    /* Read the labelfiles */
{
    register int x;
    register struct nlist *nl;
    char *stdlbl = "dynalbl";
    char filename[500];
    char *tmpnam;

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
        tmpnam = build_path ("sysnames", filename, sizeof (filename));

        if ( ! (inpath = fopen (tmpnam, "rb")))
            fprintf (stderr, "Error in opening Sysnames file..%s\n",
                              filename);
        else
        {
            RdLblFile ();
            fclose (inpath);
        }

        //free (tmpnam);
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

    tmpnam = build_path (stdlbl, filename, sizeof (filename));

    if ( ! (inpath = fopen (tmpnam, "rb")))
        fprintf (stderr, "Error in opening Sysnames file..%s\n", filename);
    else
    {
        RdLblFile ();
        fclose (inpath);
    }

    //free (tmpnam);

    /* Now read in label files specified on the command line */

    for (x = 0; x < LblFilz; x++)
    {
        tmpnam = build_path (LblFNam[x], filename, sizeof (filename));

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

        //free (tmpnam);
    }
}

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

    for (x = 1; x <= sizeof (SymLst) / sizeof (SymLst[1]); x++)
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
            if (!modfile)
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

    pass1 ();

/*    if (IsROF)
    {
        rofhdr ();
    }
    else
    {*/
        progdis ();
/*    }*/

    GetLabels ();               /* Read in Label files */
    Pass2 = 1;
    rewind (progpath);

    if (OSType == OS_Coco)
    {
        rsdoshdr();   /* Reset header pointers to first header */
    }

    progdis ();

    exit (0);
}
