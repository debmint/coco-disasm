/* *********************************** *
 * filestuff.c - handles file I/O, etc *
 * $Id: *
 * *********************************** */

#include <gtk/gtk.h>
#include "g09dis.h"
#include <stdio.h>
#include <string.h>
/*#include <unistd.h>*/
#include <stdlib.h>

#ifdef WIN32
#define DIRSTR "\\"
#define DIRCHR 92
#else
#define DIRSTR "/"
#define DIRCHR '/'
#endif

/* *******************************************
 * File selection stuff
 * *******************************************
 */

gchar *dirname;                 /* ptr to current directory selected */

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

/* ********************* *
 * select a file to open *
 * ********************* */
void
selectfile_open (glbls * hbuf, gchar * type)
{
    GtkWidget *fsel;
    gchar *title = g_strconcat ("Select a ", type, NULL);
/*	gchar     *utf8 = g_locale_to_utf8( title, -1, NULL, NULL, NULL );*/

    hbuf->filename_to_return = NULL;

    fsel = gtk_file_chooser_dialog_new (title,
                                        GTK_WINDOW (window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN,
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);    /* Leak??? */

    g_free (title);
/*	g_free( utf8 );*/

/*	g_signal_connect (G_OBJECT (fsel), "destroy",
			G_CALLBACK (gtk_main_quit), NULL);*/

    if (gtk_dialog_run (GTK_DIALOG (fsel)) == GTK_RESPONSE_ACCEPT)
    {

        hbuf->filename_to_return =
            gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fsel));
    }

    gtk_widget_destroy (fsel);
}

/* ********************* *
 * select a file to open *
 * ********************* */
void
selectfile_save (glbls * hbuf, gchar *cur_name, gchar * type)
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
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(fsel), cur_name);
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
            fprintf (outfile, "%s equ %s %c\n", label, addr, *mode);
            g_free(label); g_free(addr), g_free(mode);
        } while (gtk_tree_model_iter_next(model, &iter));
    }

    doc_set_modified(fdat, FALSE);
    
    fclose (outfile);
}

    /* *******
     * This function clears out the data in
     * an existing liststore
     * *******
     */
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

    if (!(outfile = fopen (*newfile, "wb")))
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

        /* **********
         * Clears data in an existing text buffer
         * **********
         */
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

/* load_list_tree() - generic single-column Treeview load */
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
        
        splits = g_strsplit (buffer, "\t", LST_NCOLS);

        if (splits[0] && splits[1])
        {
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
        }

        g_strfreev (splits);
        splits = NULL;
    }

    doc_set_modified(fdat,FALSE);
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
        register char *newbuf;

        /* get rid of newline, leading/trailing whitespaces
         * if present
         */
        g_strstrip(buffer);

        /* get rid of leading whitespaces */
        newbuf = g_strchug (buffer);

        if ((strlen (buffer)) && (*buffer != ' '))
        {
            gchar **splits;
            splits = g_strsplit (buffer, " ", LBL_NCOLS);

            /* Now add to GtkTree */
            if ((splits) && (*splits[0] != '*'))
            {
                GtkTreeIter iter;

                gtk_list_store_append (fdat->l_store, &iter);
                gtk_list_store_set (fdat->l_store, &iter,
                                    LBL_LBL, splits[0], LBL_EQU, splits[1],
                                    LBL_ADDR, splits[2], LBL_CLASS, splits[3],
                                    -1);
            }
            g_strfreev (splits);
        }
    }

    doc_set_modified( fdat, FALSE);

    fclose (infile);
}

void
run_disassembler (GtkAction * action, glbls * hbuf)
{
    GString *CmdL;
    gchar *cmdline;
    gboolean PipeIt = FALSE;

    CmdL = g_string_new("os9disasm ");
    
    if( !bin_file )
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
        g_string_append (CmdL,bin_file);
    }

    if (cmd_cmd)
    {
        g_string_append (CmdL, " -c=");
        g_string_append (CmdL, cmd_cmd);
    }

    if (write_obj)
    {
        g_string_append (CmdL, " -o=");
        g_string_append (CmdL, obj_file);
    }
    
    if( cputype )
    {
        g_string_append (CmdL, " -3");
    }

    if (upcase)
    {
        g_string_append (CmdL, " -u");
    }

    if (pgwdth)
    {
        g_string_append_printf (CmdL, " -pw=%d", pgwdth);
    }

    if (pgdpth)
    {
        g_string_append_printf (CmdL, " -pd=%d", pgdpth);
    }

   /* if (write_list)
    {
        g_string_append_printf (CmdL, " > %s", listing_output);
    }*/

    switch (write_list) {
        case LIST_FILE:
            if(strlen(listing_output))
            {
                g_string_append_printf (CmdL, " > %s", listing_output);
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

    cmdline = g_string_free(CmdL, FALSE);
    
    g_print ("The command line is...\n");
    g_print ("      %s\n",cmdline);

    if ( !(PipeIt))
    {
        system(cmdline);
    }
    else {
        FILE *infile;
        
        /* Now open the file and read it */
        if (!(infile = popen (cmdline, "r")))
        {
            fprintf (stderr, "Cannot pipe \"%s\" for read!\n",
                              cmdline);
        }
        else {
            load_list_tree (&hbuf->list_file, infile);
            pclose (infile);
        }
    }

    g_free (cmdline);
}
/*--------------- cut -----------*/

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

/* *************************************** *
 * Callbacks for file selection            *
 * Passed: GtkAction *action,              *
 *       gpointer user_data                *
 * *************************************** */

void
compile_listing (GtkAction * action, glbls * hbuf)
{
    FILE *infile;
    int old_write_list = write_list;

    /* temporarily flag that we're writing to GUI */
    write_list = LIST_GTK;

    run_disassembler (action, hbuf);

    write_list = old_write_list;   /* restore write_list */
}

void
load_listing (GtkAction * action, glbls * hbuf)
{
    FILE *infile;

    selectfile_open (hbuf, "Prog Listing");

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
            fprintf (stderr, "Cannot file \"%s\" for read!\n",
                              (hbuf->list_file).fname);
        }
        else {
            load_list_tree (&hbuf->list_file, infile);
            fclose (infile);
        }
    }
}

void
load_cmdfile (GtkAction * action, glbls * hbuf)
{
    selectfile_open (hbuf, "Command file");
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {
        load_text (&hbuf->cmdfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
}

void
load_lblfile (GtkAction * action, glbls * hbuf)
{
    selectfile_open (hbuf, "Label File");
    
    if (hbuf->filename_to_return != NULL)       /* Leak? */
    {
        load_lbl (&hbuf->lblfile, list_win, &(hbuf->filename_to_return));
    }

    free_filename_to_return (&hbuf->filename_to_return);
}

/* ************************************************ *
 * File Save callbacks                              *
 * These that follow are the top-level entry points *
 * for these routines - they call others to do the  *
 * "manual labor"                                   *
 * ************************************************ */

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
