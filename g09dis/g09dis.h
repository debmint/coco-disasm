/* ********************************** *
 * common header file for all modules *
 * ********************************** */

/* $Id$ */

#ifdef MAIN
#	define GLOBAL
#else
#	define GLOBAL extern
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <gtk/gtk.h>

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
    
/* The three work-area windows */
GLOBAL GtkWidget *list_win;
GLOBAL GtkWidget *cmd_win;
GLOBAL GtkWidget *lbl_win;

typedef struct
{
    gchar *fname;
    GtkWidget *tview;
    GString *tvstring;          /* ptr to "tview" data for later deletion */
    GtkListStore *l_store;
    GtkTextBuffer *tbuf;
    gboolean altered;
} fileinf;

typedef struct
{
    gchar *progfile;            /* The program file to disassemble */
    fileinf list_file;          /* The formatted listing file      */
    fileinf cmdfile;            /* The os9disasm COMMAND FILE      */
    /* TODO : include the ability to see all 16 label files */
    fileinf lblfile;            /* An os9disasm label file         */
    gchar *filename_to_return;  /* tmp storage for file selection */
/*	GtkWidget *fsel;*/
} glbls;

GLOBAL glbls O9Dis;

/* structure to pass to "response" callbacks for dialogs *
 * in dasmedit.c and amode.c                             */

struct adr_widgets {
    gchar *cmd_mode;
    GtkWidget *label_combo,
              *address_entry,
              *label_entry,
              *offset_entry;
};

/* os9disasm options */
GLOBAL gint pgwdth;
GLOBAL gint pgdpth;
GLOBAL gint cputype;
GLOBAL gint showzeros;
GLOBAL gboolean isrsdos;
GLOBAL gint upcase;
GLOBAL gchar *bin_file;
GLOBAL gchar *cmd_cmd;
GLOBAL gboolean write_obj,
                alt_defs;
GLOBAL gchar *obj_file,
             *alt_defs_path;
GLOBAL gint write_list;
GLOBAL gchar *listing_output;

GLOBAL GtkWidget *window;       /* The main, base, window */

/* The following two are needed for define in menu.h, use in filestuff.c */
GLOBAL GtkUIManager *ui_manager;
/*GLOBAL GtkTreeSelection *list_selection;*/

/* This needs to be down here so that some defs above can be used */

#include "proto.h"

