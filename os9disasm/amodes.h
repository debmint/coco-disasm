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

/* amode.h Defines for addressing modes */

#ifndef AMODES

#define AMODES 17	/* only 12, but allow for start @1 plus one */

/* Addressing modes (storage order) */
/* defines offset into data default string */
/* IMPORTANT!  These first 14 entries _MUST_ remain in the given order! */

#define AM_INH	 0	    /*  Inherent (no postbytes)	*/
#define AM_DIMM	 2	    /*  D Immediate		*/
#define AM_XIMM	 3	    /*  X Immediate		*/
#define AM_YIMM	 4	    /*  Y Immediate		*/
#define AM_UIMM	 5	    /*  U Immediate		*/
#define AM_SIMM	 6	    /*  S Immediate		*/
#define AM_WIMM	 7	    /*  W Immediate		*/
#define AM_XIDX	 8	    /*  X-Indexed	*/

    /* The following 3 classes do not apear in dtble.h - and never explicitly
	 * mentioned in the source.  They are here to reserve space in the
	 * Y,U,and S indexed modes, and are given names for clarity
	 */

#define AM_YIDX	 9	    /*  Y-Indexed	*/
#define AM_UIDX	10	    /*  U-Indexed	*/
#define AM_SIDX	11	    /*  S-Indexed	*/
#define AM_WIDX	12	    /*  W-Indexed	*/

	/* All the modes above _MUST_ remain in the exact order
	 * for processing in the function Domode (in cmdfile.c
	 */

#define AM_BYTI	 1	    /*  1-byte immediate	*/
#define AM_DRCT 13	    /*  Direct	*/
#define AM_EXT	14	    /*  Extended	*/
#define AM_REL	15	    /*  Relative	*/

/* Classes below don't affect addressing modes */

#define AM_REG	16	    /*  Register - no addressing	*/
#define AM_PSH  17	    /*  Psh/pul instr	*/
#define AM_TFM	18
#define AM_BIT  19

#endif
