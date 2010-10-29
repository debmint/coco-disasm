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

#include <windows.h>
#include "resource.h" 

// Initial dimensions of the main window

#define MAINWINWIDTH  750
#define MAINWINHEIGHT 540

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
 
typedef struct
{
    char *fname;
    HWND tview;
    char *tvstring;          // ptr to "tview" data for later deletion
    HWND  l_store;          //GtkListStore *l_store;
    int  *tbuf;             //GtkTextBuffer *tbuf;
    BOOL altered;
} FILEINF;

typedef struct
{
    char *progfile;            /* The program file to disassemble */
    FILEINF list_file;          /* The formatted listing file      */
    FILEINF cmdfile;            /* The os9disasm COMMAND FILE      */
    /* TODO : include the ability to see all 16 label files */
    FILEINF lblfile;            /* An os9disasm label file         */
    char *filename_to_return;  /* tmp storage for file selection */
/*	GtkWidget *fsel;*/
} glbls;

GLOBAL glbls O9Dis;

typedef struct ofile_widgets
{
    int *o_entry;  //GtkWidget *o_entry;
    int *browse_button; //GtkWidget *browse_button;
    BOOL is_dir;
    const char *dialog_ttl;
    char *fname;
    BOOL is_read;       /* For file_chooser - if TRUE, search for read, */
                            /* else search for saving                       */
} FILE_WIDGET;

GLOBAL FILE_WIDGET prog_prof
#ifdef MAIN
= {NULL, NULL, FALSE, "File to Disassemble", NULL, TRUE}
#endif
;

GLOBAL FILE_WIDGET cmd_prof
#ifdef MAIN
= {NULL, NULL, FALSE, "Command File", NULL, TRUE}
#endif
;
GLOBAL FILE_WIDGET asmout_prof
#ifdef MAIN
= {NULL, NULL, FALSE, "Asm Src File", NULL, FALSE}
#endif
;
GLOBAL FILE_WIDGET listing_prof
#ifdef MAIN
= {NULL, NULL, FALSE, "Listing Output", NULL, FALSE}
#endif
;
GLOBAL FILE_WIDGET defs_prof
#ifdef MAIN
= {NULL, NULL, FALSE, "Defs Path", NULL, TRUE}
#endif
;

GLOBAL FILE_WIDGET *prog_wdg
#ifdef MAIN
= &prog_prof
#endif
;
GLOBAL FILE_WIDGET *cmd_wdg
#ifdef MAIN
= &cmd_prof
#endif
;
GLOBAL FILE_WIDGET *asmout_wdg
#ifdef MAIN
= &asmout_prof
#endif
;
GLOBAL FILE_WIDGET *listing_wdg
#ifdef MAIN
= &listing_prof
#endif
;
GLOBAL FILE_WIDGET *defs_wdg
#ifdef MAIN
= &defs_prof
#endif
;

/* include proto.h here so that all structures can be defined */

#include "proto.h"

/* structure to pass to "response" callbacks for dialogs *
 * in dasmedit.c and amode.c                             */

struct adr_widgets {
    char *cmd_mode;
    int *label_combo, //GtkWidget *label_combo,
              *address_entry,
              *label_entry,
              *offset_entry;
};

/* os9disasm options */
GLOBAL int pgwdth;
GLOBAL int pgdpth;
GLOBAL int cputype;
GLOBAL int showzeros;
GLOBAL BOOL isrsdos;
GLOBAL int upcase;
GLOBAL char *bin_file;
GLOBAL char *cmd_cmd;
GLOBAL BOOL write_obj,
                alt_defs;
GLOBAL char *obj_file,
             *alt_defs_path;
GLOBAL int write_list;
GLOBAL char *listing_output;

//GLOBAL GtkWidget *window;       /* The main, base, window */

/* The following two are needed for define in menu.h, use in filestuff.c */
//GLOBAL GtkUIManager *ui_manager;
/*GLOBAL GtkTreeSelection *list_selection;*/
GLOBAL char *LastPath;      /* Last path - used for setting path in */
                                    /*  file chooser dialog                 */

   
