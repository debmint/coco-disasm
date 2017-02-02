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
 *                                                                          *
 *  os9disasm - OS9-6809 CROSS DISASSEMBLER                                 *
 *             following the example of Dynamite+                           *
 *                                                                          *
 * ************************************************************************ *
 *                                                                          *
 *  $Id$                             *
 *                                                                          *
 *  Edition History:                                                        *
 *  *  Date       Comments                                              by  *
 *  -- ---------- -------------------------------------------------     --- *
 *  01 2003/01/31 First began project                                   dlb *
 * ************************************************************************ *
 * File:  dprint.c                                                          *
 * Purpose: handle printing and output function                             *
 * ************************************************************************ */

#include "odis.h"
#include <time.h>

#define CNULL '\0'

static void PrintFormatted (char *pfmt, struct printbuf *pb);
static void NonBoundsLbl (char class);
static void PrintComment(char lblclass, int cmdlow, int cmdhi);
static void StartPage ();
static void TellLabels (struct nlist *me, int flg, char class, int minval);

extern char *CmdBuf;
extern struct printbuf *pbuf;
extern struct rof_hdr *rofptr;

char pseudcmd[80] = "%5d  %04X %-14s %-10s %-6s %-10s %s\n";
char realcmd[80] =  "%5d  %04X %-04s %-9s %-10s %-6s %-10s %s\n";
char *blankcmd = "%5d";

int PgNum = 0;
int PrevEnt = 0;                /* Previous CmdEnt - to print non-boundary labels */
int InProg;                     /* Flag that we're in main program, so that it won't
                                   munge the label name */
static char ClsHd[100];         /* header string for label equates */
static char FmtBuf[200];        /* Buffer to store formatted string */
int HadWrote;                   /* flag that header has been written */
char *SrcHd;                    /* ptr for header for source file */

/* *************************************************************** *
 * adjpscmd () - Adjusts pseudcmd/realcmd label length to          *
 *              NamLen + 2                                         *
 * *************************************************************** */

void
adjpscmd (void)
{
    sprintf (pseudcmd, "%s%d%s\n", "%5d  %04X %-14s %-",
                                   NamLen + 2,
                                   "s %-6s %-10s %s");
    sprintf (realcmd, "%s%d%s\n", "%5d  %04X %-04s %-9s %-",
                                  NamLen + 2,
                                  "s %-6s %-10s %s");
}

void
tabinit ()
{
    strcpy (realcmd, "%5d\t%04X\t%s\t%s\t%s\t%s\t%s\n");
    strcpy (pseudcmd, "%5d\t%04X\t%s\t\t%s\t%s\t%s\n");
}

/* ************************************** *
 * OutputLine () - does the actual output *
 * to the listing and/or source file      *
 * ************************************** */

static void
OutputLine (char *pfmt, struct printbuf *pb)
{
    struct nlist *nl;

    if (  InProg &&
         (nl = FindLbl (SymLst[(int) strpos (lblorder, 'L')], CmdEnt))  )
    {
        strcpy (pb->lbnm, nl->sname);

        if (IsROF && nl->global)
        {
            strcat (pb->lbnm, ":");
        }
    }

    PrintFormatted (pfmt, pb);

    if (WrtSrc)
    {
        fprintf (outpath, "%s %s %s", pb->lbnm,
                                           pb->mnem,
                                           pb->operand  );
        if (strlen (pb->comment))
        {
            fprintf (outpath, " %s", pb->comment);
        }

        fprintf (outpath, "\n");
    }
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

static void
BlankLine ()                    /* Prints a blank line */
{
    if ( ! PgLin || PgLin > (PgDepth - 6))
    {
        StartPage ();
    }

    printf ("%5d\n", LinNum++);
    ++PgLin;

    if (WrtSrc)
    {
        fprintf (outpath, "\n");
    }
}

/* ************************************************************ *
 * PrintNonCmd() - A utility function to print any non-command  *
 *          line (except stored comments).                      *
 *          Prints the line with line number, and updates       *
 *          PgLin & LinNum                                      *
 * Passed: str - the string to print                            *
 *         preblank - true if blankline before str              *
 *         postblank - true if blankline after str              *
 * ************************************************************ */

static void
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

/* ******************************************************** *
 * get_comment() - Checks for append comment for current    *
 *              command line.                               *
 * Passed: (1) class,                                       *
 *         (2) entry address for command                    *
 * Returns: ptr to comment string if present                *
 *          ptr to empty string if none                     *
 * ******************************************************** */

static char *
get_apcomment(char clas, int addr)
{
    struct apndcmnt *mytree = CmntApnd[strpos (lblorder, clas)];

    if ( ! clas)
    {
        return ("");
    }

    if (mytree)
    {
        while (1)
        {
            if (addr < mytree->adrs)
            {
                if (mytree->apLeft)
                {
                    mytree = mytree->apLeft;
                }
                else
                {
                    break;
                }
            }
            else
            {
                if (addr > mytree->adrs)
                {
                    if (mytree->apRight)
                    {
                        mytree = mytree->apRight;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    return mytree->CmPtr;
                }
            }
        }
    }

    return ("");
}

/* ******************************************************** *
 * PrintLine () - The generic, global printline function    *
 *                It checks for unlisted boundaries, prints *
 *                the line, and then does cleanup           *
 * ******************************************************** */

void
PrintLine (char *pfmt, struct printbuf *pb, char class, int cmdlow, int cmdhi)
{
    NonBoundsLbl (class);            /*Check for non-boundary labels */

    PrintComment (class, cmdlow, cmdhi);
    pb->comment = get_apcomment(class, cmdlow);
    OutputLine (pfmt, pb);

    PrintCleanup (pb);
}

static void
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
 * Passed: Pointer to the print buffer to UpCase   *
 * *********************************************** */

static void
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

static void
PrintFormatted (char *pfmt, struct printbuf *pb)
{
    int _linlen;

    if ( ! PgLin || PgLin > (PgDepth - 6))
        StartPage ();

    if (UpCase)
    {
        UpPbuf (pb);
    }

    /* make sure any non-initialized fields don't create Segfault */

    if ( ! pb->instr)       strcpy(pb->instr, "");
    if ( ! pb->lbnm)        strcpy(pb->lbnm, "");
    if ( ! pb->mnem)        strcpy(pb->mnem, "");
    if ( ! pb->mnem)        strcpy(pb->mnem, "");
    if ( ! pb->operand)     strcpy(pb->operand, "");
    if ( ! pb->comment)     pb->comment = "";

    if (pfmt == pseudcmd)
    {
        _linlen = snprintf (FmtBuf, PgWidth - 2, pfmt,
                                    LinNum, CmdEnt, pb->instr, pb->lbnm,
                                    pb->mnem, pb->operand, pb->comment);
    }
    else
    {
        _linlen = snprintf (FmtBuf, PgWidth - 2, pfmt,
                                LinNum, CmdEnt, pb->instr, pb->opcod, pb->lbnm,
                                pb->mnem, pb->operand, pb->comment);
    }

    if (_linlen >= PgWidth - 2)
    {
        FmtBuf[PgWidth - 4] = '\n';
        FmtBuf[PgWidth - 3] = '\0';
    }

    printf ("%s", FmtBuf);
    fflush (stdout);
}

static void
StartPage ()
{
    char *bywhom = "* Disassembly by Os9disasm of";
    time_t now;
    struct tm *tm;

    if (PgLin)
    {
        while (PgLin++ < PgDepth)
        {
            printf ("\n");
        }
    }

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
        {
            fprintf (outpath, "%s %s\n", bywhom, modfile);
        }

        BlankLine ();
    }
}

/* print any comments appropriate */

static void
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

static void
NonBoundsLbl (char class)
{
    if (class)
    {
        int x;

        for (x = PrevEnt + 1; x < CmdEnt; x++)
        {
            struct printbuf altbuf, *bf = &altbuf;
            struct nlist *nl;

            if ((nl = FindLbl (ListRoot (class), x)))
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
                        bf->lbnm, bf->mnem, bf->operand, "");
                ++PgLin;

                if (WrtSrc)
                {
                    fprintf (outpath, "%s %s %s\n", bf->lbnm, bf->mnem,
                             bf->operand);
                }
            }
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

    NonBoundsLbl ('L');
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
 * ROFPsect() - writes out psect                 *
 * Passed: rof_hdr *rptr                         *
 * ********************************************* */

#define OPSCAT(str) sprintf (pbuf->operand, "%s,%s", pbuf->operand, str)
#define OPDCAT(nu) sprintf (pbuf->operand, "%s,%d", pbuf->operand, nu)
#define OPHCAT(nu) sprintf (pbuf->operand, "%s,%04x", pbuf->operand, nu)

void
ROFPsect (struct rof_hdr *rptr)
{
    struct nlist *nl;

    strcpy (pbuf->instr, "");
    strcpy (pbuf->opcod, "");
    strcpy (pbuf->lbnm, "");
    strcpy (pbuf->mnem, "psect");
    sprintf (pbuf->operand, "%s,$%x,$%x,%d,%d", rptr->rname,
                                                rptr->ty_lan >> 8,
                                                rptr->ty_lan & 0xff,
                                                rptr->edition,
                                                rptr->stksz
            );

    if ((nl = FindLbl (ListRoot('L'), rptr->modent)))
    {
        OPSCAT(nl->sname);
    }
    else
    {
        OPHCAT ((int)(rptr->modent));
    }

    CmdEnt = 0;
    PrevEnt = 1;    /* To prevent NonBoundsLbl() output */
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
    prtbf->comment = "";

    if (UpCase)
    {
        UpPbuf (prtbf);
    }

    PrintFormatted (realcmd, prtbf);
    /*printf("%5d  %20s%s\n",LinNum,"","end"); */

    if (WrtSrc)
    {
        fprintf (outpath, " %s\n", "end");
    }
}

/* *************************************************** *
 * WrtEnds() - writes the "ends" command line          *
 * *************************************************** */

void
WrtEnds(void)
{
    struct printbuf bf;
    struct printbuf *pb = &bf;

    memset (pb, 0, sizeof (struct printbuf));
    strcpy (pb->mnem, "ends");

    BlankLine();

    PrintFormatted (realcmd, pb);

    if (WrtSrc)
    {
        fprintf (outpath, "%s %s %s", pb->lbnm,
                                           pb->mnem,
                                           pb->operand  );
        fprintf (outpath, "\n");
    }

    BlankLine();
}

/* *************************************************** *
 * ROFDataPrint() - Mainline routine to list data defs *
 *          for ROF's                                  *
 * *************************************************** */

void
ROFDataPrint ()
{
    struct nlist *dta, *srch;
    char *dptell[2] = {"* Uninitialized data (class %c)",
                       "* Initialized Data (class %c)"};
    int sizes[4] = { rofptr->udpsz, rofptr->idpsz,
                     rofptr->udatsz, rofptr->idatsz
                   },
        *thissz = sizes;

    int vs,
        isinit;
    int reftyp[] = {2, 3, 0, 1};     /* Label ref Type */
    char dattmp[5];
    char *dattyp = dattmp;
    char mytmp[50];

    InProg = 0;
    memset (pbuf, 0, sizeof (struct printbuf));

    /* We compute dattyp for flexibility.  If we change the label type
     * specification all we have to do is change it in rof_class() and it
     * should work here automatically rather than hard-coding the classes
     */

    dattyp[4] = '\0';

    for (vs = 0; vs < 4; vs++)
    {
        dattyp[vs] = rof_class (reftyp[vs]);
    }

    for ( vs = 0; vs <= 1; vs++)    /* Cycle through DP, non-dp */
    {
        if ((thissz[0]) || thissz[1])
        {
            strcpy (pbuf->mnem, "vsect");

            if (!vs)
            {
                strcpy (pbuf->operand, "dp");
            }
            else
            {
                strcpy (pbuf->operand, "");
            }

            BlankLine();
            PrintLine (realcmd, pbuf, dattyp[vs], 0, 0);
            BlankLine();

            /* Process each of un-init, init */

            for (isinit = 0; isinit <= 1; isinit++)
            {
                dta = ListRoot (dattyp[isinit]);

                sprintf (mytmp, dptell[isinit], dattyp[isinit]);

                if (isinit)
                {
                    if (thissz[isinit])
                    {
                        PrintNonCmd (mytmp, 0, 1);
                    }

                    ListInitROF (dta, thissz[isinit], vs, dattyp[isinit]);
                }
                else
                {
                    if (dta)
                    {
                        /* for PrintNonCmd(), send isinit so that a pre-blank
                         * line is not printed, since it is provided by
                         * PrinLine above */

                        if (thissz[isinit])
                        {
                            PrintNonCmd (mytmp, isinit, 1);
                        }

                        srch = dta;

                        while (srch->LNext)
                        {
                            srch = srch->LNext;
                        }

                        if (srch->myaddr)
                        {                       /* i.e., if not D000 */
                            strcpy (pbuf->mnem, "rmb");
                            sprintf (pbuf->operand, "%d", srch->myaddr);
                            CmdEnt = PrevEnt = 0;
                            PrintLine (realcmd, pbuf, dattyp[isinit], 0,
                                                      srch->myaddr);
                        }

                        /* For max value, send a large value so ListData
                         * will print all for class
                         */

                        ModData = thissz[isinit];
                        ListData (dta, thissz[isinit], dattyp[isinit]);
                    }           /* end "if (dta)" */
                    else        /* else no labels.. check to see */
                    {           /* if any "hidden" variables */
                        if (thissz[isinit])
                        {
                            PrintNonCmd (mytmp, isinit, 1);
                            strcpy (pbuf->mnem, "rmb");
                            sprintf (pbuf->operand, "%d", thissz[isinit]);
                            PrintLine (realcmd, pbuf, dattyp[isinit], 0,
                                                        0);
                        }
                    }
                }
            }

            /* Do "ends" for this vsect */

            WrtEnds();
        }

        dattyp += 2;   /* Done with this class, point to next */
        thissz += 2;   /* And to the next data size */
    }

    BlankLine ();
    InProg = 1;
}

/* *************************************************** *
 * OS9DataPrint()	Mainline routine to list data defs *
 * *************************************************** */

void
OS9DataPrint ()
{
    struct nlist *dta, *srch;
    char *what = "* OS9 data area definitions";

    InProg = 0;    /* Stop looking for Inline program labels to substitute */
    memset (pbuf, 0, sizeof (struct printbuf));

    if ((dta=ListRoot('D')))
    {                           /* special tree for OS9 data defs */
        BlankLine ();
        printf ("%5d %22s%s\n", LinNum++, "", what);
        ++PgLin;

        if (WrtSrc)
        {
            fprintf (outpath, "%s\n", what);
        }

        BlankLine ();

        /*first, if first entry is not D000, rmb bytes up to first */
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

        ListData (dta, ModData, 'D');
    }
    else
    {
        return;
    }

    BlankLine ();
    CmdEnt = PrevEnt = 0;
    InProg = 1;
}

/* ******************************************************** *
 * ListData() - recursive routine to print rmb's for Data   *
 *              definitions                                 *
 * Passed: pointer to current nlist element                 *
 *         address of upper (or calling) ListData() routine *
 *         Label Class                                      *
 * ******************************************************** */

void
ListData (struct nlist *me, int upadr, char class)
{
    struct printbuf PB, *pbf = &PB;
    register struct nlist *srch;
    register int datasize;

    memset (pbf, 0, sizeof (struct printbuf));

    /* Process lower entries first */

    if (me->LNext)
    {
        ListData (me->LNext, me->myaddr, class);
    }

    /* Don't print non-data elements here */

    if (me->myaddr > ModData)
    {
        return;
    }

    /* Now we've come back, print this entry */

    strcpy (pbf->lbnm, me->sname);

    if (IsROF && me->global)
    {
        strcat (pbf->lbnm, ":");
    }

    if (me->RNext)
    {
        srch = me->RNext;       /* Find smallest entry in that list */

        while (srch->LNext)
        {
            srch = srch->LNext;
        }

        datasize = (srch->myaddr) - (me->myaddr);
    }
    else
    {
        datasize = (upadr) - (me->myaddr);
    }

    /* Don't print any class 'D' variables which are not in Data area */
    /* Note, Don't think we'll get this far, we have a return up above,
     * but keep this one till we know it works

    if ((OSType == OS_9) && (me->myaddr > ModData))
    {
        return;
    }*/

    if (me->myaddr != ModData)
    {
        strcpy (pbf->mnem, "rmb");
        sprintf (pbf->operand, "%d", datasize);
    }
    else
    {
        if (IsROF)
        {
            strcpy (pbf->mnem, "rmb");
            sprintf (pbf->operand, "%d", datasize);
        }
        else
        {
            strcpy (pbf->mnem, "equ");
            strcpy (pbf->operand, ".");
        }
    }

    CmdEnt = me->myaddr;
    PrevEnt = CmdEnt;
    PrintLine (realcmd, pbf, class, me->myaddr, (me->myaddr + datasize));

    if (me->RNext && (me->myaddr < ModData))
    {
        ListData (me->RNext, upadr, class);
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

    if ( ! stdflg)                /* print ! and ^ only on std class pass */
    {
        curnt += 2;
    }

    while ((NowClass = *(curnt++)) != ';')
    {
        int minval;

        flg = stdflg;
        strcpy (ClsHd, "%5d %21s");

        if ((me = ListRoot (NowClass)))
        {
            /* For OS9, we only want external labels this pass */

            if ( (OSType == OS_9) && (NowClass == 'D'))
            {
                if (stdflg)     /* Don't print data defs */
                {
                    continue;
                }

                /* Probably an error if this happens
                 * What we're doing is positioning me to
                 * last real data element*/

               /* if (!(me = FindLbl (me, ModData)))
                {
                    continue;
                }*/
            }

            /* Don't write vsect data for ROF's */

/*            if ((IsROF) && stdflg && strchr ("BDGH", NowClass))
            {
                continue;
            }*/

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

            /* Determine minimum value for printing *
             * minval will be the first value to    *
             * print                                */

            minval = 0;     /* Default to "print all" */

            if (OSType == OS_9)
            {
                if (IsROF)
                {
                    minval = rof_datasize (NowClass);

                    /* If this class has any data, we want to exclude
                     * printing the last entry.
                     * Otherwise, if no real entries, we want to print
                     * element "0"
                     */

                    if (minval)
                    {
                        ++minval;
                    }
                }
                else
                {
                    if (NowClass == 'D')
                    {
                        minval = ModData + 1;
                    }
                    else {
                        if (NowClass == 'L')
                        {
                            minval = ModSiz + 1;
                        }
                    }
                }
            }

            TellLabels (me, flg, NowClass, minval);
        }
    }

    InProg = 1;
}

/* TellLabels(me) - Print out the labels for class in "me" tree */

static void
TellLabels (struct nlist *me, int flg, char class, int minval)
{
    struct printbuf PBF, *pb = &PBF;

    memset (pb, 0, sizeof (struct printbuf));

    if (me->LNext)
    {
        TellLabels (me->LNext, flg, class, minval);
    }

    if ((flg < 0) || (flg == me->stdnam))
    {
        /* Don't print real OS9 Data variables here */

        if (me->myaddr >= minval)
/*        if (!((OSType == OS_9) && (class == 'D') && (me->myaddr <= ModData)))*/
        {
            if ( ! HadWrote)
            {
                BlankLine ();
                printf (ClsHd, LinNum++, "", NowClass);
                ++PgLin;

                if (outpath)
                {
                    fprintf (outpath, SrcHd, NowClass);
                }

                HadWrote = 1;
                BlankLine ();
            }

            strcpy (pb->lbnm, me->sname);
            strcpy (pb->mnem, "equ");

            if (strchr ("!^", class))
            {
                sprintf (pb->operand, "$%02x", me->myaddr);
            }
            else
            {
                sprintf (pb->operand, "$%04x", me->myaddr);
            }

            CmdEnt = PrevEnt = me->myaddr;
            PrintLine (realcmd, pb, class, me->myaddr, me->myaddr + 1);
        }
    }

    if (me->RNext)
    {
        TellLabels (me->RNext, flg, class, minval);
    }
}
