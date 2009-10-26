/* ******************************************************************** *
 * filestuff.c - handles file I/O, etc                                  $
 * $Id::                                                                $
 * ******************************************************************** */


#include "g09dis.h"
#include <stdio.h>
#include <string.h>
/*#include <unistd.h>*/
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_LIBGEN_H
#   include <libgen.h>
#endif

#ifdef WIN32
#define DIRSTR "\\"
#define DIRCHR 92
#else
#define DIRSTR "/"
#define DIRCHR '/'
#endif

#define OPT_BIN "BIN-FILE:"
#define OPT_CMD "CMD-FILE:"
#define OPT_LBL "LBL-FILE:"
#define OPT_DEF "ALT-DEFS:"
#define OPT_OBJ "SRC-FILE:"
#define OPT_RS  "RS-FILE:"
#define OPT_CPU "CPU:"
#define OPT_UPC "UPCASE:"
#define OPT_PGW "PGWDTH:"
#define OPT_PGD "PGDPTH:"
#define OPT_LST "LISTING:"

/* *******************************************
 * File selection stuff
 * *******************************************
 */

#ifndef HAVE_DIRNAME

/* ******************************************************** *
 * dirname() - a replacement for the libgen dirname.        *
 *      Note:  This function, like the original, mangles    *
 *              the original string, so in most cases, a    *
 *              dup of the original should be passed        *
 * ******************************************************** */

char *
dirname (char *path)
{
    char *slash = strrchr (path, '/');

    if (slash)
    {
        *slash = '\0';
    }
    else
    {
        strcpy (path, ".");
    }

    return path;
}
#endif

/* *********************************************
 * ending_slash() append a "/" to the end of a *
 * directory pathname if not already there     *
 * *********************************************/

gchar *
ending_slash (const gchar * dirnm)
{
    if (!dirnm)
    {
        return g_strdup ("");
    }

    if (dirnm[strlen (dirnm) - 1] == DIRCHR)
    {
        return g_strdup (dirnm);
    }
    else
    {
        return g_strconcat (dirnm, DIRSTR, NULL);
    }
}

/* **************************************************************** *
 * set_fname() - Generic filename setup, called by the following    *
 *               file-selection callbacks.                          *
 *               Checks that filename is present in                 *
 *               hbuf->filename_to_return, and if so, clears and    *
 *               resets the filename ptr passed as a reference in   *
 *               "flag".                                            *
 * Passed : (1) - Ptr to the glbls structure                        *
 *          (2) - Pointer to the appropriate filename ptr           *
 * **************************************************************** */

void
set_fname (glbls *hbuf, gchar **flag)
{
    if (hbuf->filename_to_return)
    {
        if (strlen(hbuf->filename_to_return))
        {
            if (*flag != NULL)
            {
                g_free(*flag);
            }

            *flag = g_strdup(hbuf->filename_to_return);
        }

        g_free(hbuf->filename_to_return);
        hbuf->filename_to_return = NULL;
    }
}

/* Returns column number or -1 if not found or on error */

gint
get_col_number_from_tree_view_column (GtkTreeViewColumn *col)
{
    GList *cols;
    gint   num;

    g_return_val_if_fail ( col != NULL, -1 );
    g_return_val_if_fail ( col->tree_view != NULL, -1 );

    cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(col->tree_view));

    num = g_list_index(cols, (gpointer) col);

    g_list_free(cols);

    return num;
}

/* Callback for click on a row in the Listing display */

gboolean
onListRowButtonPress (GtkTreeView *treevue, GdkEventButton *event,
                      gchar *menu_name)
{
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(treevue));
   
    /* single click with the right mouse button? */
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3)
    {
        GtkWidget *popup;
        
        /* optional: select row if no row is selected or only
         * one other row is selected (will only do something
         * if you set a tree selection mode as described later
         * in the tutorial) */

        /* Note: gtk_tree_selection_count_selected_rows() does not
         * exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
        if (gtk_tree_selection_count_selected_rows(selection)  <= 1)
        {
            GtkTreePath *path;

            /* Get tree path for row that was clicked */
            if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treevue),
                                              event->x, event->y,
                                              &path, NULL, NULL, NULL))
            {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
                gtk_tree_path_free(path);
            }
        } /* end of optional bit */

        /*if( ! gtk_tree_selection_get_selected (list_selection,
                                               NULL, NULL))
        {*/
            /* No row selected ... return */
           /* return FALSE;
        }*/
        
        /* do pop-up window */
        popup = gtk_ui_manager_get_widget (ui_manager, menu_name);

        gtk_menu_popup (GTK_MENU(popup), NULL, NULL,NULL, NULL,
                        0, gdk_event_get_time((GdkEvent*)event));
        return TRUE;
    }
    
    /* we did nothing */
    return FALSE;
}

/* ******************************************************************** *
 * set_chooser_folder() - Sets the folder path in the GtkFileChooser    *
 *               if appropriate.                                        *
 * Passed:  (1) - GtkFileChooser in which to set folder                 *
 *          (2) - glbls * hbuf in which to search for paths             *
 * ******************************************************************** */

void
set_chooser_folder(GtkFileChooser *chooser, glbls *hbuf)
{
    gchar *path;

    path = NULL;
    
    /* Try to guess a suitable path */

    if (cmd_wdg->fname)
    {
        path = cmd_wdg->fname;
    }
    else if ((hbuf->cmdfile).fname)
    {
        path = (hbuf->cmdfile).fname;
    }
    else if ((hbuf->lblfile).fname)
    {
        path = (hbuf->lblfile).fname;
    }
    else if (prog_wdg->fname)
    {
        path = prog_wdg->fname;
    }
    else if (LastPath)
    {
        path = LastPath;
    }

    if (path)
    {
        /* dup path - dirname trashes the original string */

        path = dirname (g_strdup(path));
        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER(chooser), path);
        g_free (path);
    }
}

/* **************************************************** *
 * selectfile_open() - Select a file to open            *
 * Passed:  (1) - ptr to appropriate glbls struct       *
 *          (2) - Type (string for prompt               *
 *          (3) - Boolean TRUE if file, FALSE if dir    *
 *          (4) - Filename to display or NULL           *
 * Runs gtk_file_chooser_dialog                         *
 * Sets hbuf->filename_to_return to selected file       *
 * **************************************************** */

void
selectfile_open ( glbls *hbuf,
                  const gchar *type,
                  gboolean IsFile,
                  const gchar *fnam)
{
    GtkWidget *fsel;
    gchar *title = g_strconcat ("Select a ", type, NULL);
/*	gchar     *utf8 = g_locale_to_utf8( title, -1, NULL, NULL, NULL );*/
    gint action_type;

    if (IsFile)
    {
        action_type = GTK_FILE_CHOOSER_ACTION_OPEN;
    }
    else
    {
        action_type = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    }

    hbuf->filename_to_return = NULL;

    fsel = gtk_file_chooser_dialog_new (title,
                                        GTK_WINDOW (window),
                                        action_type,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);    /* Leak??? */

    g_free (title);
/*	g_free( utf8 );*/

/*	g_signal_connect (G_OBJECT (fsel), "destroy",
			G_CALLBACK (gtk_main_quit), NULL);*/

    if ((fnam && strlen(fnam)))
    {
        gchar *base_name = (gchar *)g_path_get_basename (fnam),
              * dir_name = (gchar *)g_path_get_dirname (fnam);

        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(fsel), base_name);

        if (g_path_is_absolute (dir_name))
        {
            gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(fsel),
                                                 dir_name);
        }

        g_free (base_name);
        g_free (dir_name);
    }
    else
    {
        set_chooser_folder (GTK_FILE_CHOOSER(fsel), hbuf);
    }

    if (gtk_dialog_run (GTK_DIALOG (fsel)) == GTK_RESPONSE_ACCEPT)
    {

        hbuf->filename_to_return =
            gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fsel));
    }

    gtk_widget_destroy (fsel);
}

/* ********************* *
 * select a file to save *
 * ********************* */

void
selectfile_save (glbls *hbuf, const gchar *cur_name, const gchar *type)
{
    GtkWidget *fsel;
    gchar *title = g_strconcat ("Select a ", type, NULL);
/*	gchar     *utf8 = g_locale_to_utf8( title, -1, NULL, NULL, NULL );*/

    hbuf->filename_to_return = NULL;

    fsel = gtk_file_chooser_dialog_new (title,
                                        GTK_WINDOW (window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_SAVE,
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);    /* Leak??? */

    g_free (title);
/*	g_free( utf8 );*/

    if(cur_name)
    {
        gchar *base_name = (gchar *)g_path_get_basename (cur_name),
              * dir_name = (gchar *)g_path_get_dirname (cur_name);

        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fsel), base_name);
        //gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(fsel), cur_name);

        if (g_path_is_absolute (dir_name))
        {
            gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(fsel),
                                                 dir_name);
        }

        g_free (base_name);
        g_free (dir_name);
    }

/*	g_signal_connect (G_OBJECT (fsel), "destroy",
			G_CALLBACK (gtk_main_quit), NULL);*/

    if (gtk_dialog_run (GTK_DIALOG (fsel)) == GTK_RESPONSE_ACCEPT)
    {

        hbuf->filename_to_return =
            gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fsel));
    }

    gtk_widget_destroy (fsel);
}

/* ****************************** *
 * Text Buffer handling stuff     *
 * ****************************** */

/* ******************************************** *
 * save_all_query() - notify user that multiple *
 * files (might) need saving returns choice of  *
 * NONE, ALL, or USER SELECT                    *
 * ******************************************** */

gint
save_all_query()
{
    GtkWidget *dialog;
    gint result;

    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_MODAL |
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_NONE,
                                    "You are about to exit and your files have been altered\nWhat do you wish to do?"); 

    gtk_dialog_add_buttons (GTK_DIALOG(dialog),
                            "Exit without saving", SAVALL_NONE,
                            "Save All", SAVALL_ALL,
                            "Select files", SAVALL_SOME,
                            NULL);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return result;
}

/* ***************************************************** *
 * save_warn_OW() - Alert user that a single altered but *
 *     not-saved buffer is about to be overwritten       *
 * ***************************************************** */

gint
save_warn_OW (gchar *type, gchar *fnam, gboolean can_cancel)
{
    GtkWidget *dialog;
    gint response;
    gchar *fn;

    if( fnam)
    {
        fn = fnam;
    }
    else {
        fn = "(Untitled)";
    }
    
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_MODAL |
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_NONE,
                                    "About to discard altered %s file buffer\nDo you wish to save to %s\n", type, fn); 

    gtk_dialog_add_button (GTK_DIALOG(dialog),
                           "Discard Changes", GTK_RESPONSE_NO);
    if(can_cancel) {
        gtk_dialog_add_button (GTK_DIALOG(dialog),
                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
    }
    gtk_dialog_add_button (GTK_DIALOG(dialog),
                           "Save Changes", GTK_RESPONSE_YES);
    
    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return response;
}

/* ************************* *
 * save_lbl() : save a label *
 *        buffer to a file   *
 * ************************* */

static void
save_lbl(fileinf *fdat, GtkWidget *my_win, gchar **newfile)
{
    FILE *outfile;
    GtkTreeModel *model;
    GtkTreeIter iter;

    /* TODO: rename file if it exists? */

    if (!(outfile = fopen (*newfile, "wb")))
    {
        fprintf (stderr, "Cannot open file %s for write!\n", *newfile);
    }

    /* set up buffer to write */
    model= gtk_tree_view_get_model (GTK_TREE_VIEW(O9Dis.lblfile.tview));
    
    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gchar *label, *addr, *mode;
        do {
            gtk_tree_model_get (model, &iter,
                                LBL_LBL, &label,
                                LBL_ADDR, &addr,
                                LBL_CLASS, &mode,
                                -1);
            if (*label != '*')
            {
                fprintf (outfile, "%s equ %s %c\n", label, addr, *mode);
            }
            else {  /* do differently for comment */
                fprintf (outfile, "%s %s\n", label, mode);
            }
            
            g_free(label); g_free(addr), g_free(mode);
        } while (gtk_tree_model_iter_next(model, &iter));
    }

    doc_set_modified(fdat, FALSE);
    
    fclose (outfile);
}

    /* ************************************ *
     * This function clears out the data in *
     * an existing liststore                *
     * ************************************ */

void
list_store_empty( fileinf *fdat)
{
    /* If data currently there and has been altered, save it?
     * NOTE: This function is called by both label and listing,
     * but for the listing, fdat->altered should NEVER be set
     * (we don't save the listing
     */

    if (fdat->altered == TRUE)
    {
        switch(save_warn_OW ("Label", fdat->fname, TRUE)) {
            case GTK_RESPONSE_NO: /* discard changes */
                break;
            case GTK_RESPONSE_CANCEL: /* cancel overwrite */
                return;
                break;
            case GTK_RESPONSE_YES: /* save changes */
                /* Note: in calling save_lbl, pass GtkWidget *window
                 * as a dummy - it won't be needed */
                save_lbl(fdat, window, &fdat->fname);
                break;
        }
    }

    /* clear out current list_store */

    if (fdat->fname != NULL)
    {
        gtk_list_store_clear (fdat->l_store);
        fdat->fname = NULL;

        doc_set_modified(fdat, FALSE);

        g_free (fdat->fname);
    }
}

/* ************************* *
 * save_text() : save a text *
 *          buffer to a file *
 * ************************* */

static void
save_text (fileinf * fdat, GtkWidget * my_win, gchar ** newfile)
{
    FILE *outfile;
    gchar *buf;
    GtkTextIter begin, end;

    /*fdat->fname = g_strdup (*newfile);
    g_free (*newfile);
    *newfile = NULL;*/
    
    /* TODO: rename file if it exists? */

    if ( ! (outfile = fopen (*newfile, "wb")))
    {
        fprintf (stderr, "Cannot open file %s for write!\n", *newfile);
    }

    /* set up buffer to write */

    gtk_text_buffer_get_start_iter (fdat->tbuf, &begin);
    gtk_text_buffer_get_end_iter (fdat->tbuf, &end);
    buf = gtk_text_buffer_get_text (fdat->tbuf, &begin, &end, FALSE);
    doc_set_modified(fdat, FALSE);

    if ((fputs (buf,outfile)) == EOF)
    {
        GtkWidget *dialog;
        
        dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         "Error writing data to \"%s\"",
                                            *newfile);
        gtk_dialog_run (GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);

        g_free(buf);
    }

    doc_set_modified(fdat, FALSE);

    fclose (outfile);
}

        /* ************************************** *
         * Clears data in an existing text buffer *
         * ************************************** */

void
clear_text_buf( fileinf *fdat)
{

    /* If data currently there and has been altered, save it? */
    if (fdat->altered == TRUE)
    {
        switch(save_warn_OW ("Command", fdat->fname, TRUE)) {
            case GTK_RESPONSE_NO: /* discard changes */
                break;
            case GTK_RESPONSE_CANCEL: /* cancel overwrite */
                return;
                break;
            case GTK_RESPONSE_YES: /* save changes */
                /* Note: in calling save_lbl, pass GtkWidget *window
                 * as a dummy - it won't be needed */
                save_text(fdat, window, &fdat->fname);
                break;
        }
    }

    doc_set_modified(fdat,FALSE);

    /* free up any memory previously allocated (if any) */
    if (fdat->fname != NULL)
    {
        GtkTextIter start, end;

        g_free (fdat->fname);
        fdat->fname = NULL;
        
        /* Segmentation fault below??? */
/*		g_free( fdat->tbuf );*/
        /* clear out all text */
        gtk_text_buffer_get_start_iter (fdat->tbuf, &start);
        gtk_text_buffer_get_end_iter (fdat->tbuf, &end);
        gtk_text_buffer_delete (fdat->tbuf, &start, &end);

        g_string_free (fdat->tvstring, TRUE);
        fdat->tvstring = NULL;
        /* destroy and recreate the text buffer */
        /*g_object_unref( fdat->tbuf ); */
    }
}

/* ************************* *
 * load_text() : load a file *
 *        into a text buffer *
 * ************************* */

void
load_text (fileinf * fdat, GtkWidget * my_win, gchar ** newfile)
{
    register FILE *infile;
    gchar buffer[500];
    gchar *strpt;

    clear_text_buf(fdat);

/*	fdat->tbuf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer ( GTK_TEXT_VIEW(fdat->tview), fdat->tbuf );*/
    fdat->fname = g_strdup (*newfile);
    g_free (*newfile);          /* filename_to_return */
    *newfile = NULL;

    /* Now open the file and read it */

    if (!(infile = fopen (fdat->fname, "rb")))
    {
        fprintf (stderr, "Cannot open file %s for read!\n", fdat->fname);
    }


    fdat->tvstring = g_string_new (NULL);

    while (fgets (buffer, sizeof (buffer) - 1, infile))
    {
        g_string_append (fdat->tvstring, buffer);
    }

    fclose (infile);

    strpt = fdat->tvstring->str;
    /*g_string_free(string,FALSE); */

    gtk_text_buffer_set_text (fdat->tbuf, strpt, -1);
    
    doc_set_modified(fdat, FALSE);
}

/* ****************************************************** *
 * str_digit() - check that a string is a valid deximal # *
 * Passed:  str - pointer to string to check              *
 * Returns: 1 if all chars in string match,               *
 *          0 if any do not                               *
 * ****************************************************** */

int
str_digit (char *str)
{
    char *ch = str;

    while (*ch)
    {
        if (!isdigit (*(ch++)))
        {
            return (0);
        }
    }

    return (1);     /* If we get here, all chars have matched */
}

/* ****************************************************** *
 * str_xdigit() - check that a string is a valid hex #    *
 * Passed:  str - pointer to string to check              *
 * Returns: 1 if all chars in string match,               *
 *          0 if any do not                               *
 * ****************************************************** */

int
str_xdigit (char *str)
{
    char *ch = str;

    while (*ch)
    {
        if (!isxdigit (*(ch++)))
        {
            return (0);
        }
    }

    return (1);     /* If we get here, all chars have matched */
}

/* ****************************************************** *
 * load_list_tree() - generic single-column Treeview load *
 * ****************************************************** */

static void
load_list_tree (fileinf * fdat, FILE *infile)
{
    gchar buffer[500];

    while (fgets (buffer, sizeof (buffer), infile))
    {
        gchar **splits = NULL;
        GtkTreeIter iter;

        /* get rid of newlines and leading/trailing whitespaces,
         * if present
         */

        g_strstrip(buffer);
        
        if (strlen (buffer))
        {
            /* Assure that there are enough fields */

            strcat (buffer,"\t\t\t\t\t\t");

            /* Split the string, with all excess going into splits[LS_NCOLS] */

            splits = g_strsplit (buffer, "\t", LST_NCOLS + 1);

            if (!str_digit (splits[LST_LIN]) || !str_xdigit (splits[LST_ADR]))
            {
                continue;
            }

            gtk_list_store_append (fdat->l_store, &iter);
            gtk_list_store_set (fdat->l_store, &iter,
                                LST_LIN, splits[LST_LIN],
                                LST_ADR, splits[LST_ADR],
                                LST_OPCO, splits[LST_OPCO],
                                LST_PBYT, splits[LST_PBYT],
                                LST_LBL, splits[LST_LBL],
                                LST_MNEM, splits[LST_MNEM],
                                LST_OPER, splits[LST_OPER],
                                -1);

            g_strfreev (splits);
            splits = NULL;
        }
    
        doc_set_modified(fdat,FALSE);
    }
}

/* load_lbl() - load the label file into the GtkTreeView */

void
load_lbl (fileinf * fdat, GtkWidget * my_win, gchar ** newfile)
{
    FILE *infile;
    gchar buffer[500];

    list_store_empty(fdat);

    fdat->fname = g_strdup (*newfile);
    g_free (*newfile);
    *newfile = NULL;

    /* Now open the file and read it */
    if (!(infile = fopen (fdat->fname, "rb")))
    {
        fprintf (stderr, "Cannot open file %s for read!\n", fdat->fname);
    }

    while (fgets (buffer, 160, infile))
    {
        gchar **splits;

        /* get rid of newline, leading/trailing whitespaces
         * if present
         */
        g_strstrip(buffer);

        if ((strlen (buffer)) && (*buffer != ' '))
        {
            if (*buffer != '*')
            {
                gchar *tmppt;
                gint bcnt;

                /* convert tabs to spaces  
                 * if file is saved, spaces will be used */
               
                while ((tmppt = strchr(buffer, '\t')))
                {
                    *tmppt = ' ';
                }
                
                tmppt = buffer;

                for (bcnt = 1; bcnt<4; ++bcnt)
                {
                    tmppt = strchr(tmppt, ' ');

                    if (tmppt)
                    {
                        ++tmppt;
                        g_strchug (tmppt);
                    }
                }
                splits = g_strsplit (buffer, " ", LBL_NCOLS);
            }
            else   /* A comment line */
            {
                GString * tmpstr = NULL;
                int sspos = 1;  /* Default separator to second position */
                
                if (buffer[1] == '\\')
                {
                    ++sspos;   /* Bump separator past '\' */
                    
                    /* AMode def - add to AMode list */
                    amode_add_from_string (&buffer[sspos]);
                }

                tmpstr = g_string_new_len (buffer, sspos);

                if ((!isspace (buffer[sspos])) && (buffer[sspos] != '\t'))
                {
                    g_string_append (tmpstr, " ");
                }
                
                tmpstr = g_string_append (tmpstr, &buffer[sspos]);
                
                splits = g_strsplit (tmpstr->str, " ", 2);

                g_string_free (tmpstr, TRUE);
            }

            /* Now add to ListStore */

            if ((splits) )
            {
                GtkTreeIter iter;
                gint lpos = 0;

                gtk_list_store_append (fdat->l_store, &iter);

                if (*splits[0] != '*')  /* Label definition */
                {
                    while (splits[lpos] != NULL)
                    {
                        gtk_list_store_set (fdat->l_store, &iter,
                                            lpos, splits[lpos], -1);
                        ++lpos;
                    }
                }
                else   /* else a comment */
                {
                    gtk_list_store_set (fdat->l_store, &iter,
                                        LBL_LBL, splits[0],
                                        LBL_EQU, "", LBL_ADDR, "",
                                        LBL_CLASS, splits[1],
                                        -1);
                }
            }
            g_strfreev (splits);
        }
    }

    doc_set_modified( fdat, FALSE);

    fclose (infile);
}

/* **************************************** *
 * sysfailed() - report failure of system() *
 *     call                                 *
 * **************************************** */

void
sysfailed (char *msg)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                     GTK_DIALOG_DESTROY_WITH_PARENT ||
                                         GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                     msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


/* **************************************************** *
 * run_disassembler  - a callback from the menu does a  *
 *                     disassembly pass                 *
 * Passed: action, the global variables                 *
 * **************************************************** */

void
run_disassembler (GtkAction * action, glbls * hbuf)
{
    GString *CmdL;
    gchar *cmdline;
    gboolean PipeIt = FALSE;

    CmdL = g_string_new("os9disasm ");

    /* We MUST have a binary file to disassemble */
    
    if( ! prog_wdg->fname )
    {
        GtkWidget *warnwin;
        gchar *msg;

        msg = g_strconcat("WARNING!\n",
                          "No program file specified\n",
                          "Please configure in menu item \"Options\"\n",
                          NULL );
        warnwin = gtk_message_dialog_new( GTK_WINDOW(window),
                                          GTK_DIALOG_DESTROY_WITH_PARENT ||
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                          msg );
        gtk_dialog_run (GTK_DIALOG(warnwin));
        gtk_widget_destroy (warnwin);
        g_string_free (CmdL,TRUE);
        return;
    }
    else {
        g_string_append (CmdL,prog_wdg->fname);
    }

    if (cmd_wdg->fname)
    {
        g_string_append (CmdL, " -c=");
        g_string_append (CmdL, cmd_wdg->fname);
    }

    if (alt_defs)
    {
        g_string_append_printf (CmdL, " -d=%s", defs_wdg->fname);
    }
   
    if (isrsdos)
    {
        g_string_append (CmdL, " -x=c");
    }
    if (write_obj)
    {
        if (asmout_wdg->fname)
        {
            if (strlen(asmout_wdg->fname))
            {
                g_string_append (CmdL, " -o=");
                g_string_append (CmdL, asmout_wdg->fname);
            }
        }
    }
    
    if( cputype )
    {
        g_string_append (CmdL, " -3");
    }

    if (upcase)
    {
        g_string_append (CmdL, " -u");
    }

    if (showzeros)
    {
        g_string_append (CmdL, " -z");
    }

    if (pgwdth)
    {
        g_string_append_printf (CmdL, " -pw=%d", pgwdth);
    }

    if (pgdpth)
    {
        g_string_append_printf (CmdL, " -pd=%d", pgdpth);
    }


    switch (write_list) {
        case LIST_FILE:
            if (listing_wdg->fname)
            {
                if (strlen(listing_wdg->fname))
                {
                    g_string_append_printf (CmdL, " > %s", listing_wdg->fname);
                }
            }
            break;
        case LIST_NONE:
            /* handle bit-bucket output */
            /* g_string_append (CmdL, "-q");*/
            break;
        case LIST_GTK:
            g_string_append (CmdL, " -g");
            PipeIt = TRUE;
            break;
    }

    cmdline = g_string_free (CmdL, FALSE);
    
    if ( ! (PipeIt))   /* Output is to stdout or a file */
    {
        int retval;
        char *msg = NULL;
        
        switch (retval = system (cmdline))
        {
            case 0: /* sucess */
                break;
            case -1:  /* failed fork */
                msg = "Could not fork \"os9disasm\"\nIs \"os9disasm\" in your path?";
                sysfailed(msg);
                break;
            default:
                msg = "Error in executing \"os9disasm\"\nPlease check console for error messages";
                sysfailed(msg);
        }
    }
    else        /* else we're piping to the GUI */
    {
        FILE *infile;
        
        /* Now open the file and read it */
        if (!(infile = popen (cmdline, "r")))
        {
            g_print ("Cannot pipe \"%s\" for read!\n",
                              cmdline);
        }
        else {
            gtk_list_store_clear ((hbuf->list_file).l_store);
            load_list_tree (&hbuf->list_file, infile);
            pclose (infile);
        }
    }

    g_free (cmdline);
}

/* Disassemble listing to file */

void
dasm_list_to_file_cb (GtkAction *action, glbls *hbuf)
{
    gint old_write = write_list;

    selectfile_save (hbuf, listing_wdg->fname, "File for Listing");
    set_fname (hbuf, &listing_wdg->fname);
    write_list = LIST_FILE;
    run_disassembler (action, hbuf);
    write_list = old_write;
}

/* Clean up */

void
free_filename_to_return (gchar ** fname)
{
    if (*fname != NULL)
    {
        g_free (*fname);
        *fname = NULL;
    }
}

/* ******************************************** *
 * Callbacks for file selection                 *
 * Passed:  (1) GtkAction *action,              *
 *          (2) address of global data pointer  *
 * ******************************************** */

void
compile_listing (GtkAction * action, glbls * hbuf)
{
    int old_write_list = write_list;

    /* temporarily flag that we're writing to GUI */
    
    write_list = LIST_GTK;

    run_disassembler (action, hbuf);

    write_list = old_write_list;        /* restore write_list */
}

/* ******************************************************** *
 * load_listing() - A callback from the menu(s).            *
 *          This is the "top-level" function for loading    *
 *          a listing                                       *
 * ******************************************************** */

void
load_listing (GtkAction * action, glbls * hbuf)
{
    FILE *infile;

    selectfile_open (hbuf, "Prog Listing", TRUE, NULL);

    if (hbuf->filename_to_return != NULL)
    {                           /* Leak? */
        /* Call list_store_empty - if it's a listing, fdat->altered shouldn't
         * be set to TRUE (hopefully)
         */
        
        list_store_empty(&(hbuf->list_file));

        (hbuf->list_file).fname = g_strdup (hbuf->filename_to_return);
        free_filename_to_return (&(hbuf->filename_to_return));

        /* Now open the file and read it */
        
        if (!(infile = fopen ((hbuf->list_file).fname, "r")))
        {
            fprintf (stderr, "Cannot open file \"%s\" for read!\n",
                              (hbuf->list_file).fname);
        }
        else {
            load_list_tree (&hbuf->list_file, infile);
            fclose (infile);
        }
    }
}

void
do_cmdfileload (glbls * hbuf)
{
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {
        load_text (&hbuf->cmdfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
    gtk_text_buffer_set_modified ((hbuf->cmdfile).tbuf, FALSE);
}

void
load_cmdfile (GtkAction * action, glbls * hbuf)
{
    selectfile_open (hbuf, "Command file", TRUE, cmd_wdg->fname);
    do_cmdfileload (hbuf);
}

void
do_lblfileload (glbls * hbuf)
{
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {
        load_lbl (&hbuf->lblfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
}

void
load_lblfile (GtkAction * action, glbls * hbuf)
{
    selectfile_open (hbuf, "Label File", TRUE, NULL);
    do_lblfileload (hbuf);
    
}

/* ************************************************************ *
 * File Save callbacks                                          *
 * These that follow are the top-level entry points for these   *
 * routines - they call others to do the "manual labor"         *
 * ************************************************************ */

void
cmd_save_as (GtkAction * action, glbls * hbuf)
{
    selectfile_save(hbuf, hbuf->cmdfile.fname, "Command File");
    
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {

        /* handle fname */
        
        if (hbuf->cmdfile.fname)
        {
            g_free(hbuf->cmdfile.fname);
        }

        hbuf->cmdfile.fname = g_strdup(hbuf->filename_to_return);
        
        /* save cmd file */
        save_text (&hbuf->cmdfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
}

void
cmd_save(GtkAction *action, glbls *hbuf)
{
    if (!(hbuf->cmdfile.fname))
    {
        cmd_save_as(action, hbuf);
    }
    else {
        save_text (&hbuf->cmdfile, list_win, &(hbuf->cmdfile.fname));
    }

    gtk_text_buffer_set_modified ((hbuf->cmdfile).tbuf, FALSE);
}

void
lbl_save_as (GtkAction * action, glbls * hbuf)
{
    selectfile_save(hbuf, hbuf->lblfile.fname, "Label File");
    
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {
        if (hbuf->lblfile.fname)
        {
            g_free(hbuf->lblfile.fname);
        }

        hbuf->lblfile.fname = g_strdup(hbuf->filename_to_return);
        
        /* save label file */
        save_lbl (&hbuf->lblfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
}

void
lbl_save (GtkAction *action, glbls *hbuf)
{
    if( !(hbuf->lblfile.fname))
    {
        lbl_save_as(action, hbuf);
    }
    else {
        save_lbl (&hbuf->lblfile, list_win, &(hbuf->lblfile.fname));
    }
}

/* **************************************************** *
 * Options load/save callbacks                          *
 * **************************************************** */

/* ******************************************************** *
 * opt_put() - check for state of variable ptr and if set,  *
 *          write to optsave file                           *
 * ******************************************************** */

static void
opt_put (gboolean isset, char *nam, char *hdr, FILE *fp)
{
    gchar *line;
    
    if (isset)
    {
        line = g_strconcat (hdr, nam, "\n", NULL);
        fputs (line, fp);
        g_free (line);
    }
}

static void
opt_load_path (gchar **pthptr, gchar *pthnm, gboolean *flag, gchar* hdr)
{
    char *start = pthnm;

    start += strlen (hdr);
    
    if (flag)
    {
        *flag = TRUE;
    }

    g_strstrip (start);
    *pthptr = g_strdup (start);
}

/* **************************************************************** *
 * opt_get_val() - check for a match in the HDR and store the value *
 *          of matched                                              *
 * PASSED: hdr string to check against buffer holding the loaded    *
 *          line address to store value if matched                  *
 * Returns: value if matched, NULL if no match                      *
 * **************************************************************** */

static gint
opt_get_val ( gchar *ctrl, gchar *buf, gint *addr)
{
    gchar *start = buf;

    if (!strncmp (ctrl, buf, strlen (ctrl)))
    {
        start += strlen (ctrl);
        g_strstrip (start);
        sscanf (start, "%d", addr);
        return *addr;
    }

    return 0;
}

void
opts_save (GtkAction *action, glbls *hbuf)
{
    FILE *optfile;

    selectfile_save (hbuf, NULL, "Options File");
    optfile = fopen(hbuf->filename_to_return, "rb");
    
    if (!(optfile = fopen (hbuf->filename_to_return, "wb")))
    {
        /* Report Error */
        free_filename_to_return ( &(hbuf->filename_to_return));
        return;
    }
    
    free_filename_to_return ( &(hbuf->filename_to_return));

    opt_put ((gboolean)prog_wdg->fname, prog_wdg->fname, OPT_BIN, optfile);
    opt_put ((gboolean)cmd_wdg->fname, cmd_wdg->fname, OPT_CMD, optfile);
    opt_put ((gboolean)hbuf->lblfile.fname, hbuf->lblfile.fname, OPT_LBL,
            optfile);
    opt_put (alt_defs, defs_wdg->fname, OPT_DEF, optfile);
    opt_put (write_obj, asmout_wdg->fname, OPT_OBJ, optfile);
    fprintf (optfile, "%s%d\n", OPT_RS, (int)isrsdos);
    fprintf (optfile, "%s%d\n", OPT_CPU, (int)cputype);
    fprintf (optfile, "%s%d\n", OPT_UPC, (int)upcase);
    fprintf (optfile, "%s%d\n", OPT_PGW, (int)pgwdth);
    fprintf (optfile, "%s%d\n", OPT_PGD, (int)pgdpth);
    fprintf (optfile, "%s%d ", OPT_LST, (int)write_list);
    
    switch (write_list)
    {
        case LIST_FILE:
            if (listing_wdg->fname)
            {
                if (strlen (listing_wdg->fname))
                {
                    fprintf (optfile, "%s\n", listing_wdg->fname);
                }
            }
            break;
        default:
            fputc('\n', optfile);
    }

    fclose (optfile);
}

void opts_load (GtkAction *action, glbls *hbuf)
{
    FILE *optfile;
    gchar buf[200];
    
    selectfile_open (hbuf, "n Options File", TRUE, NULL);
    optfile = fopen(hbuf->filename_to_return, "rb");
    
    if ( ! optfile)
    {
        /* Print Error message */
        
        free_filename_to_return ( &(hbuf->filename_to_return));
        return;
    }
    
    free_filename_to_return ( &(hbuf->filename_to_return));

    while (fgets (buf, 200, optfile))
    {
        if ( ! strncmp (OPT_BIN, buf, strlen(OPT_BIN)))
        {
            opt_load_path (&(prog_wdg->fname), buf, NULL, OPT_BIN);
            menu_do_dis_sensitize();
            continue;
        }
        if ( ! strncmp (OPT_CMD, buf, strlen (OPT_CMD)))
        {
            opt_load_path (&(cmd_wdg->fname), buf, NULL, OPT_CMD);
            hbuf->filename_to_return = g_strdup (cmd_wdg->fname);
            do_cmdfileload (hbuf);
            continue;
        }

        if ( ! strncmp (OPT_LBL, buf, strlen (OPT_LBL)))
        {
            opt_load_path (&(hbuf->lblfile.fname), buf, NULL, OPT_LBL);
            hbuf->filename_to_return = g_strdup (hbuf->lblfile.fname);
            do_lblfileload (hbuf);
            continue;
        }

        if ( ! strncmp (OPT_DEF, buf, strlen(OPT_DEF)))
        {
            opt_load_path (&(defs_wdg->fname), buf, &alt_defs, OPT_DEF);
            continue;
        }
        if ( ! strncmp (OPT_OBJ, buf, strlen (OPT_OBJ)))
        {
            opt_load_path (&(asmout_wdg->fname), buf, &write_obj, OPT_OBJ);
            continue;
        }
        if ( ! strncmp (OPT_LST, buf, strlen (OPT_LST)))
        {
            gchar *start = buf;
            gchar **splits;

            start += strlen (OPT_LST);
            splits = g_strsplit (start," ", -1);

            if (splits[0])
            {
                sscanf (splits[0], "%d", &write_list);
            }

            if (write_list == LIST_FILE)
            {
                free_filename_to_return (&listing_wdg->fname);
               
                if (splits[1])
                {
                    g_strstrip (splits[1]);

                    if (strlen(splits[1]))
                    {
                        listing_wdg->fname = g_strdup (splits[1]);
                    }
                }
            }

            g_strfreev (splits);
            continue;
        }

        if (!opt_get_val (OPT_RS, buf, &isrsdos))
        {
            if (!opt_get_val (OPT_CPU, buf, &cputype))
            {
                if (!opt_get_val (OPT_UPC, buf, &upcase))
                {
                    if (!opt_get_val (OPT_PGW, buf, &pgwdth))
                    {
                        /* Here, quietly test the last one and just
                         * ignore the line if this does not match
                         */
                        opt_get_val (OPT_PGD, buf, &pgdpth);
                    }
                }
            }
        }
    }
}

