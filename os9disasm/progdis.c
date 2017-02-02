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
 *  os9disasm - OS9-6809 CROSS DISASSEMBLER                                 $
 *             following the example of Dynamite+                           $
 *                                                                          $
 * ************************************************************************ $
 *                                                                          $
 *  Edition History:                                                        $
 *  #  Date       Comments                                              by  $
 *  -- ---------- -------------------------------------------------     --- $
 *  01 2003/01/31 First began project                                   dlb $
 * ************************************************************************ $
 *    File:  progdis.c                                                      $
 * Purpose:  do disassembly of program file                                 $
 *                                                                          *
 * $Id::                                                                    $
 * ************************************************************************ */

#define _GNU_SOURCE
#include "odis.h"
#include "dtble.h"

/* Flag whether Databoundary or Addressing Mode */

#define DBOUND 1
#define ADRMOD 2
#define NOIDXINDIR if(c&0x20) return 0

char *RegReg[] =        /* 6809 registers */
    { "d", "x", "y", "u", "s", "pc", "", "", "a", "b", "cc", "dp" };

char *RegR03[] =        /* 6309 registers */
    { "d", "x", "y", "u", "s", "pc", "w", "v", "a", "b", "cc", "dp",
    "0", "", "e", "f"
};

/* Register order for push, pull for regS and reg U */

char *PshPuls[] = { "cc", "a", "b", "dp", "x", "y", "u", "pc" };
char *PshPulu[] = { "cc", "a", "b", "dp", "x", "y", "s", "pc" };
char RegOrdr[] = "xyus";

/*extern struct lkuptbl *Pre10, *Pre11, *Byte1;*/

extern char pseudcmd[], realcmd[];

char CmdBuf[10];                /* buffer to hold bytes of cmd code */

struct databndaries *curbnd;
struct databndaries *amodes[sizeof (lblorder)];
struct lkuptbl *tabl;
struct printbuf *pbuf = &PBuf;

extern struct rof_extrn *xtrn_ndp[],
                        *xtrn_dp[],
                        *xtrn_code;
extern struct rof_hdr *rofptr;

int CodEnd;                     /* End of executable code (ModSize-3 for OS9 */

/* ******************************************************************** *
 * MovBytes() - Reads data for Data Boundary range from input file and  *
 *          places it onto the print buffer (and does any applicable    *
 *          printing if in pass 2).                                     *
 * ******************************************************************** */

static void
MovBytes (struct databndaries *db)
{
    char tmps[20];
    int valu;

    CmdEnt = Pc;

    while (Pc <= db->b_hi)
    {
        /* Init dest buffer to null string for LblCalc concatenation */

        tmps[0] = '\0';
        valu = fgetc (progpath);

        if (PBytSiz == 2)
        {
            /* Read data in in LittleEndian format, save in Native */

            valu = (valu << 8) + fgetc (progpath);
        }

        LblCalc (tmps, valu, AMode);

        if (Pass2)
        {
            if (strlen (pbuf->instr) < 10)
            {
                char tmp[20];

                if (PBytSiz == 1)
                {
                    sprintf (tmp, "%02x ", valu);
                }
                else
                {
                    sprintf (tmp, "%04x ", valu);
                }

                strcat (pbuf->instr, tmp);
            }

            if (strlen (pbuf->operand))
            {
                strcat (pbuf->operand, ",");
            }

            strcat (pbuf->operand, tmps);

            /* If length of operand string is max, print a line */

            if (    (strlen (pbuf->operand) > 22)
                 || (FindLbl (ListRoot ('L'), Pc + PBytSiz))  )
            {
                strcpy (pbuf->mnem, PBytSiz == 1 ? "fcb" : "fdb");
                PrintLine (pseudcmd, pbuf, 'L', CmdEnt, Pc + PBytSiz);
                CmdEnt = Pc + PBytSiz;
            }
        }

        Pc += PBytSiz;
    }

    /* Loop finished.. print any unprinted data */

    if (Pass2 && strlen (pbuf->operand))
    {
        strcpy (pbuf->mnem, PBytSiz == 1 ? "fcb" : "fdb");
        PrintLine (pseudcmd, pbuf, 'L', CmdEnt, Pc);
    }
}

/* ********************************************************* *
 * AddDelims() - Add delimiters for fcc/fcs operand -        *
 *              checks string for nonexistand delimiter      *
 *              and copies string with delims to destination *
 * ********************************************************* */

static void
AddDelims (char *dest, char *src)
{
    char delim = '"';
    char bestdelims[] = "\"'/#\\|$!";
    char *dref = bestdelims;

    /* First, try to use some "preferred" delimiters */

    while (strchr (src, *dref))
    {
        ++dref;

        if (*dref == '\0')
        {
            break;
        }
    }

    delim = *dref;

    if (delim == '\0')
    {
        /* OK.. we didn't find a delim in the above..  Now let's
         * start off basically where we left off and parse through
         * the whole ASCII set to find one
         */

        delim = '\x25';

        while ( ! strchr(src, delim))
        {
            ++delim;

            /* This should never happen, but just in case */

            if (delim == '\x7f')
            {
                fprintf (stderr,
                         "Error, string contains ALL ASCII Characters???\n");
                exit(1);
            }
        }
    }

    sprintf (dest, "%c%s%c", delim, src, delim);
}

/* ************************************************** *
 * MovAsc() - Move nb byes int fcc (or fcs) statement *
 * ************************************************** */

void
MovASC (int nb, char class)
{
    char oper_tmp[30];

    memset (pbuf, 0, sizeof (struct printbuf));
    strcpy (pbuf->mnem, "fcc");         /* Default mnemonic to "fcc" */
    CmdEnt = Pc;

    *oper_tmp = '\0';

    while (nb--)
    {
        register int x;
        char c[6];

        x = fgetc (progpath);

        if ((isprint (x)) || ((x & 0x80) && UseFCC && isprint (x & 0x7f)))
        {
            if (Pass2)
            {
                if (strlen (pbuf->instr) < 12)
                {
                    sprintf (c, "%02x ", x);
                    strcat (pbuf->instr, c);
                }

                sprintf (c, "%c", x & 0x7f);
                strcat (oper_tmp, c);

                if ((x & 0x80))
                {
                    strcpy (pbuf->mnem, "fcs");
                    AddDelims (pbuf->operand, oper_tmp);
                    PrintLine (pseudcmd, pbuf, class, CmdEnt, Pc);
                    *oper_tmp = '\0';
                    CmdEnt = Pc + 1;
                    strcpy (pbuf->mnem, "fcc");
                }

                if ((strlen (oper_tmp) > 24) ||
                    (strlen (oper_tmp) && FindLbl (ListRoot (class), Pc + 1)))
                {
                    AddDelims (pbuf->operand, oper_tmp);
                    PrintLine (pseudcmd, pbuf, class, CmdEnt, Pc);
                    *oper_tmp = '\0';
                    CmdEnt = Pc + 1;
                    strcpy (pbuf->mnem, "fcc");
                }
            }   /* end if (Pass2) */
        }
        else            /* then it's a control character */
        {
            if (Pass2 && (strlen (oper_tmp)))
            {
                AddDelims (pbuf->operand, oper_tmp);
                PrintLine (pseudcmd, pbuf, class, CmdEnt, Pc);
                *oper_tmp = '\0';
                CmdEnt = Pc;
            }

            if ( ! Pass2)
            {
                if ((x & 0x7f) < 33)
                {
                    addlbl (x & 0x7f, '^');
                }
            }
            else
            {
                /* a dummy ptr to pass to Printlbl() to satify prototypes */
                struct nlist *nlp;

                /* do the following to keep gcc quiet about uninitialized
                 * variable.. If we're wrong, we'll get a segfault letting
                 * us know of our mistake.. */

                nlp = NULL;

                strcpy (pbuf->mnem, "fcb");
                PrintLbl (pbuf->operand, '^', x, nlp);
                sprintf (pbuf->instr, "%02x", x & 0xff);
                PrintLine (pseudcmd, pbuf, class, CmdEnt, Pc);
                strcpy (pbuf->mnem, "fcc");
            }
            CmdEnt = Pc + 1;
        }

        ++Pc;
    }       /* end while (nb--) - all chars moved */

    if (strlen (oper_tmp))        /* Clear out any pending string */
    {
        AddDelims (pbuf->operand, oper_tmp);
        PrintLine (pseudcmd, pbuf, class, CmdEnt, Pc);
        *oper_tmp = '\0';
    }
}

/* ********************************* *
 * NsertBnds():	Insert boundary area *
 * ********************************* */

void
NsrtBnds (struct databndaries *bp)
{
    memset (pbuf, 0, sizeof (struct printbuf));
    AMode = 0;                  /* To prevent LblCalc from defining class */
    NowClass = bp->b_class;
    PBytSiz = 1;                /* Default to one byte length */

    switch (bp->b_typ)
    {
        case 1:                    /* Ascii */
            /* Bugfix?  Pc was bp->b_lo...  that setup allowed going past
             * the end if the lower bound was not right. */

            MovASC ((bp->b_hi) - Pc + 1, 'L');
            break;                  /* bump PC  */
        case 6:                     /* Word */
        case 4:                     /* Long */
            PBytSiz = 2;            /* Takes care of both Word & Long */
        case 2:                     /* Byte */
        case 5:                     /* Short */
            MovBytes (bp);
            break;
        case 3:                    /* "C"ode .. not implememted yet */
            break;
        default:
            break;
    }

    NowClass = 0;
}

/* ******************************************************************** *
 * IsCmd() - Checks to see if code pointed to by p is valid code.       *
 *      On entry, we are poised at the first byte of prospective code.  *
 * Returns: pointer to valid lkuptable entry or 0 on fail               *
 * ******************************************************************** */

static struct lkuptbl *
IsCmd (int *fbyte, int *csiz)
{
    struct lkuptbl *T;          /* pointer to appropriate tbl   */
    register int sz;            /* # entries in this table      */
    int c = fgetc (progpath);

    *csiz = 2;

    switch (*fbyte = c)
    {
        case '\x10':
            T = Pre10;
            c = fgetc (progpath);
            *fbyte =(*fbyte <<8) + c;
            sz = sizeof (Pre10) / sizeof (Pre10[0]);
            break;
        case '\x11':
            T = Pre11;
            c = fgetc (progpath);
            *fbyte =(*fbyte <<8) + c;
            sz = sizeof (Pre11) / sizeof (Pre11[0]);
            break;
        default:
            *csiz = 1;
            T = Byte1;
            sz = sizeof (Byte1) / sizeof (struct lkuptbl);
            break;
    }

    while ((T->cod != c))
    {
        if (--sz == 0)
        {
            return 0;
        }

        ++T;
    }

    AMode = T->amode;

    return ((T->cod == c) && (T->t_cpu <= CpuTyp)) ? T : 0;
}

/* **************************************************************** *
 * GetIdxOffset () - Reads offset bytes for an indexed instruction  *
 *   (1 or 2 depending on the postbyte)                             *
 * Passed:  The postbyte                                            *
 * Returns: The signed integer offset read from the current file    *
 *          position                                                *
 *   Also updates Pc by size of "offset"                            *
 * **************************************************************** */

static int
GetIdxOffset (int postbyte)
{
    int offset,
        msk;

    char byt_offset;

            /* Set up for n,R or n,PC later if other
            * cases don't apply */

    switch (postbyte & 1)
    {                   /* postbyte size */
    case 0:            /* single byte */
        byt_offset = (char)fgetc (progpath);
        offset = (int)byt_offset;
        break;
    default:           /* 16-bit (only other option */
        msk = o9_fgetword (progpath);

        /* The Following makes the 16-bit word a signed value. */

        if (msk > 0x7fff)
        {
            offset = (-1) ^ 0xffff;     /* Sort of a sign-extend */
        }
        else
        {
            offset = 0;     /* to be or'ed with the raw obtained value */
        }

        offset |= msk;

        break;
    }

    Pc += (postbyte & 1) + 1;

    return offset;
}

/* **************************************************************** *
 * regput() - Common setup for n,R or n,Pc indexed modes.           *
 *                                                                  *
 * Passed : (1) pbyte- the postbyte, (to determine whether ofst is  *
 *                     8-bit or 16-bit                              *
 *          (2) op1 - pointer to oper1 (the first part of the       *
 *                    opcode string data                            *
 *          (3) pcrel- Flag if it's PCREL or not.                   *
 * **************************************************************** */

char *opfmt[] = {"%s%02x", "%s%04x"};
int bytmsk[] = {0xff, 0xffff};

static void
regput (int pbyte, char *op1, int pcrel)
{
    int ofst;
    char tmp[10];
    int bofst = pbyte & 1;
    int oldpc = Pc;

    ofst = GetIdxOffset (pbyte);

    if (IsROF)
    {
        struct rof_extrn *myref;
        struct nlist *nl;

        if ((myref = find_extrn (xtrn_code, oldpc)))
        {
            sprintf (pbuf->opcod, opfmt[bofst], pbuf->opcod,
                                                ofst & bytmsk[bofst]);
            if (myref->Extrn)
            {
                if (Pass2)
                {
                    strcpy (op1, myref->name);

                    /* PCR mode is stored relative to the address
                     * ofst is already a signed number, so all we
                     * need to do here is add in the Pc address
                     */

                    if (pcrel)
                    {
                        ofst += Pc;
                    }

                    /*if ((ofst != 0) || (dozeros))*/   /* don't want xx+0 */
                    if ((ofst != 0))
                    {
                        sprintf (&(op1[strlen(op1)]), "%+d", ofst);
                    }
                }
            }
            else
            {
                if (ClasHere (LAdds[AMode], CmdEnt))
                {
                    LblCalc (op1, ofst, AMode);
                }
                else
                {
                    if (Pass2)
                    {
                        nl = FindLbl (ListRoot (rof_class (myref->Type)), ofst);

                        if (nl)
                        {
                            strcpy (op1, nl->sname);
                        }
                    }
                    else
                    {
                        addlbl (ofst, rof_class (myref->Type));
                    }
                }

            }

            /* I believe the above logic is the same as the commented-
             * out logic below
             */
        /*    if (Pass2)
            {
                if (myref->Extrn)
                {
                    strcpy (op1, myref->name);
                }
                else
                {
                    LblCalc (op1, ofst, AMode);
        //            nl = FindLbl (ListRoot (rof_class (myref->Type)), ofst);

        //            if (nl)
        //            {
          //              strcpy (op1, nl->sname);
           //         }
                }
            }
            else
            {
                if (!myref->Extrn)
                {
                    //addlbl (ofst, rof_class (myref->Type));
                    LblCalc (op1, ofst, AMode);
                }
            }*/

            return;
        }
    }

    *op1 = '\0';

    if (pbyte & 1)
    {
        if ((ofst < 0x80) && (ofst >= -128))
        {
            strcpy (op1, ">");
        }

        sprintf (tmp, "%04x", ofst & 0xffff);
    }
    else        /* If 8-bit, always specify short mode for pcr indexing */
    {           /* Some assemblers don't automatically do 8-bit mode    */
                /* for PCR indexing                                     */

        if (pbyte & 4) /* PCR indexed */
        {
            strcpy (op1, "<");
        }

        sprintf (tmp, "%02x", ofst & 0xff);
    }

    strcat (pbuf->opcod, tmp);

    /* Hmmm...  I think the program _seemed to work without this *
     * "if", but not after the above change..  However, by the   *
     * time we get here, I think all references are exhausted    *
     * and all that's left is strictly a numeric value for ROF's */

/*    if (IsROF && (AMode != AM_REL))
    {
        sprintf (op1, "%s%d", op1, ofst);
    }
    else {*/
        LblCalc (op1, ofst, AMode);
/*    }*/
}

/* ******************************************************************** *
 * TxIdx() - Handle the postbyte(s) of an indexed mode.                 *
 *                                                                      *
 * ******************************************************************** */

static int
TxIdx ()
{
    int postbyte;
    struct databndaries *kls;   /* Class for this address */
    char myclass;
    char oper1[25],
         oper2[5],
         tmp[20];
    char regNam;

    *oper1 = *oper2 = '\0';
    sprintf (tmp, "%02x ", (postbyte = fgetc (progpath)));
    strcat (pbuf->opcod, tmp);
    ++Pc;

    /* Extended indirect    [mmnn] */

    if (postbyte == 0x9f)
    {
        register unsigned short da;

        AMode = AM_EXT;

        if (IsROF)
        {
            struct rof_extrn *myval;
            int destval;

            if ( ! (myval = rof_lblref (&destval)))
            {
                   destval = o9_fgetword (progpath);
            }

            if (Pass2)
            {
                sprintf (pbuf->opcod, "%s%04x", pbuf->opcod, destval);

                if (myval)
                {
                    sprintf (pbuf->operand, "%s[%s]",
                            pbuf->operand, myval->name);
                }
                else {
                    sprintf (pbuf->operand, "%s[%d]", pbuf->operand, destval);
                }
            }
            else
            {
                rof_addlbl (destval, myval);
            }

            return 1;
        }
        else {
            da = o9_fgetword (progpath);
        }

        if (Pass2)
        {
            sprintf (tmp, "%04x", da);
            strcat (pbuf->opcod, tmp);
        }

        Pc += 2;

        LblCalc (oper1, da, AMode);

        if (Pass2)
        {
            sprintf (pbuf->operand, "%s[%s]", pbuf->operand, oper1);
        }

        return 1;
    }
    else    /* Then anything BUT extended indirect */
    {
        regNam = RegOrdr[((postbyte >> 5) & 3)]; /* Current register offset */
        AMode += (postbyte >> 5) & 3;

        if ( ! (postbyte & 0x80))
        {                       /* 0RRx xxxx = 5-bit    */
            int sbit;           /*the offset portion of the postbyte */

                /* 0,r not valid asm mode? (except for rof's)*/
            if ( ! (postbyte & 0x1f))
            {
                return 0;
            }

            sbit = postbyte & 0x0f;     /* 4 bits of data in this mode */

            if (postbyte & 0x10)        /* sign bit */
            {
                sbit -= 0x10;   /* sbit is now a signed integer */
            }

            /* The logic for the below:  rma, at least, will not assemble *
             * any label reference into 5-bit mode.  Probably it would    *
             * suffice to simply say "if (IsROF)", but we'll leave the    *
             * option open for the off-chance that an offset assignment   *
             * would work somewhere                                       */

            if (IsROF &&  ! (ClasHere (LAdds[AMode], CmdEnt)))
            {       /* Don't think that 5-bit mode will occur for labels */
                sprintf (pbuf->operand, "%s%d,%c", pbuf->operand, sbit, regNam);
            }
            else
            {
                LblCalc (pbuf->operand, sbit, AMode);
                sprintf (pbuf->operand, "%s,%c", pbuf->operand, regNam);
            }

            return 1;
        }
        else        /* then it's 8- or 16-bit offset */
        {
            if ((kls = ClasHere (LAdds[AMode], Pc)))
            {
                myclass = kls->b_typ;
                /* set up offset if present */
            }
            else
            {
                myclass = DEFAULTCLASS;
            }

            PBytSiz = (postbyte & 1) + 1;

            switch (postbyte & 0x0f)
            {
                case 0:
                    sprintf (oper1, ",%c+", regNam);
                    break;
                case 1:
                    sprintf (oper1, ",%c++", regNam);
                    break;
                case 2:
                    sprintf (oper1, ",-%c", regNam);
                    break;
                case 3:
                    sprintf (oper1, ",--%c", regNam);
                    break;
                case 4:
                    if (dozeros)
                    {
                        LblCalc (oper1, 0, AMode);
                        sprintf (oper1, "%s,%c", oper1, regNam);
                    }
                    else
                    {
                        sprintf (oper1, ",%c", regNam);
                    }

                    break;
                case 5:
                    sprintf (oper1, "b,%c", regNam);
                    break;
                case 6:
                    sprintf (oper1, "a,%c", regNam);
                    break;
                case 0x0b:
                    sprintf (oper1, "d,%c", regNam);
                    break;
                case 0x08:                      /*  n,R */
                case 0x09:                      /* nn,R */
                    regput (postbyte, oper1, 0);
                    sprintf (oper1, "%s,%c", oper1, regNam);
                    break;
                case 0x0c:                      /*  n,PC (8-bit) */
                case 0x0d:                      /* nn,PC (16 bit) */
                    AMode = AM_REL;
                    /* below is a temporary fix */
                    myclass = DEFAULTCLASS;
                    regput (postbyte, oper1, 1);

                    sprintf (oper1, "%s,%s", oper1, "pcr");
                    break;
                default:           /* Illegal Code */
                    return 0;
            }

            if (postbyte & 0x10)
            {
                sprintf (pbuf->operand, "%s[%s]", pbuf->operand, oper1);
            }
            else
            {
                sprintf (pbuf->operand, "%s%s", pbuf->operand, oper1);
            }
        }
    }

    return 1;
}

/* ******************************************************************** *
 * GetCmd(): parse through data and assemble (if valid) an assembler    *
 *         command.                                                     *
 * Return Status:                                                       *
 *    Success: file positioned at next command                          *
 *    Failure: file positioned same as entry                            *
 * ******************************************************************** */

static int
GetCmd ()
{
    char tmp[25];
    int firstbyte;  /* First byte of code */
    struct lkuptbl *tbl;
    int pcbump = 0, noncode = 0;
    unsigned int pbyte;
    char **cptr;
    int offset;
    char byte_offset;
    long file_pos = ftell (progpath);   /* Save entry position in file */
    int oprandpc;
    int a = 0;

    *tmp = '\0';
    CmdLen = 0;
    CmdEnt = Pc;                /* Save this entry point for case of bad code */

    if ( ! (tbl = IsCmd (&firstbyte, &pcbump)))
    {
        /* Need to provide for cleanup of noncode */
        /* Reset file pos to begin */
        fseek ( progpath, file_pos, SEEK_SET);
        return 0;
    }

    /* If illegal code is present, then print it out if pass 2 */

    if (noncode && Pass2)
    {
        /* Pc hasn't moved, data from Pc to
         * p - 1 is noncode     */
        return 0;
    }

    /* Now move stuff to printer buffer if Pass2 */

    if (Pass2)
    {
        if (pcbump == 2)
        {
            sprintf (pbuf->instr, "%04x", firstbyte);
        }
        else
        {
            sprintf (pbuf->instr, "%02x", firstbyte);
        }

        strcpy (pbuf->mnem, tbl->mnem);
    }

    Pc += pcbump;
    pcbump = 0;                 /* reset offset - not sure if needed */
    AMode = tbl->amode;
    PBytSiz = tbl->adbyt;

    /* Special case for OS9 */

    if ((OSType == OS_9) &&  ! (strncasecmp (tbl->mnem, "swi2", 4)))
    {
        register unsigned int ch;
        register struct nlist *nl;

        strcpy (pbuf->mnem, "os9");

        sprintf (pbuf->opcod, "%02x", (ch = fgetc (progpath)));
        ++Pc;

        if (IsROF)
        {
            struct rof_extrn *myref;

            myref = find_extrn (xtrn_code, Pc - 1);

            if (myref)
            {
                if (Pass2)
                {
                    strcat (pbuf->operand, myref->name);
                }

                return 1;
            }

        }

        if ( ! Pass2)
        {
            addlbl (ch, '!');
        }
        else
        {
            if ((nl = FindLbl (SymLst[strpos (lblorder, '!')], ch)))
            {
                strcat (pbuf->operand, nl->sname);
            }
            else
            {
                sprintf (pbuf->operand, "%02x", ch);
            }
        }
        return 1;
    }

    /* take care of aim/oim/tim immediate # */
    /* TODO  add addressing mode for this!!! */

#ifdef OSK
    if (findstr (0, tbl->mnem, "im"))
#else
    if (strstr (tbl->mnem, "im"))
#endif
    {
        unsigned int im = fgetc (progpath);

        ++Pc;
        sprintf (pbuf->opcod, "%02x", im);
        sprintf (pbuf->operand, "#$%02x,", im);
    }

    switch (AMode)
    {
        register int ct,
                 tmpbyt;
        register char **regpt;
        char tmpbuf[15];


    case AM_INH:
        return 1;               /*Nothing else to do */
    case AM_XIDX:
        if ( ! TxIdx ())
        {                       /* Process Indexed mode */
            return 0;           /* ?????? */
        }
        return 1;
        break;
    case AM_PSH:
        pbyte = fgetc (progpath);
        ++Pc;
        sprintf (pbuf->opcod, "%02x", pbyte & 0xff);
        cptr = PshPuls;

        if ((pbyte == 0x36) || (pbyte == 0x37))
        {
            cptr = PshPulu;
        }

        for (ct = 0; ct < 8; ct++)
        {

            if (pbyte & 1)
            {
                if (ct == 1)  /* Flag that reg "a" was included */
                {
                    a = 1;
                }

                if (strlen (pbuf->operand))
                {
                    if ((ct == 2) && (a))
                    {
                        pbuf->operand[strlen (pbuf->operand) - 1] = 'd';
                    }
                    else
                    {
                        strcat (pbuf->operand, ",");
                        strcat (pbuf->operand, cptr[ct]);
                    }
                }
                else
                {
                    strcat (pbuf->operand, cptr[ct]);
                }

            }

            pbyte = pbyte >> 1;
        }
        break;
    case AM_TFM:
        pbyte = fgetc (progpath);
        ++Pc;
        sprintf (pbuf->opcod, "%02x", pbyte & 0xff);
        ct = (pbyte >> 4) & 0x0f;
        pbyte &= 0x0f;

        if ((ct > 4) || (pbyte > 4))
        {
            return 0;           /* Illegal */
        }

        /* do r0 */
        strcpy (pbuf->operand, RegReg[ct]);

        if ( ! ((tbl->cod) & 1))
        {
            strcat (pbuf->operand, "+");
        }
        else
        {
            if (tbl->cod == 0x39)
            {
                strcat (pbuf->operand, "-");
            }
        }

        /* now "r1" */
        strcat (pbuf->operand, ",");
        strcat (pbuf->operand, RegReg[(pbyte) & 0x0f]);

        switch (tbl->cod)
        {
        case 0x38:
        case 0x3b:
            strcat (pbuf->operand, "+");
            break;
        case 0x39:
            strcat (pbuf->operand, "-");
            break;
        default:
            break;
        }
        break;
    case AM_REG:
        if (CpuTyp == M_09)
        {
            regpt = RegReg;
        }
        else
        {
            regpt = RegR03;
        }

        pbyte = fgetc (progpath);
        ++Pc;
        sprintf (pbuf->opcod, "%02x", pbyte & 0xff);
        ct = (pbyte >> 4) & 0x0f;
        pbyte &= 0x0f;

        /* abort if 6809 codes are out of range */
        if (CpuTyp == M_09)
        {
            if ((ct > 0x0b) || (pbyte > 0x0b))
            {
                return 0;       /* Illegal */
            }
        }

        if ( ! strlen (regpt[ct]) || !strlen (regpt[pbyte]))
            return 0;

        /* Can't tfr between different-sized registers */

        if (ct != 0x0c)         /* "0" register for 6309 */
        {
            if ((ct & 0x08) != (pbyte & 0x08))
            {
                return 0;       /* mismatched register size */
            }
        }

        sprintf (pbuf->operand, "%s,%s", regpt[ct], regpt[(pbyte) & 0x0f]);
        break;
    case AM_BYTI:
    case AM_DIMM:
    case AM_XIMM:
    case AM_YIMM:
    case AM_UIMM:
    case AM_WIMM:
        strcpy (pbuf->operand, "#");
    case AM_DRCT:
    case AM_EXT:
    case AM_REL:
        oprandpc = Pc;

        switch (tbl->adbyt)
        {
        case 4:                /* only one instance, "ldq (immediate mode)" */
            offset = o9_fgetword (progpath);
            sprintf (pbuf->opcod, "%04x", offset & 0xffff);
            sprintf (pbuf->operand, "%s$%s", pbuf->operand, pbuf->opcod);
            Pc += 2;

            offset = o9_fgetword (progpath);
            sprintf (pbuf->opcod, "%s%04x", pbuf->opcod, offset & 0xffff);
            sprintf (pbuf->operand, "%s%04x", pbuf->operand, offset & 0xffff);
            Pc += 2;
            return 1;           /* done */
        case 2:
            offset = o9_fgetword (progpath);

            /* Note:  This has the added(?) benefit of making negative
             * immediate offsets show up as decimal values.
             * Reminder: should o9_fgetword() return a signed value
             * all the time???
             */

            if (offset & 0x1000)        /* Sign extend */
            {
                offset |= (-1) ^ 0xffff;
            }

            sprintf (pbuf->opcod, "%s%04x", pbuf->opcod, offset & 0xffff);
            Pc += 2;
            
            /* Force 16-bit mode to match original code */
            /* NOTE: We may wish to add an option to turn this funcion
             * on to optimize the code.  This is in order to obtain
             * identical code for comparison.
             */

            if (AMode == AM_EXT)
            {
                if ((offset < 0x80) && (offset >= -128))
                {
                    sprintf (pbuf->operand, ">");
                }
            }

            break;
        default:
            /* Go through byte_offset to get signed stuff working */
            byte_offset = (char)fgetc (progpath);
            offset = (int)byte_offset;
            ++Pc;

            if ((AMode == AM_DRCT) || (AMode == AM_BYTI))
            {
                offset &= 0xff;
            }

            sprintf (pbuf->opcod, "%s%02x", pbuf->opcod, offset & 0xff);

            if ((AMode == AM_DRCT) && Show8bit)
            {
                strcpy (pbuf->operand, "<");
            }

            break;
        }

        if (IsROF)
        {
            struct rof_extrn *myref;

            myref = find_extrn (xtrn_code, oprandpc);

            if (myref)
            {
                char C = rof_class (myref->Type);
                struct nlist *nl;

                if (Pass2)
                {
                    nl =  FindLbl (ListRoot (C), offset);

                    if (nl)
                    {
                        sprintf (pbuf->operand, "%s%s", pbuf->operand,
                                                        nl->sname);
                    }
                    else
                    {
                        if (strlen (myref->name))
                        {
                            sprintf (pbuf->operand, "%s%s", pbuf->operand,
                                                            myref->name);
                        }
                        else        /* Will this ever happen??? */
                        {
                            sprintf (pbuf->operand, "%s%c%04x", pbuf->operand,
                                    C, offset);
                        }
                    }

                    /* We need now to take into account the fact that a
                     * variable could have been referenced with an offset
                     */

                    /* Relative addressing mode is stored differently from
                     * all the rest.  The base is assumbed to be 0, and
                     * stored as offset from begin of next instruction
                     */

                    if (AMode == AM_REL)
                    {
                        offset += Pc;

                        /* Restore back to 8 or 16 bit status because
                         * the add may have wrapped past 0
                         * We need to do this because of next action
                         */

                        offset &= (tbl->adbyt == 1) ? 0xff : 0xffff;
                    }

                    if (offset)
                    {
                        int bytpt = tbl->adbyt >> 1;
                        unsigned int signval[] = {0x80, 0x8000};
                        int signmsk[] = {0xff, 0xffff};

                        if (offset >= signval[bytpt])
                        {
                            offset |= ((-1) ^ (signmsk[bytpt]));
                        }

                        sprintf (&(pbuf->operand[strlen(pbuf->operand)]),
                                    "%+d", offset);
                    }
                    //sprintf (pbuf->opcod, "%s%04x", pbuf->opcod, offset);
                }
                else        /* else Pass 1 */
                {
                    rof_addlbl (offset, myref);
                }

                return 1;
            }
        }

        LblCalc (pbuf->operand, offset, AMode);
        break;

    case AM_BIT:
        pbyte = fgetc (progpath);
        ++Pc;

        sprintf (pbuf->opcod, "%02x", pbyte);
        pbyte &= 0xff;
        tmpbyt = pbyte >> 6;

        if (tmpbyt > 3)
        {
            return 0;
        }

        sprintf (pbuf->operand, "%s.%d,",
                    PshPuls[(int)tmpbyt], (pbyte >> 3) & 7);
        sprintf (tmpbuf, "$%02x", fgetc (progpath));
        ++Pc;
        strcat (pbuf->operand, tmpbuf);
        strcat (pbuf->operand, ".");
        strcat (pbuf->opcod, &tmpbuf[1]);   /* drop leading "$" */
        tmpbyt = pbyte & 0x7;
        sprintf (tmpbuf, "%d", tmpbyt);
        strcat (pbuf->operand, tmpbuf);
        break;

    }

    return 1;
}

/*	This that follows may not be exactly what we will use
 *	but save it.. it's the pattern for pasing the tree
 */


char prfmt[] = "%10s %04x\n";

void
DoPrt (struct nlist *nl)
{
    if (!nl)
    {
        fprintf (stderr, "Error, ptr is null!\n");
        return;
    }
    if (nl->LNext)
    {
        DoPrt (nl->LNext);
    }

    printf (prfmt, nl->sname, nl->myaddr);

    if (nl->RNext)
    {
        DoPrt (nl->RNext);
    }
}

/* ******************************** *
 * rsdoshdr () - read and interpret *
 *    preamble info                 *
 * ******************************** */

void
rsdoshdr (void)
{
    char here = fgetc (progpath);
    int length;

    if (here)
    {
        fprintf (stderr, "First character in file %s not 0\n", modfile);
        exit (1);
    }

    length = o9_fgetword (progpath);
    ModLoad = o9_fgetword (progpath);
    CmdEnt = ModLoad;
    Pc = ModLoad;
    CodEnd = ModLoad + length;
    HdrLen = (int)ftell (progpath);

    if (Pass2)
    {
        fprintf(stderr, "ModLoad = %x   CodEnd = %x\n", ModLoad, CodEnd);
    }
}

/* ****************************** *
 * os9hdr () - read and interpret *
 *    module header               *
 * ****************************** */

void
os9hdr (void)
{
    /* Check to be sure it _is_ a 6809 Memory Module */
    if (o9_fgetword (progpath) != 0x87cd)
    {
        fprintf (stderr, "The file %s is NOT a 6809 Memory Module!!!\n",
                 modfile);
        exit (1);
    }

    /* Translate Header information in Header to big-endian format */
    ModSiz = o9_fgetword (progpath);
    addlbl (ModSiz, 'L');
    ModNam = o9_fgetword (progpath);
    addlbl (ModNam, 'L');
    ModTyp = fgetc (progpath);
    ModRev = fgetc (progpath);
    fgetc (progpath);  /* Discard the parity byte */
    ModExe = o9_fgetword (progpath);
    addlbl (ModExe, 'L');

    if ((ModTyp < 3) || (ModTyp == 0x0c) || (ModTyp > 0x0b))
    {
        ModData = o9_fgetword (progpath);
        PBytSiz = 2;        /* Kludge??? */
        addlbl (ModData, 'D');
        HdrLen = 13;
    }
    else
    {
        ModData = -1;       /* Flag as not used */
        HdrLen = 9;
    }
    CodEnd = ModSiz - 3;

    /* EndAdr: Ptr to end of code (less OS9 CRC bytes
     * .. actually, next byte past last executable code
     * .. (first byte of CRC
     */
    /*EndAdr = (int) ModBegin + ModSiz - 3;*/
}

/* *************************************** *
 * progdis(): mainline disassembly routine *
 * *************************************** */

extern int code_begin;

void
progdis ()
{
    /* Work done by pass 1:
     *      read code char by char.. determine addresses
     *      and set up all labels by type.  The user-defined
     *      mnemonic label names are not read at this time
     */

    long old_pos;

    LinNum = PgLin = 0;

    /* Be sure we start with a clear pbuf */

    memset (pbuf, 0, sizeof (struct printbuf));

    if (Pass2)
    {
        if (OSType == OS_9)
        {
            if (IsROF)
            {
                ROFPsect(rofptr);
            }
            else
            {
                OS9Modline ();
            }
        }

        WrtEquates (1);         /* now do standard named labels */
        WrtEquates (0);         /* write non-standard labels */

        if (OSType == OS_9)
        {
            if (IsROF)
            {
                ROFDataPrint ();
            }
            else
            {
                OS9DataPrint ();
            }
        }
    }

    /* Now begin parsing through program */


    switch (OSType)
    {
        case OS_Coco:
            Pc = ModLoad;
            if (Pass2)
            {
                RsOrg ();
            }
            break;
        default:               /* Os9 */
            if (IsROF)
            {
                Pc = 0;
                fseek (progpath, code_begin, SEEK_SET);
            }
            else
            {
                Pc = HdrLen;       /* Entry point for executable code */
            }
    }

    while (Pc < (CodEnd))
    {
        register struct databndaries *bp;

        CmdLen = 0;
        strcpy (CmdBuf, "");

        /* Try this to see if it avoids buffer overflow */

        memset (pbuf, 0, sizeof (struct printbuf));
        CmdEnt = Pc;

        /* check if in data boundary */

        if ((bp = ClasHere (dbounds, Pc)))
        {
            NsrtBnds (bp);
        }
        else
        {
            old_pos = ftell (progpath); /* Remember position on entry */

            if (GetCmd () && (Pc <= CodEnd))
            {
                if (Pass2)
                {
                    PrintLine (realcmd, pbuf, 'L', CmdEnt, Pc);
                }

                /* allocate byte */
            }
            else   /* if ! GetCmd  */
            {
                /* Condition at this point:  Either GetCmd returned
                 * NULL, implying illegal code, or we've run past the
                 * end of the module, in this case, This wasn't a legal
                 * code, so must either be data or garbage.
                 * In either case, we'll simply pick off a single byte and
                 * write an "fcb" command, bump Pc to next byte and continue.
                 */

                int bcode;

                /* Restore file position to orig */

                fseek (progpath, old_pos, SEEK_SET);
                bcode = fgetc (progpath);
                Pc = CmdEnt;

                if (Pass2)
                {
                    char *pp;

                    memset (pbuf, 0, sizeof (struct printbuf));
                    sprintf (pbuf->instr, "%02x", bcode);
                    strcpy (pbuf->mnem, "fcb");
                    pp = stpcpy (pbuf->operand, "$");
                    sprintf (pp, "%02x", bcode);
                    PrintLine (pseudcmd, pbuf, 'L', CmdEnt, CmdEnt+1);
                }

                Pc = ++CmdEnt;
            }
        }

        /* Coco Dos supports multiple block loads.
         * If we're at the end of the current block,
         * check to see if another follows, and, if so,
         * Update pointers to new state
         */

        if ((OSType == OS_Coco) && (Pc == CodEnd))
        {
            int hstart;

            if ((hstart = fgetc (progpath)) == 0)  /* Another Block of code follows */
            {
                ungetc (hstart, progpath);  /* Restore byte for rsdoshdr() */
                rsdoshdr();

                if (Pass2)
                {
                    /* The following CmdEnt juggling is to try to get
                     * any label equ's just before the new org to print
                     * with the correct offset */

                    /*CmdEnt = prevcmdent;*/
                    RsOrg();
                    /*CmdEnt = newcmdent;*/
                }
            }
            else   /* else it's the end */
            {
                o9_fgetword (progpath); /* Skip two null bytes in Postamble */
                ModExe = o9_fgetword (progpath);
                addlbl (ModExe, 'L');

                /* Now everything should be set to do another block */
            }
        }
    }

    /* Write end-of-file lines */
    if (Pass2)
    {
        switch (OSType)
        {
            case OS_Coco:
                RsEnd ();
                break;
            default:
                if (IsROF)
                {
                    WrtEnds ();
                }
                else
                {
                    WrtEmod ();
                }
        }
    }
}
