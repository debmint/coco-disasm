/* ******************************************************************** *
 * filestuff.c - handles file I/O, selection, etc                       $
 * $Id::                                                                $
 * ******************************************************************** */


#include <stdio.h>
#include "win09dis.h"
#include <commctrl.h>
//#include <string.h>
///*#include <unistd.h>*/
//#include <stdlib.h>
#include <ctype.h>

#include <sys/stat.h>

//#ifdef HAVE_LIBGEN_H
//#   include <libgen.h>
//#endif
//
//#ifdef WIN32
//#define DIRSTR "\\"
//#define DIRCHR 92
//#else
//#define DIRSTR "/"
//#define DIRCHR '/'
//#endif
//
//#define OPT_BIN "BIN-FILE:"
//#define OPT_CMD "CMD-FILE:"
//#define OPT_LBL "LBL-FILE:"
//#define OPT_DEF "ALT-DEFS:"
//#define OPT_OBJ "SRC-FILE:"
//#define OPT_RS  "RS-FILE:"
//#define OPT_CPU "CPU:"
//#define OPT_UPC "UPCASE:"
//#define OPT_PGW "PGWDTH:"
//#define OPT_PGD "PGDPTH:"
//#define OPT_LST "LISTING:"

static char *filefilters =
                "Command files\0*.cmd\0Label files\0*.lbl\0All Files\0*.*\0";
/* ******************************************* *
 * File selection stuff                        *
 * ******************************************* */

///* *********************************************
// * ending_slash() append a "/" to the end of a *
// * directory pathname if not already there     *
// * *********************************************/
//
//gchar *
//ending_slash (const gchar * dirnm)
//{
//    if (!dirnm)
//    {
//        return g_strdup ("");
//    }
//
//    if (dirnm[strlen (dirnm) - 1] == DIRCHR)
//    {
//        return g_strdup (dirnm);
//    }
//    else
//    {
//        return g_strconcat (dirnm, DIRSTR, NULL);
//    }
//}
//
///* **************************************************************** *
// * set_fname() - Generic filename setup, called by the following    *
// *               file-selection callbacks.                          *
// *               Checks that filename is present in                 *
// *               hbuf->filename_to_return, and if so, clears and    *
// *               resets the filename ptr passed as a reference in   *
// *               "flag".                                            *
// * Passed : (1) - Ptr to the glbls structure                        *
// *          (2) - Pointer to the appropriate filename ptr           *
// * **************************************************************** */
//
//void
//set_fname (glbls *hbuf, gchar **flag)
//{
//    if (hbuf->filename_to_return)
//    {
//        if (strlen(hbuf->filename_to_return))
//        {
//            if (*flag != NULL)
//            {
//                g_free(*flag);
//            }
//
//            *flag = g_strdup(hbuf->filename_to_return);
//        }
//
//        g_free(hbuf->filename_to_return);
//        hbuf->filename_to_return = NULL;
//    }
//}
//
///* Returns column number or -1 if not found or on error */
//
//gint
//get_col_number_from_tree_view_column (GtkTreeViewColumn *col)
//{
//    GList *cols;
//    gint   num;
//
//    g_return_val_if_fail ( col != NULL, -1 );
//    g_return_val_if_fail ( col->tree_view != NULL, -1 );
//
//    cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(col->tree_view));
//
//    num = g_list_index(cols, (gpointer) col);
//
//    g_list_free(cols);
//
//    return num;
//}
//
///* Callback for click on a row in the Listing display */
//
//gboolean
//onListRowButtonPress (GtkTreeView *treevue, GdkEventButton *event,
//                      gchar *menu_name)
//{
//    GtkTreeSelection *selection;
//
//    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(treevue));
//   
//    /* single click with the right mouse button? */
//    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3)
//    {
//        GtkWidget *popup;
//        
//        /* optional: select row if no row is selected or only
//         * one other row is selected (will only do something
//         * if you set a tree selection mode as described later
//         * in the tutorial) */
//
//        /* Note: gtk_tree_selection_count_selected_rows() does not
//         * exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
//        if (gtk_tree_selection_count_selected_rows(selection)  <= 1)
//        {
//            GtkTreePath *path;
//
//            /* Get tree path for row that was clicked */
//            if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treevue),
//                                              event->x, event->y,
//                                              &path, NULL, NULL, NULL))
//            {
//                gtk_tree_selection_unselect_all(selection);
//                gtk_tree_selection_select_path(selection, path);
//                gtk_tree_path_free(path);
//            }
//        } /* end of optional bit */
//
//        /*if( ! gtk_tree_selection_get_selected (list_selection,
//                                               NULL, NULL))
//        {*/
//            /* No row selected ... return */
//           /* return FALSE;
//        }*/
//        
//        /* do pop-up window */
//        popup = gtk_ui_manager_get_widget (ui_manager, menu_name);
//
//        gtk_menu_popup (GTK_MENU(popup), NULL, NULL,NULL, NULL,
//                        0, gdk_event_get_time((GdkEvent*)event));
//        return TRUE;
//    }
//    
//    /* we did nothing */
//    return FALSE;
//}
//
///* ******************************************************************** *
// * set_chooser_folder() - Sets the folder path in the GtkFileChooser    *
// *               if appropriate.                                        *
// * Passed:  (1) - GtkFileChooser in which to set folder                 *
// *          (2) - glbls * hbuf in which to search for paths             *
// * ******************************************************************** */
//
//void
//set_chooser_folder(GtkFileChooser *chooser, glbls *hbuf)
//{
//    gchar *path;
//
//    path = NULL;
//    
//    /* Try to guess a suitable path */
//
//    if (cmd_wdg->fname)
//    {
//        path = cmd_wdg->fname;
//    }
//    else if ((hbuf->cmdfile).fname)
//    {
//        path = (hbuf->cmdfile).fname;
//    }
//    else if ((hbuf->lblfile).fname)
//    {
//        path = (hbuf->lblfile).fname;
//    }
//    else if (prog_wdg->fname)
//    {
//        path = prog_wdg->fname;
//    }
//    else if (LastPath)
//    {
//        path = LastPath;
//    }
//
//    if (path)
//    {
//        path = g_path_get_dirname (path);
//        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER(chooser), path);
//        g_free (path);
//    }
//}

/* Clean up */

void
free_reference (char **fname)
{
    if (*fname != NULL)
    {
        free (*fname);
        *fname = NULL;
    }
}

/* ********************************************************* *
 * selectfile_open() - Select a file to open                 *
 * Passed:  (1) - ptr to appropriate glbls struct            *
 *          (2) - Type (string for prompt                    *
 *          (3) - Boolean TRUE if file, FALSE if dir         *
 *          (4) - Filename to display or NULL                *
 * Runs gtk_file_chooser_dialog                              *
 * Sets hbuf->filename_to_return to selected file            *
 * ********************************************************* */

BOOL
selectfile_open ( HWND hwnd,
                  const char *type,
                  BOOL IsFile,
                  char **fnam)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory (&ofn, sizeof (ofn));

    // NOTE: We may wish to make this structure global and simply
    // update it when calling.

    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filefilters;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if ( ! GetOpenFileName (&ofn))
    {
        return FALSE;
    }

    // First, free any previously-allocated filename buffer

    //free_reference (&(hbuf->filename_to_return));

    free_reference (fnam);
    
    if ( ! (*fnam = malloc (strlen (ofn.lpstrFile) + 1)))
    {
        MessageBox (hwnd, "Memory allocation failure", "Error!",
                MB_ICONERROR | MB_OK);
        return FALSE;
    }

    strcpy (*fnam, ofn.lpstrFile);
   // hbuf->filename_to_return = fnam;

    return TRUE;
}

/* ************************************************************ *
 * selectfile_save () -select a file to save                    *
 * Passed : (1) parent - Window which will own dialog           *
 *          (2) dest   - pointer to char * which will reference *
 *                       the filename                           *
 *                       WARNING !!! Dest MUST BE NULL or this  *
 *                       function will attempt to free () the   *
 *                       memory !!!!!                           *
 * Returns: Pointer to new name on selection or                 *
 *          NULL if no selection is made                        *
 * Memory is allocated for the string                           *
 * ************************************************************ */

char *
selectfile_save (HWND parent, char **dest)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory (&ofn, sizeof (ofn));

    // NOTE: We may wish to make this structure global and simply
    // update it when calling.

    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFilter = filefilters;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT; 

//    if (fdat->fname)
//    {
//        strcpy (ofn.lpstrFile, fdat->fname);
//    }
    
    if (GetSaveFileName (&ofn))
    {
        char *fnam;
        
        free_reference (dest);

        if ( ! (fnam = malloc (strlen (ofn.lpstrFile) + 1)))
        {
            MessageBox (parent, "Memory allocation failure", "Error!",
                    MB_ICONERROR | MB_OK);
            return NULL;
        }

        strcpy (fnam, ofn.lpstrFile);
        *dest = fnam;
        return fnam;
    }

    return NULL;
}

/* ****************************** *
 * Text Buffer handling stuff     *
 * ****************************** */

/* ****************************************************** *
 * strstrip () - strips leading whitespaces and trailing  *
 *             returns and newlinesfrom string.           *
 *             Advances pointer to first non-white char   *
 * Returns: Possibly newly-advanced pointer to begin of   *
 *          string.                                       *
 * ****************************************************** */

char *
strstrip (char *ptr)
{
    while ((*ptr == ' ') || (*ptr == '\r') || (*ptr == '\t'))
    {
        ++ptr;
    }

    if (strchr (ptr, '\r'))
    {
        *strchr (ptr, '\r') = '\0';
    }

    if (strchr (ptr, '\n'))
    {
        *strchr (ptr, '\n') = '\0';
    }

    return ptr;
}

///* ******************************************** *
// * save_all_query() - notify user that multiple *
// * files (might) need saving returns choice of  *
// * NONE, ALL, or USER SELECT                    *
// * ******************************************** */
//
//gint
//save_all_query()
//{
//    GtkWidget *dialog;
//    gint result;
//
//    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
//                                    GTK_DIALOG_MODAL |
//                                       GTK_DIALOG_DESTROY_WITH_PARENT,
//                                    GTK_MESSAGE_QUESTION,
//                                    GTK_BUTTONS_NONE,
//                                    "You are about to exit and your files have been altered\nWhat do you wish to do?"); 
//
//    gtk_dialog_add_buttons (GTK_DIALOG(dialog),
//                            "Exit without saving", SAVALL_NONE,
//                            "Save All", SAVALL_ALL,
//                            "Select files", SAVALL_SOME,
//                            NULL);
//    result = gtk_dialog_run(GTK_DIALOG(dialog));
//    gtk_widget_destroy(dialog);
//    return result;
//}

/* ******************************************************** *
 * doc_set_modified():                                      *
 *                                                          *
 *     sets the FILEINF->altered flag to value              *
 *     if it is already this value, do nothing              *
 * ******************************************************** */

void
doc_set_modified (FILEINF *doc, BOOL value)
{
    if(doc->altered != value)
    {
        doc->altered = value;
    }
}

/* ***************************************************** *
 * save_warn_OW() - Alert user that a single altered but *
 *     not-saved buffer is about to be overwritten       *
 * ***************************************************** */

int
save_warn_OW (FILEINF *fdat, char *type, BOOL can_cancel)
{
    int MB_BUTTONSET;
    char warnmsg[100];
//    GtkWidget *dialog;
//    gint response;
    char *fn;

    MB_BUTTONSET= can_cancel ? MB_YESNOCANCEL : MB_YESNO;

    if (fdat->fname)
    {
        fn = fdat->fname;
    }
    else {
        fn = "(Untitled)";
    }

    sprintf (warnmsg,
       "About to discard altered %s file buffer\nDo you wish to save to %s\n",
       type, fn);

    return MessageBox (fdat->l_store, warnmsg, "Warning!",
            MB_ICONWARNING | MB_BUTTONSET);

//    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
//                                    GTK_DIALOG_MODAL |
//                                       GTK_DIALOG_DESTROY_WITH_PARENT,
//                                    GTK_MESSAGE_QUESTION,
//                                    GTK_BUTTONS_NONE,
//                                    "About to discard altered %s file buffer\nDo you wish to save to %s\n", type, fn); 
//
//    gtk_dialog_add_button (GTK_DIALOG(dialog),
//                           "Discard Changes", GTK_RESPONSE_NO);
//    if(can_cancel) {
//        gtk_dialog_add_button (GTK_DIALOG(dialog),
//                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
//    }
//    gtk_dialog_add_button (GTK_DIALOG(dialog),
//                           "Save Changes", GTK_RESPONSE_YES);
//    
//    gtk_widget_show_all(dialog);
//
//    response = gtk_dialog_run(GTK_DIALOG(dialog));
//    gtk_widget_destroy(dialog);
//    return response;
}

///* ************************* *
// * save_lbl() : save a label *
// *        buffer to a file   *
// * ************************* */
//
//static void
//save_lbl(FILEINF *fdat, GtkWidget *my_win, gchar **newfile)
//{
//    FILE *outfile;
//    GtkTreeModel *model;
//    GtkTreeIter iter;
//
//    /* TODO: rename file if it exists? */
//
//    if (!(outfile = fopen (*newfile, "wb")))
//    {
//        fprintf (stderr, "Cannot open file %s for write!\n", *newfile);
//    }
//
//    /* set up buffer to write */
//    model= gtk_tree_view_get_model (GTK_TREE_VIEW(O9Dis.lblfile.tview));
//    
//    if (gtk_tree_model_get_iter_first (model, &iter))
//    {
//        gchar *label, *addr, *mode;
//        do {
//            gtk_tree_model_get (model, &iter,
//                                LBL_LBL, &label,
//                                LBL_ADDR, &addr,
//                                LBL_CLASS, &mode,
//                                -1);
//            if (*label != '*')
//            {
//                fprintf (outfile, "%s equ %s %c\n", label, addr, *mode);
//            }
//            else {  /* do differently for comment */
//                fprintf (outfile, "%s %s\n", label, mode);
//            }
//            
//            g_free(label); g_free(addr), g_free(mode);
//        } while (gtk_tree_model_iter_next(model, &iter));
//    }
//
//    doc_set_modified(fdat, FALSE);
//    
//    fclose (outfile);
//}

    /* ************************************ *
     * This function clears out the data in *
     * an existing liststore                *
     * ************************************ */

void
list_store_empty (FILEINF *fdat)
{
    HWND mytv = fdat->l_store;  

    /* If data currently there and has been altered, save it?
     * NOTE: This function is called by both label and listing,
     * but for the listing, fdat->altered should NEVER be set
     * (we don't save the listing
     */

//    if (fdat->altered == TRUE)
//    {
//        switch(save_warn_OW ("Label", fdat->fname, TRUE)) {
//            case GTK_RESPONSE_NO: /* discard changes */
//                break;
//            case GTK_RESPONSE_CANCEL: /* cancel overwrite */
//                return;
//                break;
//            case GTK_RESPONSE_YES: /* save changes */
//                /* Note: in calling save_lbl, pass GtkWidget *window
//                 * as a dummy - it won't be needed */
//                save_lbl(fdat, window, &fdat->fname);
//                break;
//        }
//    }

    /* clear out current list_store */

//    while (ListView_GetItemCount (mytv))
//    {
//        ListView_DeleteItem (mytv, 0);  // Delete the first item each time
//    }
    ListView_DeleteAllItems (mytv);

//    if (fdat->fname != NULL)
//    {
//        doc_set_modified (fdat, FALSE);
//        free (fdat->fname);
//        fdat->fname = NULL;
//    }
}

/* ************************************************ *
 * save_text() : save a text buffer to a file       *
 * Passed : (1) Edit window containing text to save *
 *          (2) the name of the file to save to     *
 * ************************************************ */

static BOOL
save_text (HWND hEdit, char ** newfile)
{

    HANDLE hFile;
    BOOL bSuccess = FALSE;
    DWORD dwTextLength;

    hFile = CreateFile (*newfile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    dwTextLength = GetWindowTextLength(hEdit);
    
    // No need to bother if there's no text.

    if (dwTextLength > 0)
    {
        LPSTR pszText;
        DWORD dwBufferSize = dwTextLength + 1;

        pszText = GlobalAlloc(GPTR, dwBufferSize);
        
        if (pszText != NULL)
        {
            if (GetWindowText(hEdit, pszText, dwBufferSize))
            {
                DWORD dwWritten;

                if (WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL))
                    bSuccess = TRUE;
            }

            GlobalFree(pszText);
        }
    }

    CloseHandle(hFile);
    return bSuccess;
//    FILE *outfile;
//    char *buf;
//    GtkTextIter begin, end;
//
//    /*fdat->fname = g_strdup (*newfile);
//    g_free (*newfile);
//    *newfile = NULL;*/
//    
//    /* TODO: rename file if it exists? */
//
//    if ( ! (outfile = fopen (*newfile, "wb")))
//    {
//        fprintf (stderr, "Cannot open file %s for write!\n", *newfile);
//    }
//
//    /* set up buffer to write */
//
//    gtk_text_buffer_get_start_iter (fdat->tbuf, &begin);
//    gtk_text_buffer_get_end_iter (fdat->tbuf, &end);
//    buf = gtk_text_buffer_get_text (fdat->tbuf, &begin, &end, FALSE);
//    doc_set_modified(fdat, FALSE);
//
//    if ((fputs (buf,outfile)) == EOF)
//    {
//        GtkWidget *dialog;
//        
//        dialog = gtk_message_dialog_new (GTK_WINDOW(window),
//                                         GTK_DIALOG_DESTROY_WITH_PARENT,
//                                         GTK_MESSAGE_ERROR,
//                                         GTK_BUTTONS_OK,
//                                         "Error writing data to \"%s\"",
//                                            *newfile);
//        gtk_dialog_run (GTK_DIALOG(dialog));
//        gtk_widget_destroy (dialog);
//
//        g_free(buf);
//    }
//
//    doc_set_modified(fdat, FALSE);
//
//    fclose (outfile);
}

/* ******************************************************************** *
 * Clears data in an existing text buffer                               *
 * Using Windows' SetText, there is no need to even clear the           *
 * text buffer, but this call is used to check for an altered buffer    *
 * ******************************************************************** */

BOOL
clear_text_buf( FILEINF *fdat)
{
    //char msg[100];

    /* If data currently there and has been altered, save it? */
    if (SendMessage (fdat->l_store, EM_GETMODIFY,0, 0) == TRUE)
    {
        switch (save_warn_OW (fdat, "Command", TRUE)) {
            case IDNO:         /* discard changes */
                break;
            case IDCANCEL: /* cancel overwrite */
                return FALSE;
                break;
            case IDYES: /* save changes */
                save_text (fdat->l_store, &fdat->fname);
                break;
        }
    }

    //doc_set_modified (fdat,FALSE);

    /* free up any memory previously allocated (if any) */
    //free_reference (&(fdat->fname));
    
//    if (fdat->fname != NULL)
//    {
        //GtkTextIter start, end;

//        free (fdat->fname);
//        fdat->fname = NULL;
        
        SendMessage (fdat->l_store, EM_SETSEL, 0, -1);
       // SendMessage (fdat->l_store, WM_CLEAR, 0, 0);
//        gtk_text_buffer_get_bounds (fdat->tbuf, &start, &end);
//        gtk_text_buffer_delete (fdat->tbuf, &start, &end);
//    }

    return TRUE;
}

/* ******************************************************************** *
 * load_text() - Loads a text file and places it into the edit buffer   *
 * ******************************************************************** */

BOOL
load_text (FILEINF *fdat, char **newfile)
{
    register FILE *infile;
    char buffer[500];
    char *pszFileText;
    struct stat cmd_stat;
    BOOL bSuccess = FALSE;

    clear_text_buf (fdat);

    if ( newfile && *newfile)
    {
        free_reference ( &(fdat->fname));
        fdat->fname = strdup (*newfile);
    }

    stat (fdat->fname, &cmd_stat);

    /* Now open the file and read it
     * We use the stdio method because we want to convert any non-MS-DOS
     * EOL's (single \r or \l)into MS-DOS format, as the edit control
     * must (?) have it in this form
     */

    if ( !(infile = fopen (fdat->fname, "r")))
    {
        MessageBox (fdat->l_store, "Error opening cmd file", "Error!",
                MB_ICONEXCLAMATION | MB_OK);
    }

    // Allow space in the file for addition of return characters
    // for MS-DOS format

    if ((pszFileText = GlobalAlloc(GPTR,
                cmd_stat.st_size + (cmd_stat.st_size)/5)))
    {
        pszFileText[0] = '\0';

        // We use fgets so that newlines will be converted in case
        // the file is in unix format
        // Actually, This fgets seems to delete carriage returns,
        // so this check is absolutely necessary in any case.

        while (fgets (buffer, sizeof (buffer), infile))
        {
            if ( buffer[strlen(buffer) - 1] == '\x0d')
            {
                if ( ! strchr (buffer, '\x0a'))
                {
                    strcat (pszFileText, "\x0a");
                }

                strcat (pszFileText, buffer);
                continue;
            }

            strcat (pszFileText, buffer);

            if ( buffer[strlen(buffer) - 1] == '\x0a')
            {
                if ( ! strchr (buffer, '\x0d'))
                {
                    pszFileText[strlen(pszFileText) - 1] = '\0';
                    strcat (pszFileText, "\x0d\x0a");
                }
            }
        }

        //pszFileText[cmd_stat.st_size] = 0; // Add null terminator

        if (SetWindowText (fdat->l_store, pszFileText))
        {
            bSuccess = TRUE; // It worked!
        }

        GlobalFree (pszFileText);
        fclose (infile);
        doc_set_modified(fdat, FALSE);
    }

    return bSuccess;
}

/* ******************************************************************** *
 * load_text() : load a file into a text buffer                         *
 * This version uses the Windows file I/O functions, and is probably    *
 * more efficient than retrieving one line at a time using fgets(),     *
 * but it does not handle other systems' EOL setups, and the EOL MUST   *
 * be in MS-Dos \r\l format for the edit controls to display properly.  *
 * If one is certain that no files from foreigh systems will ever be    *
 * read, then this function can be renamed to "load_text" and the       *
 * current "load_text" can be renamed to "unx_load_text" or something   *
 * and it will not be accessed.                                         *
 * ******************************************************************** */

BOOL
mswin_load_text (FILEINF * fdat, char **newfile)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    clear_text_buf (fdat);

    free_reference ( &(fdat->fname));
    fdat->fname = strdup (*newfile);

    /* Now open the file and read it */

    hFile = CreateFile (fdat->fname, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwFileSize;

        dwFileSize = GetFileSize (hFile, NULL);
        if (dwFileSize != 0xFFFFFFFF)
        {
            LPSTR pszFileText;

            pszFileText = GlobalAlloc(GPTR, dwFileSize + 1);

            if (pszFileText != NULL)
            {
                DWORD dwRead;

                if (ReadFile (hFile, pszFileText, dwFileSize, &dwRead, NULL))
                {
                    pszFileText[dwFileSize] = 0; // Add null terminator

                    if (SetWindowText (fdat->l_store, pszFileText))
                        bSuccess = TRUE; // It worked!
                }

                GlobalFree (pszFileText);
            }
        }

        CloseHandle (hFile);
    }

    doc_set_modified(fdat, FALSE);
    return bSuccess;
}

/* ****************************************************** *
 * str_digit() - check that a string is a valid deximal # *
 * Passed:  str - pointer to string to check              *
 * Returns: 1 if all chars in string match,               *
 *          0 if any do not                               *
 * ****************************************************** */

BOOL
str_digit (char *str)
{
    char *ch = str;

    while (*ch)
    {
        if ( ! isdigit (*(ch++)))
        {
            return FALSE;
        }
    }

    return TRUE;     /* If we get here, all chars have matched */
}

/* ****************************************************** *
 * str_xdigit() - check that a string is a valid hex #    *
 * Passed:  str - pointer to string to check              *
 * Returns: 1 if all chars in string match,               *
 *          0 if any do not                               *
 * ****************************************************** */

BOOL
str_xdigit (char *str)
{
    char *ch = str;

    while (*ch)
    {
        if ( ! isxdigit (*(ch++)))
        {
            return FALSE;
        }
    }

    return TRUE;     /* If we get here, all chars have matched */
}

/* ****************************************************** *
 * load_list_tree() - generic single-column Treeview load *
 * ****************************************************** */

static void
load_list_tree (FILEINF *fdat, FILE *infile)
{
    char buffer[500];
    int itemcount;

    itemcount = 0;
    
    while (fgets (buffer, sizeof (buffer), infile))
    {
        // gcc complained about strpt being uninitialized, so
        // we initialize it...
        char *strpt = "";
        LV_ITEM lv;

        // Eliminate possible newline at end

        if ((strpt = strchr(strpt, '\n')))
        {
            *strpt = '\0';
        }
        
        // Skip past whitespaces
        strpt = buffer;
        
        strpt = strstrip (strpt);

        if (strlen (buffer))
        {
            char *splits[LST_NCOLS];
            int   fldnum;
            int place;
            BOOL illeg;
            
            /* Assure that there are enough fields */

            strcat (buffer,"\t\t\t\t\t\t\t");

            // Split fields into an array

            for (fldnum = 0; fldnum < LST_NCOLS; fldnum++)
            {
                // Substitute NULL for next tab (end of this field

                splits[fldnum] = strpt;
                strpt = strchr (strpt, '\t');

                *(strpt++) = '\0';
            }

            // If Line # is not all digits, or Address is not a real hex #,
            // then we're not on an assembler command line, so skip

            illeg = FALSE;
            
            for (place = 0; place < strlen (splits[LST_LIN]); place++)
            {
                if ( ! strchr ("0123456789", splits[LST_LIN][place]))
                {
                    illeg = TRUE;
                    break;
                }
            }

            if (illeg)
            {
                continue;
            }
                
            for (place = 0; place < strlen (splits[LST_ADR]); place++)
            {
                if ( ! strchr ("0123456789ABCDEFabcdef",
                                splits[LST_ADR][place]))
                {
                    illeg = TRUE;
                    break;
                }
            }

            if (illeg)
            {
                continue;
            }
                
            // Now insert items into ListView

            ListView_SetItemCount (fdat->l_store, ++itemcount);

            ZeroMemory (&lv, sizeof (lv));
            lv.mask = LVIF_TEXT;
            lv.iItem = itemcount - 1;

                if (ListView_InsertItem (fdat->l_store, &lv) == -1)
                {
                    char mesg[30];

                    sprintf (mesg, "Failed to insert item - %d,%d",
                                  itemcount - 1, fldnum);
                    MessageBox (fdat->l_store, mesg, "Error!",
                               MB_ICONERROR | MB_OK);
                }
            for (fldnum = 0; fldnum < LST_NCOLS; fldnum++)
            {
                ListView_SetItemText (fdat->l_store, itemcount - 1,
                            fldnum, splits[fldnum]); 
            }   // for (fldnum...
        }
    }       // while (fgets...

    doc_set_modified (fdat, (BOOL)FALSE);
}

/* ************************************************************** *
 * do_lblfileload() - load the label file into the GtkTreeView    *
 * ************************************************************** */

void
do_lblfileload(FILEINF *fdat)
{
    FILE *infile;
    char buffer[500];
    int itemcount = 0;

    LV_ITEM lv;
    ZeroMemory (&lv, sizeof (lv));
    lv.mask = LVIF_TEXT;

    list_store_empty (fdat);

//    if (fdat->fname)
//    {
//        free_reference (&fdat->fname);
//    }

    /* Now open the file and read it */

    if ( ! (infile = fopen (fdat->fname, "rb")))
    {
        char emsg[80];
        sprintf (emsg, "Cannot open file %s for read!", fdat->fname);
        MessageBox (fdat->l_store, emsg, "Error!",
                MB_ICONERROR | MB_OK);
        return;
    }

    while (fgets (buffer, 160, infile))
    {
        char *splits[LBL_NCOLS];
        int fldnum = 0;

        /* get rid of newline, leading/trailing whitespaces
         * if present
         */
        strstrip (buffer);

        if ((strlen (buffer)) && (*buffer != ' '))
        {
            if (*buffer != '*')
            {
                char *tmppt;

                /* convert tabs to spaces  
                 * if file is saved, spaces will be used */
               
                while ((tmppt = strchr (buffer, '\t')))
                {
                    *tmppt = ' ';
                }
                
                tmppt = buffer;

                for (fldnum = 0; fldnum < LBL_NCOLS; fldnum++)
                {
                    while ((*tmppt == ' '))
                    {
                        ++tmppt;
                    }
                    
                    splits[fldnum] = tmppt;

                    if (fldnum < (LBL_NCOLS - 1))
                    {
                        tmppt = strchr (tmppt, ' ');
                        *(tmppt++) = '\0';
                    }
                }

                
            }
            else   /* A comment line */
            {
                int sspos = 1;  /* Default separator to second position */
                
                if (buffer[1] == '\\')
                {
                    ++sspos;   /* Bump separator past '\' */
                    splits[0]= "*\\";

                    /* AMode def - add to AMode list */
//                    amode_add_from_string (&buffer[sspos]);
                }
                else
                {
                    splits[0] = "*";
                }

                if (strlen (&buffer[sspos]))
                {
                    splits[1] = &buffer[sspos];
                }
                else
                {
                    splits[1] = "";
                }

                splits[2] = splits[3] = "";

                /* Now add to ListStore */
            }

                // Now insert items into ListView

            ListView_SetItemCount (fdat->l_store, itemcount + 1);

            lv.iItem = itemcount + 1;
            lv.iSubItem = 0;

            if (ListView_InsertItem (fdat->l_store, &lv) == -1)
            {
                static int failcount;
                char mesg[30];

                sprintf (mesg, "Failed to insert item - %d,%d",
                              itemcount, fldnum);
                MessageBox (fdat->l_store, mesg, "Error!",
                           MB_ICONERROR | MB_OK);
                if (++failcount > 5)
                {
                    exit(1);
                }
            }

            for (fldnum = 0; fldnum < LBL_NCOLS; fldnum++)
            {
                ListView_SetItemText (fdat->l_store, itemcount,
                            fldnum, splits[fldnum]); 
            }

            ++itemcount;
        }       // if (strlen(buffer))

    }     // while (fgets...

    doc_set_modified( fdat, FALSE);

    fclose (infile);
}

///* **************************************** *
// * sysfailed() - report failure of system() *
// *     call                                 *
// * **************************************** */
//
//void
//sysfailed (char *msg)
//{
//    GtkWidget *dialog;
//
//    dialog = gtk_message_dialog_new (GTK_WINDOW(window),
//                                     GTK_DIALOG_DESTROY_WITH_PARENT ||
//                                         GTK_DIALOG_MODAL,
//                                     GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
//                                     msg);
//    gtk_dialog_run (GTK_DIALOG (dialog));
//    gtk_widget_destroy (dialog);
//}
//
//
///* **************************************************** *
// * run_disassembler  - a callback from the menu does a  *
// *                     disassembly pass                 *
// * Passed: action, the global variables                 *
// * **************************************************** */
//
//void
//run_disassembler (GtkAction * action, glbls * hbuf)
//{
//    GString *CmdL;
//    gchar *cmdline;
//    gboolean PipeIt = FALSE;
//
//    CmdL = g_string_new("os9disasm ");
//
//    /* We MUST have a binary file to disassemble */
//    
//    if( ! prog_wdg->fname )
//    {
//        GtkWidget *warnwin;
//        gchar *msg;
//
//        msg = g_strconcat("WARNING!\n",
//                          "No program file specified\n",
//                          "Please configure in menu item \"Options\"\n",
//                          NULL );
//        warnwin = gtk_message_dialog_new( GTK_WINDOW(window),
//                                          GTK_DIALOG_DESTROY_WITH_PARENT ||
//                                          GTK_DIALOG_MODAL,
//                                          GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
//                                          msg );
//        gtk_dialog_run (GTK_DIALOG(warnwin));
//        gtk_widget_destroy (warnwin);
//        g_string_free (CmdL,TRUE);
//        return;
//    }
//    else {
//        g_string_append (CmdL,prog_wdg->fname);
//    }
//
//    if (cmd_wdg->fname)
//    {
//        g_string_append (CmdL, " -c=");
//        g_string_append (CmdL, cmd_wdg->fname);
//    }
//
//    if (alt_defs)
//    {
//        g_string_append_printf (CmdL, " -d=%s", defs_wdg->fname);
//    }
//   
//    if (isrsdos)
//    {
//        g_string_append (CmdL, " -x=c");
//    }
//    if (write_obj)
//    {
//        if (asmout_wdg->fname)
//        {
//            if (strlen(asmout_wdg->fname))
//            {
//                g_string_append (CmdL, " -o=");
//                g_string_append (CmdL, asmout_wdg->fname);
//            }
//        }
//    }
//    
//    if( cputype )
//    {
//        g_string_append (CmdL, " -3");
//    }
//
//    if (upcase)
//    {
//        g_string_append (CmdL, " -u");
//    }
//
//    if (showzeros)
//    {
//        g_string_append (CmdL, " -z");
//    }
//
//    if (pgwdth)
//    {
//        g_string_append_printf (CmdL, " -pw=%d", pgwdth);
//    }
//
//    if (pgdpth)
//    {
//        g_string_append_printf (CmdL, " -pd=%d", pgdpth);
//    }
//
//
//    switch (write_list) {
//        case LIST_FILE:
//            if (listing_wdg->fname)
//            {
//                if (strlen(listing_wdg->fname))
//                {
//                    g_string_append_printf (CmdL, " > %s", listing_wdg->fname);
//                }
//            }
//            break;
//        case LIST_NONE:
//            /* handle bit-bucket output */
//            /* g_string_append (CmdL, "-q");*/
//            break;
//        case LIST_GTK:
//            g_string_append (CmdL, " -g");
//            PipeIt = TRUE;
//            break;
//    }
//
//    cmdline = g_string_free (CmdL, FALSE);
//    
//    if ( ! (PipeIt))   /* Output is to stdout or a file */
//    {
//        int retval;
//        char *msg = NULL;
//        
//        switch (retval = system (cmdline))
//        {
//            case 0: /* sucess */
//                break;
//            case -1:  /* failed fork */
//                msg = "Could not fork \"os9disasm\"\nIs \"os9disasm\" in your path?";
//                sysfailed(msg);
//                break;
//            default:
//                msg = "Error in executing \"os9disasm\"\nPlease check console for error messages";
//                sysfailed(msg);
//        }
//    }
//    else        /* else we're piping to the GUI */
//    {
//        FILE *infile;
//        
//        /* Now open the file and read it */
//        if (!(infile = popen (cmdline, "r")))
//        {
//            g_print ("Cannot pipe \"%s\" for read!\n",
//                              cmdline);
//        }
//        else {
//            gtk_list_store_clear ((hbuf->list_file).l_store);
//            load_list_tree (&hbuf->list_file, infile);
//            pclose (infile);
//        }
//    }
//
//    g_free (cmdline);
//}
//
///* Disassemble listing to file */
//
//void
//dasm_list_to_file_cb (GtkAction *action, glbls *hbuf)
//{
//    gint old_write = write_list;
//
//    selectfile_save (hbuf, listing_wdg->fname, "File for Listing");
//    set_fname (hbuf, &listing_wdg->fname);
//    write_list = LIST_FILE;
//    run_disassembler (action, hbuf);
//    write_list = old_write;
//}

///* ******************************************** *
// * Callbacks for file selection                 *
// * Passed:  (1) GtkAction *action,              *
// *          (2) address of global data pointer  *
// * ******************************************** */
//
//void
//compile_listing (GtkAction * action, glbls * hbuf)
//{
//    int old_write_list = write_list;
//
//    /* temporarily flag that we're writing to GUI */
//    
//    write_list = LIST_GTK;
//
//    run_disassembler (action, hbuf);
//
//    write_list = old_write_list;        /* restore write_list */
//}
//
/* ******************************************************** *
 * load_listing() - A callback from the menu(s).            *
 *          This is the "top-level" function for loading    *
 *          a listing                                       *
 * ******************************************************** */

void
load_listing (FILEINF *fdat)
{
    FILE *infile;


    if ( ! selectfile_open (fdat->l_store, "Prog Listing", TRUE,
                                            &(fdat->fname)))
    {
        return;    // We had either "Cancel" or failure
    }

    /* Call list_store_empty - if it's a listing, fdat->altered shouldn't
     * be set to TRUE (hopefully)
     */

    list_store_empty (fdat);

//    (hbuf->list_file).fname = strdup (hbuf->filename_to_return);
//    free_reference (&(hbuf->filename_to_return));

    /* Now open the file and read it */
    
    if ( ! (infile = fopen (fdat->fname, "rb")))
    {
        int lasterr;
        char errstr[200];

        lasterr = GetLastError ();
        sprintf (errstr, "Error #%d encountered opening file \"%s\"",
                         lasterr, fdat->fname);
        return;
    }

    load_list_tree (fdat, infile);
    fclose (infile);
}

void
do_cmdfileload (FILEINF *fdat)
{
    if (fdat->fname != NULL)       /* Leak? */
    {
        load_text (fdat, NULL);
    }

    //free_reference (&hbuf->filename_to_return);
}

/* **************************************************************** *
 * load_cmdfile() - Callback for Button click to load command file  *
 * **************************************************************** */

void
load_cmdfile (FILEINF *fdat)
{
    if (selectfile_open (fdat->l_store, "Command file", TRUE, &(fdat->fname)))
    {
//        free_reference (&(cmd_wdg->fname));
//        cmd_wdg->fname = strdup (hbuf->filename_to_return);
        do_cmdfileload (fdat);
    }
}

//void
//do_lblfileload (FILEINF *fdat)
//{
//    if (fdat->fname != NULL)       /* Leak? */
//    {
//        load_lbl (fdat, fdat->l_store,
//                &(fdat->fname));
//    }
//
//    //free_reference (&hbuf->filename_to_return);
//}

void
load_lblfile (FILEINF *fdat)
{
    if (selectfile_open (fdat->l_store, "Label File", TRUE,
                        &(fdat->fname)))
    {
        do_lblfileload (fdat);
    }
}

/* ************************************************************ *
 * File Save callbacks                                          *
 * These that follow are the top-level entry points for these   *
 * routines - they call others to do the "manual labor"         *
 * ************************************************************ */

void
cmd_save_as (FILEINF *fdat)
{
    if ( ! selectfile_save (fdat->l_store, &(fdat->fname)))
    {
        return;
    }

        /* handle fname */
        
//    free_reference (hbuf->cmdfile.fname);
//    fdat->fname = strdup (hbuf->filename_to_return);
        
        /* save cmd file */
    save_text (fdat->l_store, &(fdat->fname));

//    free_reference (&hbuf->filename_to_return);

}

//void
//cmd_save(GtkAction *action, glbls *hbuf)
//{
//    if (!(hbuf->cmdfile.fname))
//    {
//        cmd_save_as(action, hbuf);
//    }
//    else {
//        save_text (&hbuf->cmdfile, list_win, &(hbuf->cmdfile.fname));
//    }
//
//    gtk_text_buffer_set_modified ((hbuf->cmdfile).tbuf, FALSE);
//}
//
//void
//lbl_save_as (GtkAction * action, glbls * hbuf)
//{
//    selectfile_save(hbuf, hbuf->lblfile.fname, "Label File");
//    
//    if (hbuf->filename_to_return != NULL)       /* Leak? */
//    {
//        if (hbuf->lblfile.fname)
//        {
//            g_free(hbuf->lblfile.fname);
//        }
//
//        hbuf->lblfile.fname = g_strdup(hbuf->filename_to_return);
//        
//        /* save label file */
//        save_lbl (&hbuf->lblfile, list_win, &(hbuf->filename_to_return));
//    }
//
//    free_reference (&hbuf->filename_to_return);
//}
//
//void
//lbl_save (GtkAction *action, glbls *hbuf)
//{
//    if( !(hbuf->lblfile.fname))
//    {
//        lbl_save_as(action, hbuf);
//    }
//    else {
//        save_lbl (&hbuf->lblfile, list_win, &(hbuf->lblfile.fname));
//    }
//}
//
///* **************************************************** *
// * Options load/save callbacks                          *
// * **************************************************** */
//
///* ******************************************************** *
// * opt_put() - check for state of variable ptr and if set,  *
// *          write to optsave file                           *
// * ******************************************************** */
//
//static void
//opt_put (gboolean isset, char *nam, char *hdr, FILE *fp)
//{
//    gchar *line;
//    
//    if (isset)
//    {
//        line = g_strconcat (hdr, nam, "\n", NULL);
//        fputs (line, fp);
//        g_free (line);
//    }
//}
//
//static void
//opt_load_path (gchar **pthptr, gchar *pthnm, gboolean *flag, gchar* hdr)
//{
//    char *start = pthnm;
//
//    start += strlen (hdr);
//    
//    if (flag)
//    {
//        *flag = TRUE;
//    }
//
//    g_strstrip (start);
//    *pthptr = g_strdup (start);
//}
//
///* **************************************************************** *
// * opt_get_val() - check for a match in the HDR and store the value *
// *          of matched                                              *
// * PASSED: hdr string to check against buffer holding the loaded    *
// *          line address to store value if matched                  *
// * Returns: value if matched, NULL if no match                      *
// * **************************************************************** */
//
//static gint
//opt_get_val ( gchar *ctrl, gchar *buf, gint *addr)
//{
//    gchar *start = buf;
//
//    if (!strncmp (ctrl, buf, strlen (ctrl)))
//    {
//        start += strlen (ctrl);
//        g_strstrip (start);
//        sscanf (start, "%d", addr);
//        return *addr;
//    }
//
//    return 0;
//}
//
//void
//opts_save (GtkAction *action, glbls *hbuf)
//{
//    FILE *optfile;
//
//    selectfile_save (hbuf, NULL, "Options File");
//    optfile = fopen(hbuf->filename_to_return, "rb");
//    
//    if (!(optfile = fopen (hbuf->filename_to_return, "wb")))
//    {
//        /* Report Error */
//        free_reference ( &(hbuf->filename_to_return));
//        return;
//    }
//    
//    free_reference ( &(hbuf->filename_to_return));
//
//    opt_put ((gboolean)prog_wdg->fname, prog_wdg->fname, OPT_BIN, optfile);
//    opt_put ((gboolean)cmd_wdg->fname, cmd_wdg->fname, OPT_CMD, optfile);
//    opt_put ((gboolean)hbuf->lblfile.fname, hbuf->lblfile.fname, OPT_LBL,
//            optfile);
//    opt_put (alt_defs, defs_wdg->fname, OPT_DEF, optfile);
//    opt_put (write_obj, asmout_wdg->fname, OPT_OBJ, optfile);
//    fprintf (optfile, "%s%d\n", OPT_RS, (int)isrsdos);
//    fprintf (optfile, "%s%d\n", OPT_CPU, (int)cputype);
//    fprintf (optfile, "%s%d\n", OPT_UPC, (int)upcase);
//    fprintf (optfile, "%s%d\n", OPT_PGW, (int)pgwdth);
//    fprintf (optfile, "%s%d\n", OPT_PGD, (int)pgdpth);
//    fprintf (optfile, "%s%d ", OPT_LST, (int)write_list);
//    
//    switch (write_list)
//    {
//        case LIST_FILE:
//            if (listing_wdg->fname)
//            {
//                if (strlen (listing_wdg->fname))
//                {
//                    fprintf (optfile, "%s\n", listing_wdg->fname);
//                }
//            }
//            break;
//        default:
//            fputc('\n', optfile);
//    }
//
//    fclose (optfile);
//}
//
//void opts_load (GtkAction *action, glbls *hbuf)
//{
//    FILE *optfile;
//    gchar buf[200];
//    
//    selectfile_open (hbuf, "n Options File", TRUE, NULL);  //Final parameter should point to a char * !!!!
//    optfile = fopen(hbuf->filename_to_return, "rb");
//    
//    if ( ! optfile)
//    {
//        /* Print Error message */
//        
//        free_reference ( &(hbuf->filename_to_return));
//        return;
//    }
//    
//    free_reference ( &(hbuf->filename_to_return));
//
//    while (fgets (buf, 200, optfile))
//    {
//        if ( ! strncmp (OPT_BIN, buf, strlen(OPT_BIN)))
//        {
//            opt_load_path (&(prog_wdg->fname), buf, NULL, OPT_BIN);
//            menu_do_dis_sensitize();
//            continue;
//        }
//        if ( ! strncmp (OPT_CMD, buf, strlen (OPT_CMD)))
//        {
//            opt_load_path (&(cmd_wdg->fname), buf, NULL, OPT_CMD);
//            hbuf->filename_to_return = g_strdup (cmd_wdg->fname);
//            do_cmdfileload (hbuf);        // NEED TO CHANGE PARAMETER
//            continue;
//        }
//
//        if ( ! strncmp (OPT_LBL, buf, strlen (OPT_LBL)))
//        {
//            opt_load_path (&(hbuf->lblfile.fname), buf, NULL, OPT_LBL);
//            hbuf->filename_to_return = g_strdup (hbuf->lblfile.fname);
//            do_lblfileload (hbuf);
//            continue;
//        }
//
//        if ( ! strncmp (OPT_DEF, buf, strlen(OPT_DEF)))
//        {
//            opt_load_path (&(defs_wdg->fname), buf, &alt_defs, OPT_DEF);
//            continue;
//        }
//        if ( ! strncmp (OPT_OBJ, buf, strlen (OPT_OBJ)))
//        {
//            opt_load_path (&(asmout_wdg->fname), buf, &write_obj, OPT_OBJ);
//            continue;
//        }
//        if ( ! strncmp (OPT_LST, buf, strlen (OPT_LST)))
//        {
//            gchar *start = buf;
//            gchar **splits;
//
//            start += strlen (OPT_LST);
//            splits = g_strsplit (start," ", -1);
//
//            if (splits[0])
//            {
//                sscanf (splits[0], "%d", &write_list);
//            }
//
//            if (write_list == LIST_FILE)
//            {
//                free_reference (&listing_wdg->fname);
//               
//                if (splits[1])
//                {
//                    g_strstrip (splits[1]);
//
//                    if (strlen(splits[1]))
//                    {
//                        listing_wdg->fname = g_strdup (splits[1]);
//                    }
//                }
//            }
//
//            g_strfreev (splits);
//            continue;
//        }
//
//        if (!opt_get_val (OPT_RS, buf, &isrsdos))
//        {
//            if (!opt_get_val (OPT_CPU, buf, &cputype))
//            {
//                if (!opt_get_val (OPT_UPC, buf, &upcase))
//                {
//                    if (!opt_get_val (OPT_PGW, buf, &pgwdth))
//                    {
//                        /* Here, quietly test the last one and just
//                         * ignore the line if this does not match
//                         */
//                        opt_get_val (OPT_PGD, buf, &pgdpth);
//                    }
//                }
//            }
//        }
//    }
//}
//
