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


extern char *CmdBuf;

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

void
PrintLine (char *pfmt, struct printbuf *pb)
{
    struct nlist *nl;

    NonBoundsLbl ();            /*Check for non-boundary labels */
    if (InProg
        && (nl = FindLbl (SymLst[(int) strpos (lblorder, 'L')], CmdEnt)))
    {
        strcpy (pb->lbnm, nl->sname);
    }

    if (UpCase)
    {
        /*UpString(pb->lbnm); */
        UpString (pb->instr);
        UpString (pb->opcod);
        UpString (pb->mnem);
        /*UpString(pb->operand); */
    }
    PrintFormatted (pfmt, pb);

    if (WrtSrc)
        fprintf (outpath, "%s %s %s\n", pb->lbnm, pb->mnem, pb->operand);

    /* Now straighten/clean up - prepare for next line  */
    PrevEnt = CmdEnt;
    memset (pb, 0, sizeof (struct printbuf));
    CmdLen = 0;
    ++PgLin;
    ++LinNum;
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

void
PrintFormatted (char *pfmt, struct printbuf *pb)
{
    if (!PgLin || PgLin > (PgDepth - 6))
        StartPage ();

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
        ("OS9 Cross Disassembler - Ver. 01.00.00    %02d/%02d/%02d %02d:%02d:%02d      Page %03d\n\n",
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
                sprintf (bf->operand, "*+%d", x - CmdEnt);
            else
                sprintf (bf->operand, "*-%d", CmdEnt - x);
            printf (pseudcmd, LinNum++, nl->myaddr, bf->instr,
                    bf->lbnm, bf->mnem, bf->operand);
            ++PgLin;
            if (WrtSrc)
                fprintf (outpath, "%s %s %s\n", bf->lbnm, bf->mnem,
                         bf->operand);
        }
    }
}


/* OS9Modline()  - print out OS9 mod line */

void
OS9Modline ()
{
    struct nlist *LL = SymLst[strpos (lblorder, 'L')];
    register struct modhead *mhd = (struct modhead *) ModBegin;
    struct printbuf PBf, *prtbf = &PBf;
    char hbf[10];

    hbf[0] = '\0';
    InProg = 0;
    memset (prtbf, 0, sizeof (struct printbuf));
    PBFcp (hbf, "%04x", ModBegin, 2);
    sscanf (hbf, "%04x", &CmdEnt);

    PBFcp (prtbf->instr, "%04x", &ModBegin[2], 2);

    /*sscanf(hbf,"%08x",&CmdEnt); */
    PrevEnt = CmdEnt + 1;       /* To prevent NonBoundsLbl() printouts */

    strcpy (prtbf->mnem, "mod");

    /* Now copy the operand line */
    sprintf (prtbf->operand, "%s,%s,$%02x,$%02x,%s",
             FindLbl (LL, ModSiz)->sname, FindLbl (LL, ModNam)->sname,
             (mhd->M_Type), (mhd->M_Revs), FindLbl (LL, ModExe)->sname);
    if (HdrLen == 13)
    {
        strcat (prtbf->operand, ",");
        /*strcat(prtbf->operand,FindLbl(SymLst[strpos(lblorder,'D')], */
        strcat (prtbf->operand, FindLbl (SymLst[0], ModData)->sname);
    }
    /*PrintLine("%5d  %08x %-10s%s %-10s %-6s %s\n",prtbf); */
    PrintLine (pseudcmd, prtbf);
    InProg = 1;
}

/* WrtEmod() - writes EMOD statement to OS9 file */
void
WrtEmod ()
{
    struct printbuf PBf, *prtbf = &PBf;
    register struct nlist *nl;

    CmdEnt = Pc;
    memset (prtbf, 0, sizeof (struct printbuf));
    PBFcp (prtbf->instr, "%04x", &ModBegin[Pc], 2);
    PBFcat (prtbf->instr, "%02x", &ModBegin[Pc + 2], 1);
    strcpy (prtbf->mnem, "emod");
    BlankLine ();
/*	PrintLine("%5d  %06x %-12s%s %-10s %-6s %s\n",prtbf);*/
    PrintLine (pseudcmd, prtbf);
    BlankLine ();
    memset (prtbf, 0, sizeof (struct printbuf));
    Pc += 3;
    CmdEnt = Pc;
    if ((nl = FindLbl (SymLst[strpos (lblorder, 'L')], Pc)))
    {
        strcpy (prtbf->lbnm, nl->sname);
        strcpy (prtbf->mnem, "equ");
        strcpy (prtbf->operand, "*");
        PrintLine (realcmd, prtbf);
    }
    BlankLine ();
    memset (prtbf, 0, sizeof (struct printbuf));
    strcpy (prtbf->mnem, "end");

    if (UpCase)
    {
        UpString (prtbf->mnem);
    }

    PrintFormatted (realcmd, prtbf);
    /*printf("%5d  %20s%s\n",LinNum,"","end"); */
    if (WrtSrc)
        fprintf (outpath, " %s\n", "end");
}

/* OS9DataPrint()	Mainline routine to list data defs
 */

void
OS9DataPrint ()
{
    struct nlist *dta, *srch;
    struct printbuf PB, *pbf = &PB;
    char *what = "* OS9 data area definitions";

    InProg = 0;                 /* Stop looking for Inline program labels to substitute */
    memset (pbf, 0, sizeof (struct printbuf));
    /*if( (dta=ListRoot('D')) ) { */
    if ((dta = SymLst[0]))
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
            strcpy (pbf->mnem, "rmb");
            sprintf (pbf->operand, "%d", srch->myaddr);
            CmdEnt = PrevEnt = 0;
            PrintLine (realcmd, pbf);
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
        datasize = (upadr) - (me->myaddr);
    if (me->myaddr != ModData)
        sprintf (pbf->operand, "%d", datasize);
    else
        strcpy (pbf->operand, ".");

    CmdEnt = me->myaddr;
    PrevEnt = CmdEnt;
    PrintLine (realcmd, pbf);

    if (me->RNext)
    {
        ListData (me->RNext, upadr);
    }
}

/* WrtEquates() - Print out label defs */

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
            TellLabels (me, flg);
        }
    }
    InProg = 1;
}

/* TellLabels(me) - Print out the labels for class in "me" tree */

void
TellLabels (struct nlist *me, int flg)
{
    struct printbuf PBF, *pb = &PBF;

    memset (pb, 0, sizeof (struct printbuf));
    if (me->LNext)
        TellLabels (me->LNext, flg);

    if ((flg < 0) || (flg == me->stdnam))
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
            UpString (pb->operand);
        CmdEnt = PrevEnt = me->myaddr;
        PrintLine (realcmd, pb);
    }

    if (me->RNext)
        TellLabels (me->RNext, flg);
}
