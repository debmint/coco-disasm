/* ************************************************************************ *
 *                                                                          $
 * os9disasm - OS9-6809 CROSS DISASSEMBLER                                  $
 *             following the example of Dynamite+                           $
 *                                                                          $
 * ************************************************************************ $
 *                                                                          $
 *  Edition History:                                                        $
 *  #  Date       Comments                                              by  $
 *  -- ---------- -------------------------------------------------     --- $
 *  01 2003/01/31 First began project                                   dlb $
 * ************************************************************************ $
 * File:  util_dis.c                                                        $
 * Purpose: Contain common utilities needed by all prog                     $
 *                                                                          *
 * $Id::                                                                    $
 * ************************************************************************ */

#include "odis.h"

/* ******************************************************** *
 * o9_fgetword() - get a 6809-formatted word (double byte)  *
 *      from stream                                         *
 * Passed: FILE *stream                                     *
 * Returns: WORD in native-endian format.                   *
 * ******************************************************** */

int
o9_fgetword(FILE *fp)
{
    int msb,
        lsb;

    msb = fgetc(fp);
    lsb = fgetc(fp);

    return ((msb<<8) + lsb);
}

/* o9_int() - read a string integer representation and conv to int    */

int
o9_int (char *o9num)
{
    unsigned int msb = (unsigned char) o9num[0],
        lsb = (unsigned char) o9num[1];

    if ( ! o9num)
    {
        return -1;
    }

    return ((msb << 8) + lsb);
}

/* **************************************************************** *
 * skipblank() passes over any space, tab, or any newline character *
 *             in the string                                        *
 * Passed: p pointer to begin of string to parse                    *
 * Returns: pointer to first character of "valid" data,             *
 *          or null if end of data                                  *
 * **************************************************************** */

char *
skipblank (char *p)
{
    /* We did just pass over spaces or tabs, but we need to
     * also be sure we are past all return characters
     * ( especially the extra character MS-Dos uses
     */

    while ((*p) && (strchr (" \t\r\f\n", *p)))
    {
        ++p;
    }

    if (*p == '\n')
    {
        *p = 0;
    }

    return p;
}

/* ******************************************************************** *
 * Strpos():  similar to strchr except that it returns the offset from  * 
 *          the beginning of the string rather that the actual          *
 *          memory address                                              *
 * ******************************************************************** */

int
strpos (char *s, char c)
{
    register char *p;

    return ((p = strchr (s, c)) ? (p - s) + 1 : 0);
}


/* PBFcp():	take 1 or 2 byte and moves them to the appropriate
 * *pbuf->instr and pbuf->operand positions
 */

/*void
PBFcp (char *dst, char *fmt, unsigned char *src, int sz)
{
    register unsigned int x;

    x = (sz == 2) ? o9_int (src) : *src;
    sprintf (dst, fmt, x);
}*/

/* PBFcat():	same as PBFcp() except that it cats onto an existing
 * string
 */

/*void
PBFcat (char *dst, char *fmt, unsigned char *src, int sz)
{
    register unsigned int x;
    char buf[6];

    x = (sz == 2) ? o9_int (src) : *src;
    x &= (sz == 1 ? 0xff : 0xffff);
    sprintf (buf, fmt, x);
    strcat (dst, buf);
}*/

#ifndef HAVE_STPCPY
    char *
    stpcpy (char *dest, char *src)
    {
        char *str;

        str = strcpy (dest, src);
        return (str += strlen(str));
    }
#endif

/* MinGw doesn't seem to have functions strchr() or isblank */

/*#ifndef HAVE_INDEX
char *
index (char *s, int c)
{
    while (*s)
    {
        if ( *s == c)
        {
            return s;
        }
        else
        {
            ++s;
        }
    }

    return NULL;
}
#endif*/

#ifndef HAVE_ISBLANK
int
isblank (int c)
{
    if ((c == ' ') || ( c == '\t'))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

#ifndef HAVE_STRNCASECMP
int
strncasecmp (s1, s2, n)
    char *s1,
         *s2;
    int   n;
{
    while (n--)
    {
        if ((*s1 == '\0') && (*s2 == '\0'))
        {
            return 0; /* End of both strings */
        }

        if (toupper (*s1) != toupper (*s2))
        {
            return (*s2 - *s1);
        }

        ++s1;
        ++s2;
    }

    return 0;
}
#endif

#ifndef HAVE_STRCASECMP
int
strcasecmp (char *s1, char *s2)
{
    return (strncasecmp (s1, s2, -1));
}
#endif

#ifndef HAVE_STRDUP
char *
strdup (char *str)
{
    char *dupstr;

    if ( (dupstr = malloc (strlen (str) + 1)) )
    {
        strcpy (dupstr, str);
    }

    return dupstr;
}
#endif

/* This is a debugging function */

void printbounds (struct databndaries *bnd)
{
    if (bnd->DLeft)
    {
        printbounds (bnd->DLeft);
    }

    printf ("b_lo %x; b_hi %x b_typ %c b_class %d\n", bnd->b_lo, bnd->b_hi,
                                                      bnd->b_typ, bnd->b_class);

    if (bnd->dofst)
    {
        struct ofsetree *o = bnd->dofst;

        printf ("    oclas_maj %c, of_maj %x, add_to %d, incl_pc %d\n",
                o->oclas_maj, o->of_maj, o->add_to, o->incl_pc);
    }

    if (bnd->DRight)
    {
        printbounds (bnd->DRight);
    }
}

void
listbounds ()
{
    struct databndaries *dbs;
    int count;

    for (count = 1; count < 32; count++)
    {
        dbs = LAdds[count];

        if (dbs)
        {
            printf ("-------\n");
            printbounds(dbs);
        }
    }
}

void
nerrexit (char *l)
{
    fprintf (stderr, l);
    fprintf (stderr, " Line #%d\n", LinNum);
    exit (1);
}

/* This is a debugging function */

void
listlbls ()
{
    struct nlist *nl;
    int count;

    for (count = 1; count < strlen (lblorder); count++)
    {
        nl = SymLst[count];
        if (nl)
        {
            printf ("Labels of class: %c\n", lblorder[count]);
            DoPrt (nl);
        }
    }
}
