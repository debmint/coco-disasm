/*############################################################################
#
#  os9disasm - OS9-6809 CROSS DISASSEMBLER 
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
# File:  dprint.c                                                            #
# Purpose: handle printing and output function                               #
############################################################################*/

#include "odis.h"
#include <time.h>

#define CNULL '\0'

extern char *CmdBuf;
extern struct printbuf *pbuf;
extern struct rof_hdr *rofptr;

char *pseudcmd = "%5d  %04X %-14s %-10s %-6s %s\n";
char *realcmd = "%5d  %04X %-04s %-9s %-10s %-6s %s\n";
char *blankcmd = "%5d";

int PgNum = 0;
int PrevEnt = 0;                /* Previous CmdEnt - to print non-boundary labels */
int InProg;                     /* Flag that we're in main program, so that it won't
                                   munge the label name */
static char ClsHd[100];         /* header string for label equates */
int HadWrote;                   /* flag that header has been written */
char *SrcHd;                    /* ptr for header for source file */

void
tabinit ()
{
    realcmd = "%5d\t%04X\t%s\t%s\t%s\t%s\t%s\n";
    pseudcmd = "%5d\t%04X\t%s\t\t%s\t%s\t%s\n";
}

/* ************************************** *
 * OutputLine () - does the actual output *
 * to the listing and/or source file      *
 * ************************************** */

static void
OutputLine (char *pfmt, struct printbuf *pb)
{
    struct nlist *nl;

    if (InProg
        && (nl = FindLbl (SymLst[(int) strpos (lblorder, 'L')], CmdEnt)))
    {
        strcpy (pb->lbnm, nl->sname);
    }

    PrintFormatted (pfmt, pb);

    if (WrtSrc)
        fprintf (outpath, "%s %s %s\n", pb->lbnm, pb->mnem, pb->operand);
}

    /* Straighten/clean up - prepare for next line  */

static void
PrintCleanup (struct printbuf *pb)
{
    PrevEnt = CmdEnt;

    if (pb)
    {
        memset (pb, 0, sizeof (struct printbuf));
    }

    CmdLen = 0;
    ++PgLin;
    ++LinNum;
}

/* *********************************************** *
 * PrintNonCmd() - A utility function to print any *
 *          non-command line (except stored        *
 *          comments).  Prints the line with line  *
 *          number, and updates PgLin, LinNum      *
 * Passed: str - the string to print               *
 *         preblank - true if blankline before str *
 *         postblank - true if blankline after str *
 * *********************************************** */

void
PrintNonCmd (char *str, int preblank, int postblank)
{
    if (IsROF)
    {
        if (preblank)
        {
            BlankLine();
        }

        printf ("%5d %s\n", LinNum, str);
        
        if (WrtSrc)
        {
            fprintf (outpath, "%s", str);
        }

        if (postblank)
        {
            BlankLine();
        }
    }
    
    PrintCleanup (0);
}

/* *********************************************** *
 * PrintLine () - The generic, global printline    *
 *    function.  It checks for unlisted boundaries *
 *    prints the line, and then does cleanup       *
 * *********************************************** */

void
PrintLine (char *pfmt, struct printbuf *pb, char class, int cmdlow, int cmdhi)
{
    NonBoundsLbl ();            /*Check for non-boundary labels */

    PrintComment (class, cmdlow, cmdhi);
    OutputLine (pfmt, pb);

    PrintCleanup (pb);
}

void
UpString (char *s)              /* Translate a string to uppercase */
{
    register int x = strlen (s);

    while (x--)
    {
        if (isalpha (*s))
            *s = toupper (*s);
        ++s;
    }
}

/* *********************************************** *
 * UpPbuf() - Translates a whole print buffer's    *
 *            contents to upper case, if UpCase    *
 * Passed: Pointer to the print buffer to upcase   *
 * *********************************************** */

void
UpPbuf (struct printbuf *pb)
{
    if (UpCase)
    {
        UpString (pb->instr);
        UpString (pb->opcod);
        UpString(pb->lbnm);
        UpString (pb->mnem);
        UpString(pb->operand);
    }
}

void
PrintFormatted (char *pfmt, struct printbuf *pb)
{
    if (!PgLin || PgLin > (PgDepth - 6))
        StartPage ();

    if (UpCase)
    {
        UpPbuf (pb);
    }
    
    if (pfmt == pseudcmd)
    {
        printf (pfmt, LinNum, CmdEnt, pb->instr, pb->lbnm,
                pb->mnem, pb->operand);
    }
    else
    {
        printf (pfmt, LinNum, CmdEnt, pb->instr, pb->opcod, pb->lbnm,
                pb->mnem, pb->operand);
    }
}

void
StartPage ()
{
    char *bywhom = "* Disassembly by Os9disasm of";
    time_t now;
    struct tm *tm;

    if (PgLin)
        while (PgLin++ < PgDepth)
            printf ("\n");

    ++PgNum;
    PgLin = 0;
    /*.. */

    now = time (0);
    tm = localtime (&now);

    printf
        ("OS9 Cross Disassembler - Ver. %s    %02d/%02d/%02d %02d:%02d:%02d      Page %03d\n\n", VERSION,
         tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900, tm->tm_hour,
         tm->tm_min, tm->tm_sec, PgNum);
    PgLin = 2;

    if (PgNum == 1)
    {                           /* print disassembler info on first page */
        LinNum = 1;
        printf ("%5d%20s %s %s\n", LinNum++, "", bywhom, modfile);
        if (WrtSrc)
            fprintf (outpath, "%s %s\n", bywhom, modfile);
        BlankLine ();
    }
}

void
BlankLine ()                    /* Prints a blank line */
{
    if (!PgLin || PgLin > (PgDepth - 6))
        StartPage ();

    printf ("%5d\n", LinNum++);
    ++PgLin;
    if (WrtSrc)
        fprintf (outpath, "\n");
}

/* print any comments appropriate */
void
PrintComment(char lblclass, int cmdlow, int cmdhi)
{
    register struct commenttree *me;
    register int x;

    for (x = cmdlow; x < cmdhi; x++)
    {
        me = Comments[strpos (lblorder, lblclass)];

        while (me)
        {
            if (x < me->adrs)
            {
                me = me->cmtLeft;
            }
            else
            {
                if (x > me->adrs)
                {
                    me = me->cmtRight;
                }
                else        /* Assume for now it's equal */
                {
                    struct cmntline *line;
    
                    line = me->commts;
    
                    do {
                        printf("%5d       * %s\n", LinNum++, line->ctxt);
                        
                        if (WrtSrc)
                        {
                            fprintf (outpath, "* %s\n", line->ctxt);
                        }
                        
                    } while ((line = line->nextline));

                    break;  /* This address done, proceed with next x */
                }
            }
        }
    }
}

void
NonBoundsLbl ()
{
    int x;
    struct nlist *nl;
    struct printbuf altbuf, *bf = &altbuf;

    for (x = PrevEnt + 1; x < CmdEnt; x++)
    {
        if ((nl = FindLbl (SymLst[(int) strpos (lblorder, 'L')], x)))
        {
            memset (bf, 0, sizeof (struct printbuf));
            strcpy (bf->lbnm, nl->sname);
            strcpy (bf->mnem, "equ");
            if (x > CmdEnt)
            {
                sprintf (bf->operand, "*+%d", x - CmdEnt);
            }
            else
            {
                sprintf (bf->operand, "*-%d", CmdEnt - x);
            }
            printf (pseudcmd, LinNum++, nl->myaddr, bf->instr,
                    bf->lbnm, bf->mnem, bf->operand);
            ++PgLin;
            if (WrtSrc)
                fprintf (outpath, "%s %s %s\n", bf->lbnm, bf->mnem,
                         bf->operand);
        }
    }
}

/* ****************************** *
 * RsOrg() - print RSDos org line *
 * ****************************** */

void
RsOrg (void)
{
    struct printbuf PBf,  *prtbf = &PBf;

    memset (prtbf, 0, sizeof (struct printbuf));
    strcpy (prtbf->mnem, "org");
    sprintf (prtbf->operand, "$%04x", ModLoad);

    /* The following block prints out the line */
    BlankLine ();
    OutputLine (realcmd, prtbf);

    /* Call NonBoundsLbl() after OutputLine to try to get any unlisted
     * labels listed with the correct Pc offsets */

    NonBoundsLbl ();
    PrintCleanup (prtbf);
    BlankLine ();
    
    fseek (progpath, HdrLen, SEEK_SET);
}

/* *************************************** *
 * RsEnd() - Print end statement for RSDos *
 * *************************************** */

void
RsEnd (void)
{
    struct printbuf PBf,
                    *prtbf = &PBf;

    memset (prtbf, 0, sizeof (struct printbuf));
    strcpy (prtbf->mnem, "end");
    sprintf (prtbf->operand, "$%04x", ModExe);
    BlankLine ();
    PrintLine (realcmd, prtbf, CNULL, 0, 0);
    BlankLine ();
}

/* ********************************************* *
 * RIFPsect() - writes out psect                 *
 * Passed: rof_hdr *rptr                         *
 * ********************************************* */

#define OPSCAT(str) sprintf (pbuf->operand, "%s,%s", pbuf->operand, str)
#define OPDCAT(nu) sprintf (pbuf->operand, "%s,%d", pbuf->operand, nu)
#define OPHCAT(nu) sprintf (pbuf->operand, "%s,%04x", pbuf->operand, nu)

void
ROFPsect (struct rof_hdr *rptr)
{
    char psct[200];
    struct nlist *nl;

    strcpy (pbuf->instr, "");
    strcpy (pbuf->opcod, "");
    strcpy (pbuf->lbnm, "");
    strcpy (pbuf->mnem, "psect");
    sprintf (pbuf->operand, "%s,0,0,0,0", rptr->rname);

    if ((nl = FindLbl (ListRoot('L'), rptr->modent)))
    {
        OPSCAT(nl->sname);
    }
    else
    {
        OPHCAT ((int)(rptr->modent));
    }

    PrintLine (pseudcmd, pbuf, CNULL, 0, 0); 
}


/* ********************************************* *
 * OS9Modline()  - print out OS9 mod line        *
 * Note: We're going to assume that the          *
 *    file is positioned at the right            *
 *    place - we may try merged modules          *
 * ********************************************* */

void
OS9Modline ()
{
    struct nlist *LL = SymLst[strpos (lblorder, 'L')];
    struct printbuf PBf, *prtbf = &PBf;
    char hbf[10];
    long progstart = ftell (progpath);

    hbf[0] = '\0';
    InProg = 0;
    memset (prtbf, 0, sizeof (struct printbuf));
    CmdEnt = o9_fgetword (progpath);

    sprintf (prtbf->instr, "%04x", o9_fgetword (progpath));

    PrevEnt = CmdEnt + 1;       /* To prevent NonBoundsLbl() printouts */

    strcpy (prtbf->mnem, "mod");

    /* Now copy the operand line */
    sprintf (prtbf->operand, "%s,%s,$%02x,$%02x,%s",
             FindLbl (LL, ModSiz)->sname, FindLbl (LL, ModNam)->sname,
             ModTyp, ModRev, FindLbl (LL, ModExe)->sname);
    if (HdrLen == 13)
    {
        strcat (prtbf->operand, ",");
        strcat(prtbf->operand,FindLbl(SymLst[strpos(lblorder,'D')],
                    ModData)->sname);
        /*strcat (prtbf->operand, FindLbl (SymLst[0], ModData)->sname);*/
    }
    /*PrintLine("%5d  %08x %-10s%s %-10s %-6s %s\n",prtbf); */
    PrintLine (pseudcmd, prtbf, CNULL, 0, 0);
    InProg = 1;
    fseek (progpath, progstart+HdrLen, SEEK_SET);
}

/* **************************************** *
 * coco_wrt_end() - write the end statement *
 * to the listing                           *
 * **************************************** */

void
coco_wrt_end (void)
{
    /* Fill in later */
}

/* WrtEmod() - writes EMOD statement to OS9 file */
void
WrtEmod ()
{
    struct printbuf PBf, *prtbf = &PBf;
    register struct nlist *nl;
    int first;
    unsigned char last;

    CmdEnt = Pc;
    memset (prtbf, 0, sizeof (struct printbuf));
    first = o9_fgetword (progpath);
    last = fgetc (progpath);
    sprintf(prtbf->instr, "%04x%02x", first, last); 
    strcpy (prtbf->mnem, "emod");
    BlankLine ();
/*	PrintLine("%5d  %06x %-12s%s %-10s %-6s %s\n",prtbf);*/
    PrintLine (pseudcmd, prtbf, CNULL, 0, 0);
    BlankLine ();
    memset (prtbf, 0, sizeof (struct printbuf));
    Pc += 3;
    CmdEnt = Pc;

    if ((nl = FindLbl (SymLst[strpos (lblorder, 'L')], Pc)))
    {
        strcpy (prtbf->lbnm, nl->sname);
        strcpy (prtbf->mnem, "equ");
        strcpy (prtbf->operand, "*");
        PrintLine (realcmd, prtbf, CNULL, 0, 0);
    }

    BlankLine ();
    memset (prtbf, 0, sizeof (struct printbuf));
    strcpy (prtbf->mnem, "end");

    if (UpCase)
    {
        UpPbuf (prtbf);
    }

    PrintFormatted (realcmd, prtbf);
    /*printf("%5d  %20s%s\n",LinNum,"","end"); */
    if (WrtSrc)
        fprintf (outpath, " %s\n", "end");
}

/* *************************************************** *
 * ROFDataPrint() - Mainline routine to list data defs *
 *          for ROF's                                  *
 * *************************************************** */

void
ROFDataPrint ()
{
    struct nlist *dta, *srch;
    char *vstyp[2] = {"vsect dp", "vsect"};
    char *dptell[2] = {"* Uninitialized data (class %c)",
                       "* Initialized Data (class %c)"};
    int sizes[4] = { rofptr->udpsz, rofptr->idpsz,
                     rofptr->udatsz, rofptr->idatsz
                   },
        *thissz = sizes;

    int vs,
        isinit;
    char *dattyp = "DFCE";
    char mytmp[50];

    InProg = 0;
    memset (pbuf, 0, sizeof (struct printbuf));

    for ( vs = 0; vs <= 1; vs++)    /* Cycle through DP, non-dp */
    {
        if ((ListRoot (dattyp[0]) || ListRoot (dattyp[1])))
        {
            strcpy (pbuf->mnem, vstyp[vs]);
            BlankLine();
            PrintLine (realcmd, pbuf, *dattyp, 0, 0);
            BlankLine();

            /* Process each of un-init, init */

            for (isinit = 0; isinit <= 1; isinit++)
            {
                if (dta = ListRoot (*dattyp))
                {
                    sprintf (mytmp, dptell[isinit], *dattyp);

                    /* for PrintNonCmd(), send isinit so that a pre-blank line
                     * is not printed, since it is provided by PrinLine above
                     */

                    PrintNonCmd (mytmp, isinit, 1);
                    srch = dta;

                    while (srch->LNext)
                    {
                        srch = srch->LNext;
                    }

                    if ((srch->myaddr))
                    {                       /* i.e., if not D000 */
                        strcpy (pbuf->mnem, "rmb");
                        sprintf (pbuf->operand, "%d", srch->myaddr);
                        CmdEnt = PrevEnt = 0;
                        PrintLine (realcmd, pbuf, 'D', 0, srch->myaddr);
                    }

                    /* Note: We'll need to do something different for
                     * init data - but try to get it to work for now */

                    /* For max value, send a large value so ListData
                     * will print all for class
                     */

                    ListData (dta, *(thissz++));
                    ++dattyp;   /* Done with this class, point to next */
                }
            }

            /* Do "ends" for this vsect */

            strcpy (pbuf->mnem, "ends");
            BlankLine();
            PrintLine (realcmd, pbuf, 'D', 0, 0);
            BlankLine();
        }
    }
}

/* *************************************************** *
 * OS9DataPrint()	Mainline routine to list data defs *
 * *************************************************** */

void
OS9DataPrint ()
{
    struct nlist *dta, *srch;
/*    struct printbuf PB, *pbuf = &PB;*/
    char *what = "* OS9 data area definitions";

    InProg = 0;    /* Stop looking for Inline program labels to substitute */
    memset (pbuf, 0, sizeof (struct printbuf));

    if ((dta=ListRoot('D')))
    {                           /* special tree for OS9 data defs */
        BlankLine ();
        printf ("%5d %22s%s\n", LinNum++, "", what);
        ++PgLin;
        if (WrtSrc)
            fprintf (outpath, "%s\n", what);
        BlankLine ();

        /*first, if first entry is not D000, rmb bytes up to first */
        srch = dta;
        while (srch->LNext)
            srch = srch->LNext;

        if ((srch->myaddr))
        {                       /* i.e., if not D000 */
            strcpy (pbuf->mnem, "rmb");
            sprintf (pbuf->operand, "%d", srch->myaddr);
            CmdEnt = PrevEnt = 0;
            PrintLine (realcmd, pbuf, 'D', 0, srch->myaddr);
        }
        ListData (dta, ModData);
    }
    else
    {
        return;
    }

    BlankLine ();
    CmdEnt = PrevEnt = 0;
    InProg = 1;
}

/* ListData() - recursive routine to print rmb's for Data definitions
 *    Passed: address of upper (or calling) ListData() routine
 */

void
ListData (struct nlist *me, int upadr)
{
    struct printbuf PB, *pbf = &PB;
    register struct nlist *srch;
    register int datasize;

    memset (pbf, 0, sizeof (struct printbuf));

    /* Process preceding entries first */
    if (me->LNext)
    {
        ListData (me->LNext, me->myaddr);
    }

    /* Now we've come back, print this entry */
    strcpy (pbf->lbnm, me->sname);

    if (me->myaddr != ModData)
        strcpy (pbf->mnem, "rmb");
    else
        strcpy (pbf->mnem, "equ");

    if (me->RNext)
    {
        srch = me->RNext;       /* Find smallest entry in that list */

        while (srch->LNext)
            srch = srch->LNext;

        datasize = (srch->myaddr) - (me->myaddr);
    }
    else
    {
        datasize = (upadr) - (me->myaddr);
    }
    
    /* Don't print any class 'D' variables which are not in Data area */
    if ((OSType == OS_9) && (me->myaddr > ModData))
    {
        return;
    }
    
    if (me->myaddr != ModData)
        sprintf (pbf->operand, "%d", datasize);
    else
        strcpy (pbf->operand, ".");

    CmdEnt = me->myaddr;
    PrevEnt = CmdEnt;
    PrintLine (realcmd, pbf, 'D', me->myaddr, (me->myaddr + datasize));

    if (me->RNext)
    {
        ListData (me->RNext, upadr);
    }
}

/* ************************************************** *
 * WrtEquates() - Print out label defs                *
 * Passed: stdflg - 1 for std labels, 0 for externals *        
 * ************************************************** */

void
WrtEquates (int stdflg)
{
    char *claspt = "!^ABCDEFGHIJKMNOPQRSTUVWXYZ;",
        *curnt = claspt,
        *syshd = "* OS-9 system function equates\n",
        *aschd = "* ASCII control character equates\n",
        *genhd[2] = { "* class %c external label equates\n",
                      "* class %c standard named label equates\n"
                    };
    register int flg;           /* local working flg - clone of stdflg */
    struct nlist *me;

    InProg = 0;
    if (!stdflg)                /* print ! and ^ only on std class pass */
        curnt += 2;

    while ((NowClass = *(curnt++)) != ';')
    {
        flg = stdflg;
        strcpy (ClsHd, "%5d %21s");
        
        if ((me = ListRoot (NowClass)))
        {
            /* For OS9, we only want external labels this pass */
            
            if ( (OSType == OS_9) && (NowClass == 'D'))
            {
                if (stdflg)     /* Don't print data defs */
                {
                    return;
                }

                /* Probably an error if this happens
                 * What we're doing is positioning me to
                 * last real data element*/
                
                if (!(me = FindLbl (me, ModData)))
                {
                    return;
                }
            }

            /* Don't write vsect data for ROF's */

            if ((IsROF) && (NowClass >= 'C') && (NowClass <= 'D'))
            {
                return;
            }
            
            switch (NowClass)
            {
            case '!':
                strcat (ClsHd, syshd);
                SrcHd = syshd;
                flg = -1;
                break;
            case '^':
                strcat (ClsHd, aschd);
                SrcHd = aschd;
                flg = -1;
                break;
            default:
                strcat (ClsHd, genhd[flg]);
                SrcHd = genhd[flg];
            }

            HadWrote = 0;       /* flag header not written */
            TellLabels (me, flg, NowClass);
        }
    }

    InProg = 1;
}

/* TellLabels(me) - Print out the labels for class in "me" tree */

void
TellLabels (struct nlist *me, int flg, char class)
{
    struct printbuf PBF, *pb = &PBF;

    memset (pb, 0, sizeof (struct printbuf));
    if (me->LNext)
    {
        TellLabels (me->LNext, flg, class);
    }

    if ((flg < 0) || (flg == me->stdnam))
    {
        /* Don't print real OS9 Data variables here */
        if (!((OSType == OS_9) && (class == 'D') && (me->myaddr<=ModData)))
        {
            if (!HadWrote)
            {
                BlankLine ();
                printf (ClsHd, LinNum++, "", NowClass);
                ++PgLin;
                if (outpath)
                    fprintf (outpath, SrcHd, NowClass);
                HadWrote = 1;
                BlankLine ();
            }
    
            strcpy (pb->lbnm, me->sname);
            strcpy (pb->mnem, "equ");
            sprintf (pb->operand, "$%04x", me->myaddr);
            if (UpCase)
                UpPbuf (pb);
            CmdEnt = PrevEnt = me->myaddr;
            PrintLine (realcmd, pb, class, me->myaddr, me->myaddr + 1);
        }
    }

    if (me->RNext)
        TellLabels (me->RNext, flg,class);
}
