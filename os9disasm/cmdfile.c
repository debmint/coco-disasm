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
# File:  cmdfile.c                                                           #
# Purpose: process command file                                              #
############################################################################*/

/*#include <ctype.h>*/
#define _GNU_SOURCE     /* Needed to get isblank() defined */
#include "odis.h"
#include "amodes.h"

static int NoEnd;      /* Flag that no end on last bound                 */
static int TypeGet;    /* Flag 1=getting Addressing mode 0=Data Boundary */
static struct databndaries *prevbnd ;

void
do_cmd_file ()
{
    FILE *cmdfp;
    char miscbuf[240];
    char *mbf = miscbuf;

    NxtBnd = 0;                 /* init Next boundary pointer */
    
    if (!(cmdfp = fopen (cmdfilename, "rb")))
    {
        fprintf (stderr, "Erorr # %d: Cannot open cmd file %s for read\n",
                 errno, cmdfilename);
        exit (1);
    }

    while (fgets (miscbuf, sizeof (miscbuf), cmdfp))
    {
        char *th;
        ++LinNum;
       
        /* Convert newline to null */

        if ((th = (char *) strchr (miscbuf, '\n')))
        {
            *th = '\0';
        }
        
        mbf = skipblank (miscbuf);
        
        if (!strlen (mbf))      /* blank line? */
        {
            continue;
        }
        
        if (*mbf == '*')        /* Cmdfile Comment?     */
        {
            continue;           /*yes, ignore   */
        }
        
        if (*mbf == '+')
        {
            if (optincmd (++mbf) == -1)
            {   /* an error in an option would probably be fatal, anyway */
                fprintf (stderr, "Error processing cmd line #%d\n", LinNum);
                exit (1);
            }

            continue;
        }

        if (*mbf == '"')        /* Assembler file comment(s)    */
        {
            if (asmcomment (++mbf, cmdfp) == -1)
            {
                fprintf(stderr, "Drror processing cmd line #%d\n", LinNum);
                exit (1);
            }

            continue;
        }
        
        if (*mbf == '>')
        {
            cmdamode (skipblank (++mbf));
            continue;
        }

        /* rof ascii data definition */

        if (*mbf == '=')
        {
            rof_ascii (skipblank (++mbf));
            continue;
        }
        
        if (strchr (BoundsNames, toupper (*mbf)) || isdigit (*mbf))
        {
            boundsline (mbf);
        }
        else   /* All possible options are exhausted */
        {
            fprintf (stderr, "Illegal cmd or switch in line %d\n", LinNum);
            exit (1);
        }
    }
}

/* *********************************************** *
 * newcomment() adds new commenttree address       *
 * Passed: address for comment,                    *
 *         parent or null if first in tree         *
 * *********************************************** */
struct commenttree *
newcomment (int addrs, struct commenttree *parent)
{
    struct commenttree *newtree;

    if (!(newtree = calloc(1,sizeof (struct commenttree))))
    {
        fprintf (stderr, "Cannot allocate memory for commenttree\n");
        exit (1);
    }

    newtree->adrs = addrs;
    newtree->cmtUp = parent;
    return newtree;
}

/* ************************************************************************ *
 * asmcomment() Add comments to be placed into the assembler source/listing *
 * Format for this entry:                                                   *
 *      " addr Dtext                                                        *
 *      ---                                                                 *
 *      textD\n                                                             *
 * D represents any delimiter, following lines will be added until a line   *
 *      ended by the delimiter is encountered.  The delimiter can be        *
 *      included in the text as long as it's not the last char on the line. *
 *      The delimiter can be the last character if it's doubled             *
 * ************************************************************************ */

int
asmcomment (char *lpos, FILE *cmdfile)
{
    unsigned int adr = 0;
    register char *txt;
    char delim;
    int lastline;
    struct commenttree *treebase;
    register struct commenttree *me;
    struct cmntline *prevline = 0;
    char lblclass;
    
    lpos = skipblank (lpos);

    lblclass = *(lpos++);       /* first element is Label Class */

    if (isalpha (lblclass) && islower (lblclass))
    {
        lblclass = toupper(lblclass);
    }
    
    if (!strchr(lblorder,lblclass))
    {
        fprintf (stderr, "Illegal label class for comment, Line %d\n", LinNum);
        return -1;
    }
    
    lpos = skipblank (lpos);    /* lpos now points to address */
    
    if (sscanf(lpos,"%x",&adr) != 1)
    {
        fprintf (stderr,"Error in getting address of comment : Line #%d\n",
                 LinNum);
        exit(1);
    }
    
    /* Now find begin of text */

    while (!(isblank(*lpos)))
    {
        ++lpos;
    }

    lpos = skipblank (lpos);     /* Now lpos is begin of (first) line of text */
    
    switch (*lpos)
    {
        case '\r':
        case '\n':
            return 0;    /* Require AT LEAST the delimiter on the first line */
        default:
            delim = *lpos;
            *lpos = '\0';   /* So we can have an empty string for first line */
            ++lpos;
    }

    /* Now locate or set up an appropriate tree structure */

    treebase = Comments[strpos (lblorder, lblclass)];
    
    if (!treebase)
    {
        me = treebase = Comments[strpos (lblorder, lblclass)] =
            newcomment (adr, 0);
    }
    else
    {
        struct commenttree *oldme;
        
       /* int cmtfound = 0;*/
        me = treebase;

        while (1) {
            if (adr < me->adrs)
            {
                if (me->cmtLeft)
                {
                    me = me->cmtLeft;
                    continue;
                }
                else
                {
                    oldme = me;
                    me = newcomment (adr, me);
                    oldme->cmtLeft = me;

                    /*cmtfound = 1;*/
                    break;
                }
            }
            else
            {
                if (adr > me->adrs)
                {
                    if (me->cmtRight)
                    {
                        me = me->cmtRight;
                        continue;
                    }
                    else        /* The same address was previously done */
                    {
                        oldme = me; 
                        me = newcomment (adr, me);
                        oldme->cmtRight = me;
                        break;
                    }
                }
                else
                {
                    /*cmtfound = 1;*/

                    /* Here, we need to find the last comment */
                    prevline = me->commts;
                    while (prevline->nextline) {
                        prevline = prevline->nextline;
                    }

                    /*cmtfound = 1;*/
                    break;
                }
            }
        }       /* while (1) ( for locating or adding comments */
    }           /* end of search in commenttree - me = appropriate tree */

/************************************************************************** */

    /* Now get the comment strings */
    while (1)
    {
        struct cmntline *cline;
        char mbuf[500];
        
        lastline = 0;
        if (strchr(lpos,'\n'))
        {
            *strchr(lpos,'\n') = '\0';
        }
        if (strchr(lpos,'\r'))
        {
            *strchr(lpos,'\r') = '\0';
        }

        /* If this is the last line, set flag for later */
        if (lpos[strlen(lpos) - 1] == delim)
        {
            lastline = 1;
            lpos[strlen(lpos) - 1] = '\0'; /* Get rid of the delimiter */
        }

        /* Now we can finally store the comment */
        if (!(txt = calloc (1, strlen(lpos)+1)))
        {
            fprintf(stderr,"Error - cannot allocate memory for comment\n");
            exit(1);
        }
        strncpy (txt,lpos,strlen(lpos));

        if (!(cline = calloc (1, sizeof (struct cmntline))))
        {
            fprintf (stderr,
                    "Error - cannot allocate memory for comment line\n");
            exit(1);
        }
        if (prevline)
        {
            prevline->nextline = cline;
        }
        else
        {
            me->commts = cline;
        }

        prevline = cline;
        cline->ctxt = txt;

        if (lastline)
        {
            return 0;
        }
        else
        {
            if (!(lpos = fgets(mbuf,sizeof(mbuf),cmdfile)))
            {
                return -1; /* Try to proceed on error */
            }

            ++LinNum;
            lpos = skipblank(lpos);
        }
    }
}

/* ****************************************** *
 * Process options found in command file line *
 * ****************************************** */

int
optincmd (char *lpos)
{
    char st[80], *spt = st;

    while (*(lpos = skipblank (lpos)))
    {
        if (sscanf (lpos, "%80s", spt) != 1)
        {
            return (-1);
        }
        
        lpos += strlen (spt);
        
        if (*spt == '-')
        {
            ++spt;
        }
        
        do_opt (spt);
    }
    return (0);
}

/* ************************************************************** *
 * cmdsplit() :  split cmdline..  i.e. transfer characters up to  *
 * ';' or end of line - returns ptr to next char in cmdline buffer*
 * ************************************************************** */

char *
cmdsplit (char *dest, char *src)
{
    char c;

    src = skipblank (src);

    if (!(c = *src) || (c == '\n'))
        return 0;

    if (strchr (src, ';'))
    {
        while ((c = *(src++)) != ';')
            *(dest++) = c;
        *dest = '\0';
    }
    else
    {
        strcpy (dest, src);
        src += strlen (src);
    }
    return src;
}


/* *************************************************** *
 * Process addressing modes found in command file line *
 * *************************************************** */

void
cmdamode (char *pt)
{
    char buf[80];

    TypeGet = 1;
    
    while ((pt = cmdsplit (buf, pt)))
    {
        DoMode (buf);
    }
}

/* ********************************************* *
 * getrange() This function interprets the range *
 *   specified in the command line and stores    *
 *   the low and high values in "lo" and "hi"    *
 *   (the addresses are passed by the caller     *
 * ********************************************* */

static void
getrange (char *pt, int *lo, int *hi, int usize)
{
    char tmpdat[50], *dpt, c;

    dpt = tmpdat;               /* just to be sure */

    /* see if it's just a single byte/word */
    if (!(isxdigit (*(pt = skipblank (pt)))))
    {
        if ((*pt == '-') || ((*pt == '/') && !TypeGet))
        {
            if (NoEnd)
            {
                nerrexit ("No start address/no end address in prev line %d");
            }
            *lo = NxtBnd;
        }
        else
        {                       /* no range specified */
            if (*pt && (*pt != '\n'))   /* non-digit gargabe */
            {
                nerrexit ("Illegal Address specified");
            }
            else
            {
                /*sprintf (pt, "%x", NxtBnd);*/
                *lo = NxtBnd;
                NxtBnd = *lo + usize;
                *hi = NxtBnd - 1;
                return;   /* We're done with this line */
            }
        }
    }
    else  /* We have a (first) number) */
    {
        pt = movxnum (tmpdat, pt);
        sscanf (tmpdat, "%x", lo);
    }

    /* Scan for second number/range/etc */
    
    switch (c = *(pt = skipblank (pt)))
    {
    case '/':
        if (TypeGet == 1)
        {
            nerrexit ("Cannot specify \"/\" in this mode!");
        }

        switch (*(pt = skipblank (++pt)))
        {
        case ';':
        case '\0':
            if (NoEnd)
            {  /* if a NoEnd from prev cmd, second bump won't be done */
                ++NoEnd;
            }
            
            ++NoEnd;
            *hi = *lo;
            break;
        default:
            pt = movdigit (tmpdat, pt);
            NxtBnd = *lo + atoi (tmpdat);
            /*fprintf(stderr,"Next bdry \\%x\n",NxtBnd);*/
            *hi = NxtBnd - 1;
            break;
        }
        break;
    case '-':
        switch (*(pt = skipblank (++pt)))
        {
        case ';':
        case '\0':
            if (NoEnd)
            {  /* if a NoEnd from prev cmd, second bump won't be done */
                ++NoEnd;
            }
            
            ++NoEnd;          /* Flag need end address */
            *hi = *lo;          /* tmp value */
            break;
        default:
            if (isxdigit (*pt))
            {
                tellme (pt);
                pt = movxnum (tmpdat, pt);
                sscanf (tmpdat, "%x", hi);
                NxtBnd = *hi + 1;
            }
        }
        break;
    default:
        if (!(*pt))
        {
            NxtBnd = *lo + usize;
            *hi = NxtBnd - 1;
        }
    }
    
    if (*pt)
    {
        fprintf(stderr,"|%s|  ",pt);
        nerrexit ("Extra data..");
    }
}

int
DoMode (char *lpos)
{
    struct databndaries *mptr;
    register int class;         /* addressing mode */
    register int notimm = 4;    /* Was 5, but we moved AM_DRCT */
    char c;
    int lo, hi;
    register struct databndaries *lp;
    struct ofsetree *otreept = 0;

    if (*(lpos = skipblank (lpos)) == '#')
    {
        notimm = 0;
        lpos = skipblank (++lpos);
    }
    switch (c = toupper (*(lpos++)))
    {
    case 'D':
			if(notimm)
				AMode=AM_DRCT;
			else
				AMode=AM_DIMM;
			break;
    case 'X':
    case 'Y':
    case 'U':
    case 'S':
        AMode = (int) strpos ("DXYUS", c) + notimm + 1; /*+2 */
        if (!AMode)
        {
            nerrexit ("Class \'%c\' not found");
        }
        break;
    case '1':
        AMode = AM_BYTI;
        break;
    case 'E':
        AMode = AM_EXT;
        break;
    case 'R':
        AMode = AM_REL;
        break;
    default:
        nerrexit ("Illegal addressing mode");
        exit (1);               /* not needed but just to be safe */
    }

    lpos = skipblank (lpos);
    class = *(lpos++);
    class = toupper (class);
    
    if (!index (lblorder, class))
    {
        nerrexit ("Illegal class definition");
    }

    /* Offset spec (if any) */

    /* check for default reset (no address) */
    
    if (!(lpos = skipblank (lpos)) || !(*lpos) || (*lpos == ';'))
    {
        /* Changed the following after change for addlbl() */
        //DfltLbls[AMode - 1] = lblorder[class - 1];
        DfltLbls[AMode - 1] = class;
        return 1;
    }

    if (*(lpos) == '(')
    {
        otreept = calloc (1, sizeof (struct ofsetree));
        
        if (!otreept)
        {
            nerrexit ("Cannot allocate memory for offset!");
        }
        
        lpos = setoffset (++lpos, otreept);
    }

    lpos = skipblank (lpos);

     /*  Hopefully, passing a hard-coded 1 will work always.
     */
    getrange (lpos, &lo, &hi, 1);

    /* Now insert new range into tree */
    
    if (!(mptr = calloc (1, sizeof (struct databndaries))))
    {
        nerrexit ("Cannot allocate memory for data definition");
    }

    mptr->b_lo = lo;
    mptr->b_hi = hi;
    mptr->DLeft = mptr->DRight = 0;
    mptr->b_typ = class;        /*a_mode; */
    mptr->dofst = otreept;

    if (!LAdds[AMode])
    {
        LAdds[AMode] = mptr;
        mptr->dabove = 0;
    }
    else
    {
        lp = LAdds[AMode];
        
        while (1)
        {
            if (hi < lp->b_lo)
            {
                if (lp->DLeft)
                {
                    lp = lp->DLeft;
                    continue;
                }
                else
                {
                    lp->DLeft = mptr;
                    mptr->dabove = lp;
                    return 1;
                }
            }
            else
            {
                if (lo > lp->b_hi)
                {
                    if (lp->DRight)
                    {
                        lp = lp->DRight;
                        continue;
                    }
                    else
                    {
                        lp->DRight = mptr;
                        mptr->dabove = lp;
                        return 1;
                    }
                }
                else
                {
                    nerrexit ("Addressing mode segments overlap");
                }
            }
        }
    }
    return 1;
}

#define ILLBDRYNAM "Illegal boundary name in line %d\n"

/* ********************************************* *
 * process boundaries found in command file line *
 * ********************************************* */

void
boundsline (char *mypos)
{
    char tmpbuf[80];
    register char *hold;
    /*char *stophere=&mypos[strlen(mypos)]; */
    register int count = 1;

    TypeGet = 0;

    if (isdigit (*mypos))
    {                           /*      Repeat count for line   */
        mypos = movdigit (tmpbuf, mypos);
        count = atoi (tmpbuf);
        mypos = skipblank (mypos);
    }

    hold = mypos;

    while (count--)
    {
        char *nextpos;
        mypos = hold;
        
        while ((nextpos = cmdsplit (tmpbuf, mypos)))
        {
            setupbounds (tmpbuf);
            mypos = nextpos;
        }
    }
}

/* ************************************************* *
 * set up offset (if there) for offset specification *
 * ************************************************* */

char *
setoffset (char *p, struct ofsetree *oft)
{
    char c, bufr[80];

    oft->oclas_maj = oft->of_maj = oft->add_to = 0;

    p = skipblank (p);
    if (!strchr (p, ')'))
    {
        nerrexit ("\"(\" in command with no \")\"");
    }
    
    if ((c = toupper (*(p++))) == '*')
    {
        /* Hope this works - flag * addressing like this  */
        oft->incl_pc++;
        p = skipblank (p);      /* used this char, position for next */
        c = toupper (*(p++));
    }
    switch (c)
    {
    case '+':
        ++oft->add_to;
        break;
    case '-':
        break;
    case ')':
        if (!oft->incl_pc)
            nerrexit ("Blank offset spec!!!");
        return p;
    default:
        nerrexit ("No '+', '-', or '*' in offset specification");
    }

    p = skipblank (p);
    c = toupper (*(p++));

    if (! index (lblorder, oft->oclas_maj = c))
        nerrexit ("No offset specified !!");

    p = skipblank (p);
    if (!isxdigit (*p))
        nerrexit ("Non-Hex number in offset value spec");
    /* NOTE: need to be sure string is lowercase and following
     * value needs to go into the structure */
    p = movxnum (bufr, p);
    sscanf (bufr, "%x", &oft->of_maj);
    /*if(add_to)
       oft->of_maj = -(oft->of_maj); */

    if (*(p = skipblank (p)) == ')')
    {
        addlbl (oft->of_maj, c);
    }
    else {
        nerrexit ("Illegal character.. offset must end with \")\"");
    }
    
    return ++p;
}

/* ******************************************* *
 * setupbounds() - The entry point for setting *
 *            up a single boundary             *
 * Entry: lpos = current position in cmd line  *
 * ******************************************* */

void
setupbounds (char *lpos)
{
    struct databndaries *bdry;
    register int bdtyp, lclass = 0;
    int rglo, rghi;
    char c;
    struct ofsetree *otreept = 0;

    TypeGet = 0;
    PBytSiz = 1;                /* Default to single byte */

    /* First character should be boundary type */

    switch (c = toupper (*(lpos = skipblank (lpos))))
    {
    case 'L':
        PBytSiz = 2;
    case 'S':
        lpos = skipblank (++lpos);
        lclass = toupper (*lpos);
        
        if (!index (lblorder, lclass))
        {
            nerrexit ("Illegal Label Class");
        }
        
        break;
    case 'W':
        PBytSiz = 2;
    case 'B':
        lclass = '$';
        break;
    case 'A':
        lclass = '^';
        break;
    default:
        fprintf(stderr, "%s\n", lpos);
        nerrexit ("Illegal boundary name");
    }

    bdtyp = (int) strpos (BoundsNames, c);
    /* Offset spec (if any) */
    lpos = skipblank (++lpos);
    
    if (*(lpos) == '(')
    {
        otreept = calloc (1, sizeof (struct ofsetree));
        if (!otreept)
            nerrexit ("Cannot allocate memory for offset!");
        lpos = setoffset (++lpos, otreept);
    }

    getrange (lpos, &rglo, &rghi, PBytSiz);
    
    /* Now create the addition to the list */
    
    if (!(bdry = calloc (1, sizeof (struct databndaries))))
    {
        fprintf (stderr, "Cannot allocate memory for boundary\n");
        exit (1);
    }

    bdry->b_lo = rglo;
    bdry->b_hi = rghi;
    bdry->b_class = lclass;
    bdry->b_typ = bdtyp;
    bdry->DLeft = bdry->DRight = 0;
    bdry->dofst = otreept;

    /* We had to put it down here where bdry had already been malloc'ed 
     * The way it works:  if a range is open-ended, getrange()
     * flags it 1, This flag passed through here.. We don't want
     * to substitute till the next pass, so we bump it up one
     * again here, so that, actually, "2" causes the substitution
     *
     * Note that we can have two open-ended commands in succession,
     * therefore, the actual logic is substute if NoEnd >= 2
     */

    if (NoEnd)
    {
        switch (NoEnd)
        {
        case 1:                /* NoEnd from this pass */
            ++NoEnd;            /* flag for next pass */
            break;
        default:
            fprintf(stderr,"NoEnd in prev cmd.. inserting \\x%x for prev hi\n", bdry->b_lo);
            fprintf(stderr,"...\n");
            prevbnd->b_hi = bdry->b_lo - 1;
            NoEnd -= 2;          /* undo one flagging */
        }
    }
    
    prevbnd = bdry;             /* save this for open-ended bound */

    if (!dbounds)
    {                           /* First entry  */
        bdry->dabove = 0;
        dbounds = bdry;
    }
    else
    {
        bdinsert (bdry);
    }
}

void
bndoverlap ()
{
    fprintf (stderr, "Data segment overlap in line %d\n", LinNum);
    exit (1);
}

/* ***************************************************** *
 * bdinsert() - inserts an entry into the boundary table *
 * ***************************************************** */

void
bdinsert (struct databndaries *bb)
{
    register struct databndaries *npt;
    register int mylo = bb->b_lo, myhi = bb->b_hi;

    npt = dbounds;              /*  Start at base       */

    while (1)
    {
        if (myhi < npt->b_lo)
        {
            if (npt->DLeft)
            {
                npt = npt->DLeft;
                continue;
            }
            else
            {
                bb->dabove = npt;
                npt->DLeft = bb;
                return;
            }
        }
        else
        {
            if (mylo > npt->b_hi)
            {
                if (npt->DRight)
                {
                    npt = npt->DRight;
                    continue;
                }
                else
                {
                    bb->dabove = npt;
                    npt->DRight = bb;
                    return;
                }
            }
            else
            {
                bndoverlap ();
            }
        }
    }
}

void
tellme (char *pt)
{
    return;
}

char *
movxnum (char *dst, char *src)
{
    while (isxdigit (*(src)))
        *(dst++) = tolower (*(src++));
    *dst = '\0';
    return src;
}

char *
movdigit (char *dst, char *src)
{
    while (isdigit (*(src)))
        *(dst++) = *(src++);
    *dst = '\0';
    return src;
}

int
endofcmd (char *pp)
{
    return (1 ? ((*pp == '\n') || (*pp == ';') || (!(*pp))) : 0);
}

void
nerrexit (char *l)
{
    fprintf (stderr, l);
    fprintf (stderr, " Line #%d\n", LinNum);
    exit (1);
}
