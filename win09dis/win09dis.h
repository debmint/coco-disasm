/* ************************************************************** *
 * win09dis.h - Header file for all win09dis files                $
 *                                                                $
 * $Id::                                                          $
 * ************************************************************** */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef MAIN
#  define GLOBAL
#else
#  define GLOBAL extern
#endif

#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "0.11.1"
#endif

#include <windows.h>
#include <winuser.h>
#include "resource.h" 

// Initial dimensions of the main window

#define MAINWINWIDTH  750
#define MAINWINHEIGHT 540
#define G_N_ELEMENTS(a) sizeof (a)/sizeof(a[0])

/* Label file Treeview column defs */
enum
{
    LBL_LBL = 0,
    LBL_EQU,
    LBL_ADDR,
    LBL_CLASS,
    LBL_NCOLS
};

/* Listing Treeview column defs */
enum
{
    LST_LIN = 0,
    LST_ADR,
    LST_OPCO,
    LST_PBYT,
    LST_LBL,
    LST_MNEM,
    LST_OPER,
    LST_NCOLS
};

/* Listing output destination */
enum
{
    LIST_GTK, LIST_FILE, LIST_NONE
};

/* Save_all() return values */
enum
{
    SAVALL_NONE = -20,
    SAVALL_ALL,
    SAVALL_SOME
};

enum
{
    FFT_CMD = 1,
    FFT_LBL,
    FFT_MISC
};

typedef struct
{
	HWND btn;
	int set;
} OPTSETTING;

typedef struct glist_str {
    void *data;
    struct glist_str *next;
    struct glist_str *prev;
} GList;

OPTSETTING PgWdth,
           PgDpth,
           CpuType,
           RsDos,
           ShowZeros,
           UpCase;

//typedef struct
//{
//    OPTSETTING pgwdth,
//               pgdpth,
//               cputype,
//	       rsdos,		// Flag - if set, disassemble for RS-DOS
//               showzeros,    // Flag - if set, show 0,R offsets
//               upcase;       // Flag - if set, display output in upper case
//	  // The following may not be needed, as each has a FILEINF
//	  // structure but we'll leave them here for now anyway
//    char *binfile,      // Ptr to name of module to disassemble
//         *odiscmd,      // Ptr to path to win09dis cmd file
//         *objfile,      // Ptr to .asm filename to output (NULL if none)
//         *altdefs,      // Ptr to path to alt DEFS directory (NULL if none)
//         *listingpath;  // Ptr to pathname of listing (NULL if none)
//} OPTSTBL;

typedef struct
{
    char *fname;
    const char *dialog_ttl;
    HWND o_entry,  //GtkWidget *o_entry;
         browse_button, //GtkWidget *browse_button;
         tview,
         l_store;      // GtkListStore *l_store;
    char *tvstring;     // ptr to "tview" data for later deletion
    int  *tbuf;         // GtkTextBuffer *tbuf;
    BOOL altered;
    BOOL is_read;       /* For file_chooser - if TRUE, search for read, */
    BOOL is_dir;
} FILEINF;

typedef struct
{
    FILEINF binfile,        // The binary file to disassemble
            list_file,      // The formatted listing file
            cmdfile,        // The os9disasm COMMAND FILE
            lblfile,        // An os9disasm label file
            asmout,         // The assembly source file to output (optional)
            defsfile,       // The alternate DEFS path (optional)
            list_out;       // Filename for listing output (if specified)
    char *filename_to_return;  /* tmp storage for file selection */
/*	GtkWidget *fsel;*/
} glbls;

GLOBAL glbls O9Dis;
GLOBAL HWND ToolTipControl;

/* structure to pass to "response" callbacks for dialogs *
 * in dasmedit.c and amode.c                             */

struct adr_widgets {
    char *cmd_mode;
    int *label_combo, //GtkWidget *label_combo,
              *address_entry,
              *label_entry,
              *offset_entry;
};

GLOBAL int write_list;

/* The following two are needed for define in menu.h, use in filestuff.c */

GLOBAL char *LastPath;      /* Last path - used for setting path in */
                                    /*  file chooser dialog                 */

   
