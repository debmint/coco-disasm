/*############################################################################
#                                                                            #
# os9disasm - OS9-6809 CROSS DISASSEMBLER                                    # 
#             following the example of Dynamite+                             #
#                                                                            #
# # ######################################################################## #
#
#  $Id$
#                                                                            #
#  Edition History:                                                          #
#  #  Date       Comments                                              by    #
#  -- ---------- -------------------------------------------------     ---   #
#  01 2003/01/31 First began project                                   dlb   #
##############################################################################
# File:  util_dis.c                                                          #
# Purpose: Contain common utilities needed by all prog                       #
############################################################################*/

#include "odis.h"

/* o9_int() - read a string integer representation and conv to int    */

int
o9_int (char *o9num)
{
    unsigned int msb = (unsigned char) o9num[0],
        lsb = (unsigned char) o9num[1];

    if (!o9num)
        return -1;
    return ((msb << 8) + lsb);
}

char *
skipblank (char *p)
{
    while ((*p == ' ') || (*p == '\t'))
        ++p;
    if (*p == '\n')
        *p = 0;
    return p;
}

/* Strpos():  similar to strchr except that it returns the
 * offset from the beginning of the string rather that the
 * actual memory address
 */

int
strpos (char *s, char c)
{
    register char *p;

    return ((p = strchr (s, c)) ? (p - s) + 1 : 0);
/*	if( p=strchr(s,c) )
		return p-s+(char *)1;
	else
		return 0;*/
}


/* PBFcp():	take 1 or 2 byte and moves them to the appropriate
 * *pbuf->instr and pbuf->operand positions  
 */

void
PBFcp (char *dst, char *fmt, unsigned char *src, int sz)
{
    register unsigned int x;

    x = (sz == 2) ? o9_int (src) : *src;
    sprintf (dst, fmt, x);
}

/* PBFcat():	same as PBFcp() except that it cats onto an existing
 * string
 */

void
PBFcat (char *dst, char *fmt, unsigned char *src, int sz)
{
    register unsigned int x;
    char buf[6];

    x = (sz == 2) ? o9_int (src) : *src;
    x &= (sz == 1 ? 0xff : 0xffff);
    sprintf (buf, fmt, x);
    strcat (dst, buf);
}

/* zalloc()  - mallocs memory and if successful, nulls out everything */

/*void *zalloc(size_t size)
{
	void *myadr;

	if( (myadr = malloc(size)) )
		memset(myadr,0,size);
	return myadr;
}*/
