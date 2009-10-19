/* ************************************************************************ *
 *                                                                          $
 * os9disasm - OS9-6809 CROSS DISASSEMBLER                                  $
 *            following the example of Dynamite+                            $
 *                                                                          $
 * ************************************************************************ $
 *                                                                          $
 * Edition History:                                                         $
 * #  Date       Comments                                              by   $
 * -- ---------- -------------------------------------------------     ---  $
 * 01 2003/01/31 First began project                                   dlb  $
 * ************************************************************************ $
 * File:  lbltree.c                                                         $
 * Purpose: handle operations on label trees                                $
 *                                                                          *
 * $Id::                                                                    $
 * ************************************************************************ */

#include "odis.h"
            /* Print format for printing byte/integer output */
char *OpCodFmt[] = { "", /* Placeholder, bytesize=1 or =2 */ "%02x","%04x" };

#include "amodes.h"

/* ************************************************** *
 * ListRoot() - Returns the Base entry point for      *
 *      the Address Class passed to it                *
 * ************************************************** */

struct nlist *
ListRoot (char symbol)
{
    return SymLst[strpos (lblorder, symbol)];
}


/* ******************************************************************** *
 * dataget() : this will get Long or short labels, inserting them into  *
 *     the label list - for pass 1,                                     *
 * Passed : (1) beginning address l,                                    *
 *          (2) ending address h,                                       *
 *          (3) class c,                                                *
 *          (4) data size.                                              *
 *      We may have to fix this one up!!!                               *
 * ******************************************************************** */

/* NOTE: renamed - not currently used */

static void
no_dataget (struct databndaries *bp, char c, int k)
{
    int x;
    char bf[6];

    /* Let's add ModBegin to save conversion */
    register char *cp = ModBegin + bp->b_lo;
    register char *dtop = ModBegin + bp->b_hi;
    int offset = 0;
    char ofcls;

    if (bp->dofst)
    {
        offset = bp->dofst->of_maj;

        if ( ! bp->dofst->add_to)
            offset = -offset;
        if ((bp->dofst)->incl_pc)
            offset += Pc;
        if ((bp->dofst)->oclas_maj)
            ofcls = (bp->dofst)->oclas_maj;
    }

    /* Assume now that this routine only does data boundaries */
    while (cp < dtop)
    {
        strncpy (bf, cp, k);    /* if it's single byte, it ordered x0 */
        bf[k] = '\0';           /* Be sure it's null terminated */
        x = o9_int (bf);        /* and this takes care of little-endian */
        x += offset;
        if ( ! (Pass2))
        {
            addlbl (x, c);
        }

        cp += k;
    }
}

/* **************************************************************** *
 * bGoBegin():	Go to the beginning of the tree, or branch of tree. *
 *      for Data Boundary tree                                      *
 * **************************************************************** */

struct databndaries *
bGoBegin (struct databndaries *pt)
{
    while (pt->DLeft || pt->DRight)
    {
        if (pt->DLeft)
            pt = pt->DLeft;
        else
            pt = pt->DRight;
    }

    /*  Here, we're at the lowest left-most point on the list */
    return pt;
}

/* **************************************************************** *
 * ClasHere()	See if a Data boundary for this address is defined  *
 * Passed : (1) Pointer to Boundary Class list                      *
 *          (2) Address to check for                                *
 * Returns: Ptr to Boundary definition if found,  NULL if no match. *
 * **************************************************************** */

struct databndaries *
ClasHere (struct databndaries *bp, int adrs)
{
    register struct databndaries *pt;
    register int h = (int) adrs;

    if ( ! (pt = bp))
    {
        return 0;
    }

    while (1)
    {
        if (h < pt->b_lo)
        {
            if (pt->DLeft)
                pt = pt->DLeft;
            else
                return 0;
        }
        else
        {
            if (h > pt->b_hi)
            {
                if (pt->DRight)
                {
                    pt = pt->DRight;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return pt;
            }
        }
    }
}

/* **************************************************************** *
 * LblCalc() - Calculate the Label for a location                   *
 * Passed:  (1) dst - pointer to character string into which to     *    
 *              APPEND result                                       *
 *          (2) adr -  the address of the label                     *
 *          (3) amod - the AMode desired                            *
 * **************************************************************** */

int
LblCalc (char *dst, int adr, int amod)
{
    int raw = adr /*& 0xffff */ ;   /* Raw offset (postbyte) - was unsigned */
    char mainclass,                 /* Class for this location */
         oclass = 0;                /* Class for offset (if present) */

    struct databndaries *kls = 0;
    struct nlist *mylabel = 0;

    if (amod == AM_REL)
    {
        raw += Pc;
    }

    /* if amod is non-zero, we're doing a label class */

    if (amod)
    {
        if ((kls = ClasHere (LAdds[amod], CmdEnt)))
        {
            mainclass = kls->b_typ;

            if (kls->dofst)     /* Offset ? */
            {
                oclass = (char) (kls->dofst->oclas_maj);

                if (kls->dofst->add_to)
                {
                    raw -= kls->dofst->of_maj;
                }
                else
                {
                    raw += kls->dofst->of_maj;
                }

                /* Let's attempt to insert the label for PC-Rel offets here */

                if (kls->dofst->incl_pc)
                {
                    raw += CmdEnt;
                    addlbl (raw, kls->dofst->oclas_maj);
                }
            }
        }
        else
        {
            /*mainclass = DEFAULTCLASS;*/
            mainclass = DfltLbls[AMode-1];
        }
    }
    else
    {                           /* amod=0, it's a boundary def  */
        if (NowClass)
        {
            kls = ClasHere (dbounds, CmdEnt);
            mainclass = NowClass;

            if (kls->dofst)
            {
                oclass = kls->dofst->oclas_maj;

                if (kls->dofst->add_to)
                {
                    raw -= kls->dofst->of_maj;
                }
                else
                {
                    raw += kls->dofst->of_maj;
                }

                if (kls->dofst->incl_pc)
                {
                    raw += Pc;
                }
            }
        }
        else
        {
            return 0;
        }
    }

    if ( ! Pass2)
    {
        addlbl (raw, mainclass);
    }
    else
    {                           /*Pass2 */
        char tmpname[20];

        if ((mylabel = FindLbl (SymLst[strpos (lblorder, mainclass)], raw)))
        {
            PrintLbl (tmpname, mainclass, raw, mylabel);
            strcat (dst, tmpname);
        }
        else
        {                       /* Special case for these */
            if (strchr ("^$@&%", mainclass))
            {
                PrintLbl (tmpname, mainclass, raw, mylabel);
                strcat (dst, tmpname);
            }
            else
            {
                char t;

                t = (mainclass ? mainclass : 'D');
                fprintf (stderr, "Lookup error on Pass 2 (main)\n");
                fprintf (stderr, "Cannot find %c%x\n", t, raw);
             /*   fprintf (stderr, "Cmd line thus far: %s\n", tmpname);*/
                exit (1);
            }
        }

        /* Now process offset, if any */

        if (kls && kls->dofst)
        {
            char c = kls->dofst->oclas_maj;

            if (kls->dofst->add_to)
            {
                strcat (dst, "+");
            }
            else
            {
                strcat (dst, "-");
            }

            if (kls->dofst->incl_pc)
            {
                strcat (dst, "*");

                if (kls->dofst->of_maj)
                {
                    strcat (dst, "-");
                }
                else
                {
                    return 1;
                }
            }

            if ((mylabel = FindLbl (SymLst[strpos (lblorder, c)],
                                    kls->dofst->of_maj)))
            {
                PrintLbl (tmpname, c, kls->dofst->of_maj, mylabel);
                strcat (dst, tmpname);
            }
            else
            {                   /* Special case for these */
                if (strchr ("^$@&", c))
                {
                    PrintLbl (tmpname, c, kls->dofst->of_maj, mylabel);
                    strcat (dst, tmpname);
                }
                else
                {
                    char t;

                    t = (c ? c : 'D');
                    fprintf (stderr, "Lookup error on Pass 2 (offset)\n");
                    fprintf (stderr, "Cannot find %c%x\n", t,
                             kls->dofst->of_maj);
                    //fprintf (stderr, "Cmd line thus far: %s\n", tmpname);
                    exit (1);
                }
            }

        }
    }

    return 1;
}

/* ******************************************************************** *
 * movchr() - Append a char in the desired printable format onto dst    *
 * ******************************************************************** */

static void
movchr (char *dst, unsigned char ch)
{
    char mytmp[10];
    register struct nlist *pp;

    if (isprint (ch & 0x7f) && ((ch & 0x7f) != ' '))
    {
        sprintf (mytmp, "'%c", ch & 0x7f);
        strcat (dst, mytmp);
    }
    else
    {
        if ((pp = FindLbl (ListRoot ('^'), ch & 0x7f)))
        {
            strcat (dst, pp->sname);
        }
        else
        {
            sprintf (mytmp, "$%02x", ch & 0x7f);
            strcat (dst, mytmp);
        }
    }

    if (ch & 0x80)
    {
        strcat (dst, "+$80");
    }
}

/* ******************************************************************** *
 * PrintLbl () - Prints out the label to "dest", in the format needed.  *
 * Passed : (1) dest - The string buffer into which to print the label. *
 *          (2) clas - The Class Letter for the label.                  *
 *          (3)  adr - The label's address.                             *
 *          (4)   dl - ptr to the nlist tree for the label              *
 * ******************************************************************** */

void
PrintLbl (char *dest, char clas, int adr, struct nlist *dl)
{
    char tmp[10];
    short decn = adr & 0xffff;
    register int mask;

    /* Readjust class definition if necessary */

    if (clas == '@')
    {
        if ( (adr <= 9) ||
             ((PBytSiz == 1) && adr > 244) ||
             ((PBytSiz == 2) && adr > 65526)  )
        {
            clas = '&';
        }
        else
        {
            clas = '$';
        }
    }

    switch (clas)
    {
        case '$':       /* Hexadecimal notation */
            if (PBytSiz == 1)       /* This may be a kludge */
            {
                adr &= 0xff;
            }
            else
            {
                adr &= 0xffff;
            }

            sprintf (tmp, OpCodFmt[PBytSiz], adr);
            sprintf (dest, "$%s", tmp);
            break;
        case '&':       /* Decimal */
            sprintf (dest, "%d", decn);
            break;
        case '^':       /* ASCII */
            *dest = '\0';

            if (adr > 0xff)
            {
                movchr (dest, (adr >> 8) & 0xff);
                strcat (dest, "*256+");
            }

            movchr (dest, adr & 0xff);

            break;
        case '%':       /* Binary */
            strcpy (dest, "%");

            if (adr > 0xff)
            {
                mask = 0x8000;
            }
            else
            {
                mask = 0x80;
            }

            while (mask)
            {
                strcat (dest, (mask & adr ? "1" : "0"));
                mask >>= 1;
            }

            break;
        default:
            strcpy (dest, dl->sname);
    }
}

/* ************************************************************************ *
 * addlbl() - Adds a label to list of existing labels                       *
 *          Finds the appropriate list and then parses through the list and *
 *          determines the location where it needs to be.                   *
 * Passed : (1) loc = address of label                                      *
 *          (2) C = class (letter).                                         *
 * Returns: ptr to new entry if added, NULL if match found, or if no        *
 *          addition was made.                                              *
 *          Function returns NULL with no action if C is '$', or '&', or    *
 *          '@', or'^' and > 0x20.                                          *
 * ************************************************************************ */

struct nlist *
addlbl (int loc, char C)
{
    struct nlist * me,
                 * pt;
    register int c_indx = strpos (lblorder, C);

    loc &= 0xffff;              /* 6809 addressing is never more than 2 bytes */

    /* At this point, we will try to separate the non-data class D labels
     * from the data types
     */

    /* (for now, at least), don't add labels for class '@', '$', or '&' */

    if (strchr ("@$&", C))
    {
        return 0;
    }

    if ( ! strchr (lblorder, C))
    {                           /* Nonexistant label class      */
        fprintf (stderr,
                 "Illegal label Class '\\%x' for location %04x Pc = %04x\n",
                C, loc, Pc);
        exit (1);
    }

    /* This may be a kludge - may need to fix later */

    if (C == '^')
    {
        loc &= 0x7f;

        if (loc > 0x20)
        {                       /* don't allow labels for printables
                                   except space */
            return 0;
        }
    }

    /* Now search list to see if label already defined      */

    if ((pt = SymLst[c_indx]))
    {                           /* Already have entries      */
        register int found = 0;

        while ( ! found)
        {
            if (loc < (int) pt->myaddr)
            {
                if (pt->LNext)
                {
                    pt = pt->LNext;
                    continue;
                }
                else
                    ++found;
            }
            else
            {
                if (loc > (int) pt->myaddr)
                {
                    if (pt->RNext)
                    {
                        pt = pt->RNext;
                        continue;
                    }
                    else
                        ++found;
                }
                else
                {               /* Must be equal        */
                    return 0;
                }
            }
        }
    }

    if ( ! (me = calloc (1, sizeof (struct nlist))))
    {
        fprintf (stderr, "Cannot allocate memory for Label.\n");
        exit (errno);
    }

    sprintf (me->sname, "%c%04x", C, loc & 0xffff);

    me->myaddr = loc;

    if (pt)
    {
        if (loc < (int) pt->myaddr)
        {
            if (pt->LNext)
            {
                fprintf (stderr, "Error in tree lookup!");
                exit (1);
            }
            pt->LNext = me;
        }
        else
        {
            if (pt->RNext)
            {
                fprintf (stderr, "Error in tree lookup!");
                exit (1);
            }
            pt->RNext = me;
        }
        me->parent = pt;
    }
    else
    {                           /* First entry to this class */
        SymLst[c_indx] = me;
        me->parent = 0;         /* Not needed, but just to be safe */
    }
    return me;
}

/* **************************************************************** *
 * FindLbl - starting at nl, search tree (or remainder) for a match *
 * Passed : (1) nl = Ptr to starting tree member,                   *
 *          (2) loc = address to match                              *
 * Returned: ptr to correct entry if match,                         *
 *           0 if not found                                         *
 * **************************************************************** */

struct nlist *
FindLbl (struct nlist *nl, int loc)
{
    loc &= 0xffff;
    if (!nl)
        return 0;
    while (1)
    {
        if (loc < nl->myaddr)
        {
            if (nl->LNext)      /* Still another entry */
            {
                nl = nl->LNext;
            }
            else
            {
                return 0;       /* No more in this direction */
            }
        }
        else
        {
            if (loc > nl->myaddr)
            {
                if (nl->RNext)
                {
                    nl = nl->RNext;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return nl;          /* Success */
            }
        }
    }
}
