
/* **************************************************** *
 * rof.c - handles rof files                            $
 *                                                      $
 * $Id::                                                $
 * **************************************************** */


#include "odis.h"

static void get_refs(char *vname, int count, int is_extern);

/* symbol table types */
/* symbol definition/reference type/location */
/* location flags */
#define RELATIVE    0x80        /* Store relative to location of reference */
#define NEGMASK     0x40        /* negate on resolution */
#define CODLOC      0x20        /* Reference is in CODE */
#define DIRLOC      0x10        /* if data, store in DP */
/* type flags */
#define LOC1BYT     0x08        /* one byte long */
/* What reference refers to */
#define CODENT      0x04        /* refers to CODE */
#define DIRENT      0x02        /* refers to DP data */
#define INIENT      0x01        /* refers to init data */
#define LOCMASK     (CODLOC|DIRLOC)

struct asc_data {
    int start,
        length;
    struct asc_data *LNext,
                    *RNext;
};

struct asc_data *dp_ascii = 0,
                *nondp_ascii = 0;

struct rof_glbl *glbl_code = 0,
                *glbl_dp = 0,
                *glbl_nondp = 0,
                *glbls;                     /* Generic global pointer */

struct rof_extrn *xtrn_ndp = 0,
                 *xtrn_dp = 0,
                 *xtrn_code = 0,
                 *extrns;                   /* Generic external pointer */

struct nlist *dp_base,
             *vsect_base,
             *code_base,
             *lblptr;

struct rof_hdr ROF_hd,
               *rofptr = &ROF_hd;

int code_begin,
    idp_begin,
    indp_begin;

extern int CodEnd,
           PrevEnt;
extern char realcmd[];
extern struct printbuf *pbuf;

/* DEBUGGING function */
static void
reflst (struct asc_data *cl)
{

    if (cl->LNext)
    {
        reflst (cl->LNext);
    }

    printf ("   >>>   %04x    %d\n",cl->start,cl->length);

    if (cl->RNext)
    {
        reflst (cl->RNext);
    }
}

/* ******************************************************************** *
 * fstrncpy() - equivalent to strncpy, except that the source comes     *
 *           from a stdio filehandle                                    *
 *           Will copy n-1 chars, and returns a null-terminated string  *
 * Passed : (1) dest - the destination                                  *
 *          (2) n    - the maximum number of chars                      *
 *          (3) fp   - the file ptr for the source                      *
 * Returns: pointer to the stored string                                *
 * ******************************************************************** */

static char *
fstrncpy (char *dest, int n, FILE *fp)
{
    char *pt = dest;

    while (n-- )
    {
        if ( ! (*(pt++) = fgetc (fp)))    /* Null == end of string */
        {
            return dest;
        }
    }

    /* If we get here, we've maxed out the count */

    *pt = '\0';
    return dest;
}

/* ************************************************** *
 * rofhdr() - read and interpret rof header           *
 * ************************************************** */

void
rofhdr (void)
{
    int glbl_cnt,
        ext_count,
        count;          /* Generic counter */

    /* get header data */

    rofptr->sync[0] = o9_fgetword (progpath);
    rofptr->sync[1] = o9_fgetword (progpath);
    rofptr->ty_lan = o9_fgetword (progpath);
    rofptr->valid = fgetc (progpath);

    if ((count = fread ( rofptr->rdate,
                         sizeof (rofptr->rdate),
                         1, progpath)
                       ) != 1)
    {
        fprintf (stderr, "read() FAILED for date - read %d bytes\n",count);
    }

    rofptr->edition = fgetc (progpath);
    rofptr->version = fgetc (progpath);
    rofptr->udatsz = o9_fgetword (progpath);
    rofptr->udpsz = o9_fgetword (progpath);
    rofptr->idatsz = o9_fgetword (progpath);
    rofptr->idpsz = o9_fgetword (progpath);
    rofptr->codsz = o9_fgetword (progpath);
    rofptr->stksz = o9_fgetword (progpath);
    rofptr->modent = o9_fgetword (progpath);
    fstrncpy (rofptr->rname, sizeof (rofptr->rname), progpath);

    /* Set ModData to an unreasonable high number so ListData
     * won't do it's thing...
     */

    ModData = 0x7fff;

    count = glbl_cnt = o9_fgetword (progpath);

    while (count--)
    {
        char name[100];
        struct nlist *me;
        int adrs;
        int typ;

        fstrncpy (name, sizeof (glbls->name), progpath);
        typ = fgetc (progpath);
        adrs = o9_fgetword (progpath);

        if ((me = addlbl (adrs, rof_class (typ)))) {
            strcpy (me->sname, name);
            me->global = 1;
        }
    }

    /* Code section... read, or save file position   */
    code_begin = ftell (progpath);
    idp_begin = code_begin + rofptr->codsz;
    indp_begin = idp_begin + rofptr->idpsz;

    if (fseek (progpath,
               rofptr->codsz + rofptr->idatsz + rofptr->idpsz, SEEK_CUR) == -1)
    {
        fprintf (stderr, "rofhdr(): Seek error on module");
        exit (errno);
    }

    /* external references... */

    ext_count = o9_fgetword (progpath);

    while (ext_count--)
    {
        char _name[100];
        int ncount;

        fstrncpy (_name, sizeof (_name), progpath);
        ncount = o9_fgetword (progpath);

        /* Get the individual occurrences for this name */

        get_refs (_name, ncount, 1);
    }

    /* Local variables... */

    ext_count = o9_fgetword (progpath);

    get_refs("", ext_count, 0);

    /* common block variables... */
    /* Do this after everything else is done */

    /* Now we're ready to disassemble the code */

    CodEnd = rofptr->codsz;

    fseek (progpath, code_begin, SEEK_SET);

   /* rofdis();*/
}

/* ****************************************************** *
 * rof_class () - returns the Destination reference for   *
 *          the reference                                 *
 * Passed: The Type byte from the reference               *
 * Returns: The Class Letter for the entry                *
 * ****************************************************** */

char
rof_class (int typ)
{
    int class;

    /* We'll tie up additional classes for data/bss as follows
     * B for uninit non-dp
     * D for uninit DP
     * G for init non-dp
     * H for init DP
     *
     */

    char klasses[5] = "BDLGH",
         *real_to = klasses;

    if (typ & 1)
    {
        real_to += 3;
    }

    class = (typ & 0x07) >> 1;

    return real_to[class];
}

/* ************************************************** *
 * rof_addlbl() - Adds a label to the nlist tree if   *
 *                applicable                          *
 *                Copies rof name to nlist name if    *
 *                different
 * Passed: adrs - address of label                    *
 *         ref  - reference structure                 *
 * ************************************************** */

void
rof_addlbl (int adrs, struct rof_extrn *ref)
{
    struct nlist *nl;

    /* The following may be a kludge.  The problem is that Relative
     * external references get added to class C.
     * Hopefully, no external references are needed in the label ref
     * tables.  We'll try this to see...
     */

    if (ref->Extrn)
    {
        return;
    }

    if ((nl = addlbl (adrs, rof_class (ref->Type))))
    {
        if (strlen (ref->name))
        {
            if (strcmp (nl->sname, ref->name))
            {
                strcpy (nl->sname, ref->name);
            }
        }
    }
}

/* ************************************************** *
 * get_refs() - get entries for given reference,      *
 *    either external or local.                       *
 * Passed:  name (or blank for locals)                *
 *          count - number of entries to process      *
 *          1 if external, 0 if local                 *
 * ************************************************** */

static void
get_refs(char *vname, int count, int is_extern)
{
    while (count--)
    {
        unsigned char _ty;
        int _ofst;
        struct rof_extrn *me,
                         **base = 0;

        _ty = fgetc (progpath);
        _ofst = o9_fgetword (progpath);

        /* Add to externs table */

        switch (_ty & 0xf0)
        {
            case 0x00:
                base = &xtrn_ndp;
                extrns = *base;
                break;
            case 0x10:
                base = &xtrn_dp;
                extrns = *base;
                break;

                /* Try to do both pcr and non-pcr together
                 * Don't think it will matter here
                 */
            case 0x20:
            case 0xa0:
                base = &xtrn_code;
                extrns = xtrn_code;
                break;
            default:        /* Error */
                nerrexit ("Unknown external type encountered");
        }

        if (  ! (me=calloc(1, sizeof(struct rof_extrn))))
        {
            nerrexit ("Cannot allocate memory for rof external ref");
        }

        if (strlen(vname))
        {
            strncpy (me->name, vname, sizeof(me->name));
        }

        me->Type = _ty;
        me->Ofst = _ofst;
        me->Extrn = is_extern;

        /* If this tree has not yet been initialized, simply set the
         * base pointer to this entry (as the first)
         */

        if ( ! *base)
        {
            *base = me;
        }

        /* If we get here, this particular tree has alreay been started,
         * so find where to put the new entry.  Note, for starters, let's
         * assume that each entry will be unique, that is, this location
         * won't be here
         */

        else
        {
            extrns = *base;     /* Use the global externs pointer */

            while (1) {
                struct rof_extrn **to_me = 0;

                if (_ofst < extrns->Ofst)
                {
                    if (extrns->LNext)
                    {
                        extrns = extrns->LNext;
                        continue;
                    }
                    else
                    {
                        to_me = &(extrns->LNext);
                    }
                }
                else
                {
                    if (_ofst > extrns->Ofst)
                    {
                        if (extrns->RNext) {
                            extrns = extrns->RNext;
                            continue;
                        }
                        else {
                            to_me = &(extrns->RNext);
                        }
                    }
                }

                if (to_me)          /* I.E. found the proper empty spot */
                {
                    *to_me = me;    /* Point Parent's next ptr to "me"  */
                    me->up = extrns;    /* "my" parent  */
                    break;
                }
            }
        }
    }
}

/* ************************************************** *
 * find_extrn() - find an external reference          *
 * Passed : (1) xtrn - starting extrn ref             *
 *          (2) adrs - Address to match               *
 * ************************************************** */

struct rof_extrn *
find_extrn ( struct rof_extrn *xtrn, int adrs)
{
    int found = 0;

    if (!xtrn)
    {
        return 0;
    }

    while ( ! found)
    {
        if (adrs < xtrn->Ofst)
        {
            if (xtrn->LNext)
            {
                xtrn = xtrn->LNext;
            }
            else {
                return 0;
            }
        }
        else {
            if (adrs > xtrn->Ofst)
            {
                if (xtrn->RNext)
                {
                    xtrn = xtrn->RNext;
                }
                else {
                    return 0;
                }
            }
            else {
                if (adrs == xtrn->Ofst)
                {
                    return xtrn;
                }
            }
        }
    }

    /* Don't think we'll get here, but to prevent compiler error message */

    return 0;
}

/* ************************************************************ *
 * rof_lblref() - Process a label reference found inthe code.   *
 *       On entry, Pc points to the begin of the reference      *
 * Passed: pointer to int variable to store value of operand    *
 *         value                                                *
 * Returns: pointer to the rof_extern entry, with a label name  *
 *       added, if applicable                                   *
 * ************************************************************ */

struct rof_extrn *
rof_lblref (int *value)
{
    struct rof_extrn *thisref;

    if ( ! (thisref = find_extrn (xtrn_code, Pc)))
    {
        return 0;
    }

    /**value = getc (progpath);
    ++Pc;*/

    if ( ! (thisref->Type & LOC1BYT))
    {
        *value = getc (progpath);
        ++Pc;
        /**value = (*value << 8) | getc (progpath);
        ++Pc;*/
    }
    else
    {
        *value = o9_fgetword (progpath);
        Pc += 2;
    }

    if ( ! (Pass2) && ! (thisref->Extrn))
    {
        if ( ! strlen(thisref->name))
        {
            sprintf (thisref->name, "%c%04x",
                    rof_class (thisref->Type) , *value);
        }
    }

    return thisref;
}

/* ******************************************************** *
 * rof_find_asc() - Find an ascii data block def            *
 * Passed : (1) tree - ptr to asc_dat tree                  *
 *          (2) entry - Command entry point (usually CmdEnt *
 * Returns: tree entry if present, 0 if no match            *
 * ******************************************************** */

static struct asc_data *
rof_find_asc (struct asc_data *tree, int entry)
{
    if (!tree)
    {
        return 0;
    }

    while (1)
    {
        if (entry < tree->start)
        {
            if (tree->LNext)
            {
                tree = tree->LNext;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (entry > tree->start)
            {
                if (tree->RNext)
                {
                    tree = tree->RNext;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return tree;
            }
        }
    }
}

/* ******************************************************** *
 * rof_datasize() - returns the end of rof data area        *
 * Passed: Label Class letter to search                     *
 * Returns: size of this data area                          *
 *          If not a data area, returns 0                   *
 * ******************************************************** */

int
rof_datasize (char class)
{
    int dsize;

    switch (class)
    {
        case 'D':
            dsize = rofptr->udpsz;
            break;
        case 'H':
            dsize = rofptr->idpsz;
            break;
        case 'B':
            dsize = rofptr->udatsz;
            break;
        case 'G':
            dsize = rofptr->idatsz;
            break;
        default:
            dsize = 0;
    }

    return dsize;
}

/* ******************************************************************** *
 * DataDoBlock - Process a block composed of an initialized reference   *
 *               from a data area                                       *
 * Passed : (1) struct rof_extrn *mylist - pointer to tree element      *
 *          (2) int datasize - the size of the area to process          *
 *          (3) char class - the label class (D or C)                   *
 * ******************************************************************** */

static void
DataDoBlock (struct rof_extrn *mylist, int datasize,
             struct asc_data *ascdat, char class)
{
    struct rof_extrn *srch;
    struct nlist *nl;

    while (datasize > 0)
    {
        int bump = 2,
            my_val;

        /* Insert Label if applicable */

        if ( (nl = FindLbl (ListRoot (class), CmdEnt)) )
        {
            strcpy (pbuf->lbnm, nl->sname);

            if (nl->global)
            {
                strcat (pbuf->lbnm, ":");
            }
        }

        /* First check that mylist is not null. If this vsect has no
         * references, 'mylist' will be null
         */

        if ( mylist && (srch = find_extrn (mylist, CmdEnt)) )
        {

            if (srch->Type & LOC1BYT)
            {
                my_val = fgetc (progpath);
                bump = 1;
                strcpy (pbuf->mnem, "fcb");
            }
            else
            {
                my_val = o9_fgetword (progpath);
                bump = 2;
                strcpy (pbuf->mnem, "fdb");
            }

            if (strlen (srch->name))
            {
                strcpy (pbuf->operand, srch->name);
            }

            else
            {
                if ( (nl = FindLbl (ListRoot (rof_class (srch->Type)),
                                              my_val)) )
                {
                    strcpy (pbuf->operand, nl->sname);
                }
                else
                {
                    sprintf (pbuf->operand, "%c%04x", rof_class (srch->Type),
                           my_val);
                }
            }
        }
        else      /* No reference entry for this area */
        {
            struct asc_data *mydat;

            /* Check for ASCII definition, and print it out if so */

            mydat = rof_find_asc (ascdat, CmdEnt);

            if (mydat)
            {
                Pc = CmdEnt;    /* MovASC sets CmdEnt = Pc */
                MovASC (mydat->length, class);
                CmdEnt += mydat->length;
                PrevEnt = CmdEnt;
                datasize -= mydat->length;
                continue;
            }

            my_val = fgetc (progpath);
            bump = 1;
            strcpy (pbuf->mnem, "fcb");
            sprintf (pbuf->operand, "$%02x", my_val);
        }

        PrintLine (realcmd, pbuf, class, CmdEnt, (CmdEnt + datasize));
        CmdEnt += bump;
        PrevEnt = CmdEnt;
        datasize -= bump;
    }
}

static void
ROFDataLst (struct rof_extrn *mylist, int maxcount, struct asc_data *ascdat,
            char class)
{
    struct rof_extrn *my_ref,
                     *srch;
    int datasize;

    my_ref = mylist;

    /* First, process all entries below this one */

    if (my_ref->LNext)
    {
        ROFDataLst (my_ref->LNext, my_ref->Ofst, ascdat, class);
    }

    /* ************************************** *
     * Now we've returned, process this entry *
     * ************************************** */

    /* Determine how many bytes in this block */

    if (my_ref->RNext)
    {

        srch = my_ref->RNext;

        while (srch->LNext)
        {
            srch = srch->LNext;
        }

        datasize = srch->Ofst - my_ref->Ofst;
    }
    else
    {
        datasize = maxcount - my_ref->Ofst;
    }

    CmdEnt = my_ref->Ofst;
    DataDoBlock (my_ref, datasize, ascdat, class);

    if (my_ref->RNext)
    {
        ROFDataLst (my_ref->RNext, maxcount, ascdat, class);
    }
}

/* ******************************************************** *
 * ListInitROF() - moves initialized data into the listing. *
 *                 Really a setup routine                   *
 * Passed : (1) nl - Ptr to proper nlist, positioned at the *
 *              first element to be listed                  *
 *          (2) mycount - count of elements in this sect.   *
 *          (3) notdp   - 0 if DP, 1 if non-DP              *
 *          (4) class   - Label Class letter                *
 * ******************************************************** */

void
ListInitROF (struct nlist *nl, int mycount, int notdp, char class)
{
    struct rof_extrn *mylist,
                     *srchlst;
    struct asc_data *ascdat;

    /* Set up pointers, and file position */

    CmdEnt = 0;

    if (notdp)
    {
        mylist = xtrn_ndp;
        ascdat = nondp_ascii;
        fseek (progpath, indp_begin, SEEK_SET);
    }
    else
    {
        mylist = xtrn_dp;
        ascdat = dp_ascii;
        fseek (progpath, idp_begin, SEEK_SET);
    }

    nl = ListRoot (class);  /* Entry point for this class's label list */

    if (mylist)
    {
        srchlst = mylist;

        while (srchlst->LNext)
        {
            srchlst = srchlst->LNext;
        }

        if (srchlst->Ofst != 0)   /* I.E., if not 0 */
        {
            DataDoBlock (mylist, srchlst->Ofst, ascdat, class);
        }

        ROFDataLst (mylist, mycount, ascdat, class);
    }
    else
    {
        DataDoBlock(mylist, mycount, ascdat, class);
    }
}

/* *********************************************************** *
 * rof_ascii() - set up ASCII specification for initialized    *
 *               data                                          *
 * Passed: ptr - pointer to command line position, Positioned  *
 *               to the first character of the specification   *
 *               line, past the "=" command file specifier     *
 *         Format for cmdline is "= d|n <start> - <end> or     *
 *                                      <start>/<length>       *
 * *********************************************************** */

void
rof_ascii ( char *cmdline)
{

    struct asc_data *me,
                    **tree = NULL;
    char oneline[80];

    while ((cmdline = cmdsplit (oneline, cmdline)))
    {
        char vsct,      /* vsect type, d=dp, b=bss */
             *ptr;
        int start,
            end;

        ptr = oneline;
        vsct = *ptr;
        ptr = skipblank (++ptr);

        getrange (ptr, &start, &end, 1, 0);

        if (end > 0)
        {
            if (!(me = calloc (1, sizeof (struct asc_data))))
            {
                nerrexit ("Cannot allocate memory for asc_data");
            }

            me->start = start;
            me->length = end - start + 1;

            switch (tolower (vsct))
            {
                case 'd':
                    tree = &dp_ascii;
                    break;
                case 'n':
                    tree = &nondp_ascii;
                    break;
                default:
                    nerrexit ("Unknown ascii tree specification");
            }

            if ( ! (*tree))       /* If this tree has not been yet started */
            {
                *tree = me;
            }
            else
            {
                struct asc_data *srch;

                srch = *tree;

                while (1)
                {
                    if (start < srch->start)
                    {
                        if (srch->LNext)
                        {
                            srch = srch->LNext;
                        }
                        else
                        {
                            srch->LNext = me;
                            return;
                        }
                    }
                    else
                    {
                        if (start > srch->start)
                        {
                            if (srch->RNext)
                            {
                                srch = srch->RNext;
                            }
                            else
                            {
                                srch->RNext = me;
                                return;
                            }
                        }
                        else
                        {
                            fprintf (stderr,
                                     "Address %04x for vsect %c already defined\n",
                                     start, vsct
                                    );
                            return;
                        }
                    }
                }
            }
        }
    }
}
