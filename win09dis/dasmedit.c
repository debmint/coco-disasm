/* ************************************************************ *
 * dasmedit.c - handles editing os9disam control files          $
 * first created Thu, 09 Sep 2004 18:19:02 -0500                $
 * $Id::                                                        $
 * ************************************************************ */


#include "win09dis.h"
#include <commctrl.h>
#include <string.h>
#include <stdio.h>

GList *amode_list = NULL;
char *lblEdVals[3];

/* **************************************************************** *
 * List of standard addressing modes for insertion into the GList   *
 * for the gtkcombobox                                              *
 * **************************************************************** */

char *bounds_list[] = {"A - ASCII string",
                       "B - Byte data",
                       "W - Word data",
                       "S - Short Label data",
                       "L - Long Label data",
                        NULL};

/*  -------------------  generic functions first ------------------------ */

/* ******************************************************************** *
 * build_label_selector() - build a combobox and include the selections *
 * PASSED:  (1) combo - The combo box to fill                           *
 *          (2) modept - if not null, points to an array of strings to  *
 *                       add to the combobox.  If NULL, fill from       *
 *                       amode_list.                                    *
 * ******************************************************************** */

void
build_label_selector (HWND combo, char **modept)
{
    if (modept)
    {       /* We're building addressing mode from char array */
        
        while (*modept)
        {
            SendMessage (combo, CB_ADDSTRING, 0, (LPARAM)(*modept));
            ++modept;
        }
    }
    else
    {
        GList *glp;
        
        if ( ! amode_list)    /* never been initialized? */
        {
            amode_list = amode_init();
        }

        glp = amode_list;

        if (glp && glp->data)
        {
            do
            {
                SendMessage (combo, CB_ADDSTRING, 0, (LPARAM)(glp->data));
            } while ((glp = glp->next));
        }
    }
}

/* ----------------- end general functions begin locals ----------------- */

/* ************************************************************************ *
 * txtbuf_insert_line() - Inserts text on a new line after the line where   *
 *      the cursor is.  It automatically prepends a newline to the string,  *
 *      moves the cursor to the end of the current line, and inserts the    *
 *      text.                                                               *
 * Passed : (1) the GtkTextBuffer into which to insert the text.            *
 *          (2) a GString containing a NULL-terminated string (no newline)  *
 * ************************************************************************ */

void
txtbuf_insert_line (HWND t_buf, char *line)
{
    HANDLE hClip;
    HGLOBAL globMem;
    LPTSTR lockedMem;

// We may need to provide for cursor positioning....

    if ( ! OpenClipboard (t_buf))
    {
        return;
    }

    if ( ! EmptyClipboard())
    {
        CloseClipboard();
        return;
    }

    if ( ! (globMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, 
                                  strlen (line) + 1)))
    {
        CloseClipboard();
        return;
    }
    
    lockedMem = GlobalLock (globMem);
    strcpy (lockedMem, line);
    GlobalUnlock (globMem);
    
    if ( ! (hClip = SetClipboardData (CF_TEXT, globMem)))
    {
        return;
    }

    SendMessage (t_buf, WM_PASTE, 0, 0);
    EmptyClipboard ();
    CloseClipboard ();
/*    if (GlobalFree (globMem))
    {
        LPVOID lpMsgBuf;

        FormatMessage (
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL 
            );

            // Display the string.
            MessageBox( NULL, lpMsgBuf, "GetLastError",
                    MB_OK|MB_ICONINFORMATION );

            // Free the buffer.
            LocalFree( lpMsgBuf );
    }*/

}

/* **************************************************************** *
 * GetComboText() - Retrieves selected text from combo box, storing *
 *          in a buffer provided by the caller                      *
 * Returns: Pointer to string if there is text, NULL on zero-length *
 *          string, or -1 on error                                  *
 * NOTE:  This function allocates memory for the string, if there   *
 *        is text to retrieve.  If any value except -1 or NULL is   *
 *        returned, the string should be FREED!!!                   *
 * **************************************************************** */

static char *
GetComboText (HWND window)
{
    char *txt;
    int indx = SendMessage (window, CB_GETCURSEL, 0, 0);
    
    if ((indx != CB_ERR))
    {
        int txtlen = SendMessage (window, CB_GETLBTEXTLEN, (WPARAM)indx, 0);

        if (txtlen > 0)
        {
            if ( ! (txt = malloc (txtlen + 1)))
            {
                return (char *)(-1);
            }

            SendMessage (window, CB_GETLBTEXT, (WPARAM)indx, (LPARAM)txt);
            return txt;
        }
        else
        {
            return NULL;
        }
    }

    return (char *)(-1);
}

/* **************************************************************** *
 * on_bnds_define_response() - Handle 'OK' response for bounds      *
 *                             define                               *
 * **************************************************************** */

static void
on_bnds_define_response (HWND bndsDlg)
{
    char  line[100];
    char *tmpline;
    HWND window;

    window = GetDlgItem (bndsDlg, IDDB_BT);

    if ((tmpline = GetComboText (window)) < (char *)1)
    {
        return;
    }

    sprintf (line, "%c ", tmpline[0]);
    free (tmpline);
    
    if ((*line == 'S') || (*line == 'L'))
    {
        window = GetDlgItem (bndsDlg, IDDB_AM);
        tmpline = GetComboText (window);

        if ((tmpline == (char *)(-1)) || (tmpline == NULL))
        {
            return;
        }

        sprintf (&(line[strlen (line)]), "%c ", tmpline[0]);
        free (tmpline);
    
        window = GetDlgItem (bndsDlg, IDDB_OFST);
        tmpline = GetComboText (window);

        if ((tmpline != NULL) && (tmpline != (char *)-1))
        {
            sprintf (&(line[strlen (line)]), "(%s) ", tmpline);
            free (tmpline);
        }
    }
    
    GetWindowText (GetDlgItem (bndsDlg, IDDB_RNGE),
                    &(line[strlen (line)]),
                    sizeof (line) - strlen (line) - 1);

    if ((sizeof (line) - strlen (line)) > 2)
    {
        strcat (line, "\r\n");
    }

    txtbuf_insert_line (O9Dis.cmdfile.l_store, line);

//            doc_set_modified(&O9Dis.cmdfile, TRUE);
            
}

///* ******************************************************************** *
// * abort_warn() - pop up a dialog notifying that a function cannot      *
// *                continue                                              *
// * ******************************************************************** */
//
//void abort_warn (char *msg)
//{
//    GtkWidget *dialog;
//    gchar *warnmsg;
//
//    warnmsg = g_strconcat(msg, "\n\n", "Cannot continue with task", NULL);
//    
//    dialog = gtk_message_dialog_new (GTK_WINDOW(w_main),
//                                     GTK_DIALOG_DESTROY_WITH_PARENT |
//                                     GTK_DIALOG_MODAL,
//                                     GTK_MESSAGE_WARNING,
//                                     GTK_BUTTONS_OK,
//                                     warnmsg);
//    gtk_dialog_run (GTK_DIALOG (dialog));
//    g_free (warnmsg);
//    gtk_widget_destroy (dialog);
//}

/* **************************************************************** *
 * name_label_response() - callback function for exit from label    *
 *                  definition                                      *
 * **************************************************************** */

static void
name_label_response (HWND hWndDlg)
{
    LV_ITEM lvi;
    char buf[20];
    HWND combo = GetDlgItem (hWndDlg, IDRN_COMBO);

    // Insert a new row into Labels listview

    ZeroMemory (&lvi, sizeof (lvi));
    lvi.mask = LVIF_TEXT | LVIF_STATE;
    lvi.iItem = ListView_GetItemCount (hWndDlg);
    GetWindowText (GetDlgItem (hWndDlg, IDRN_LBLNAME), buf, sizeof (buf) - 1);
    lvi.pszText = buf;
    lvi.cchTextMax = sizeof (buf) - 1;
    ListView_InsertItem (O9Dis.lblfile.l_store, &lvi);

    lvi.mask = LVIF_TEXT;
    lvi.iSubItem = 1;

    // "equ"

    strcpy (buf, "equ");
    ListView_SetItem (O9Dis.lblfile.l_store, &lvi);
    ++lvi.iSubItem;

    // Address

    
    buf[0] = '$';
    GetWindowText (GetDlgItem (hWndDlg, IDRN_LBLADDR),
                                                &buf[1], sizeof (buf) - 2);
    ListView_SetItem (O9Dis.lblfile.l_store, &lvi);
    ++lvi.iSubItem;

    // Class

    SendMessage (combo, CB_GETLBTEXT, SendMessage (combo, CB_GETCURSEL, 0, 0),
                                                                (int)buf);
    buf[1] = '\0';
    ListView_SetItem (O9Dis.lblfile.l_store, &lvi);
    doc_set_modified (&O9Dis.lblfile, TRUE);
}

/* **************************************************************** *
 * GetListingSelRow() - Retrieves the selected row from the Lising  *
 *          and stores the text in an array provided by the caller  *
 * **************************************************************** */

int
GetListingSelRow (HWND lv)
{
    int row;

    if ((row = (ListView_GetNextItem (lv, -1,
                                        LVNI_SELECTED))) == -1)
    {
        MessageBox (lv, "Could not find a selected item",
                "Find Error!", MB_ICONERROR | MB_OK);
    }

    return row;
}

/* **************************************************************** *
 * BndsDefineProc () - Window procedure to handle messages from the *
 *              "Define Bounds" dialog box.                         *
 * **************************************************************** */

static BOOL CALLBACK
BndsDefineProc (HWND bndsDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            {
                HWND listview = O9Dis.list_file.l_store;
                int row,
                    col;
                char row_data[LST_NCOLS][10];

                if ( (row = GetListingSelRow (listview)) == -1)
                {
                    // Handle error
                }
                
                for (col = 1; col < LST_NCOLS; col++)
                {
                    ListView_GetItemText (listview, row, col,
                                    row_data[col], sizeof (row_data[col]));
                }

                 // Boundary Type dropdown list

                build_label_selector ( GetDlgItem (bndsDlg, IDDB_BT),
                                       bounds_list);

                // Addressing Mode dropdown list

                build_label_selector ( GetDlgItem (bndsDlg, IDDB_AM),
                                       NULL);
                // Address entry

                SetWindowText ( GetDlgItem (bndsDlg, IDDB_RNGE),
                                row_data[LST_ADR]);

                // Offset dropdown list
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    return EndDialog (bndsDlg, 0);
                case IDOK:
                    on_bnds_define_response (bndsDlg); 
                    return EndDialog (bndsDlg, 0);
                case IDDB_BT:
                    // Enable or disable AMode & offset according to
                    // whether bounds type is label or not
                    if ((HIWORD(wParam) == CBN_SELCHANGE))
                    {
                        BOOL enab = FALSE;
                        char bty[20];

                        GetWindowText ((HWND)lParam, bty, sizeof (bty) - 1);

                        if (strchr ("SL", bty[0]))
                        {
                            enab = TRUE;
                        }
                        
                        EnableWindow (GetDlgItem (bndsDlg, IDDB_AM), enab);
                        EnableWindow (GetDlgItem (bndsDlg, IDDB_OFST), enab);
                    }
                    return FALSE;
            }
    }

    return FALSE;
}

/* ******************************************************************** *
 * bnds_define_cb() - Callback function to handle boundary definitions  *
 *      The dialog and widget list are created upon first call to       *
 *      function and then kept for the duration of program.             *
 * ******************************************************************** */

void
bnds_define_cb (HWND hWnd, FILEINF *fdat)
{

    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_BNDSDEFINE),
                    fdat->l_store, BndsDefineProc);
}

/* **************************************************************** *
 * RenameLblProc() - Window Proc for the "Rename Label" dialog box  *
 * **************************************************************** */

static BOOL CALLBACK
RenameLblProc (HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            {
                HWND listview = O9Dis.list_file.l_store;
                HWND ctrlitem;
                int row,
                    col;
                char row_data[LST_NCOLS][10];
                char *cmd_mode;

                if ( (row = GetListingSelRow (listview)) == -1)
                {
                    // Handle error
                }
                
                for (col = 1; col < LST_NCOLS; col++)
                {
                    ListView_GetItemText (listview, row, col,
                                    row_data[col], sizeof (row_data[col]));
                }

                /* *************************
                 * Get addressing mode for current command
                 * ************************* */
                
                if ( ! ( cmd_mode =
                            get_addressing_mode (listview,
                                                 row_data[LST_OPCO],
                                                 row_data[LST_MNEM],
                                                 row_data[LST_OPER])) )
                {
                    cmd_mode = "L";
                }
            
                /* ******************** *
                 * Label name entry box *
                 * ******************** */

                if (strlen(row_data[LST_LBL]) )
                {
                    SetWindowText (GetDlgItem (hWndDlg, IDRN_LBLNAME),
                                        row_data[LST_LBL]);
                }

                ctrlitem = GetDlgItem (hWndDlg, IDRN_COMBO);
                SendMessage (ctrlitem, CB_SETCURSEL,
                        SendMessage (ctrlitem, CB_FINDSTRING, 0, 0), 0);

                SetWindowText (GetDlgItem (hWndDlg, IDRN_LBLADDR),
                                row_data[LST_ADR]);

                // Fill Label Class Combo box

                {
                    HWND lcCombo = GetDlgItem (hWndDlg, IDRN_COMBO);

                    build_label_selector (lcCombo, NULL);
                }
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    return EndDialog (hWndDlg, 0);
                case IDOK:
                    name_label_response (hWndDlg);
                    return EndDialog (hWndDlg, 0);
            }
    }

    return FALSE;
}

/* ************************************************************ *
 * rename_label() - callback function to handle label name mode *
 * ************************************************************ */

void
rename_label (HWND parent, FILEINF *fdat)
{

    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_RENAME_LBL),
                    fdat->l_store, RenameLblProc);
}

/* ******************************************************** *
 *  LblEditProc() - Window procedure for Label Edit Dialog  *
 * ******************************************************** */

static BOOL CALLBACK
LblEditProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int txtlen[3];
    int entID[3] = {ID_LBLED_NAME, ID_LBLED_ADDR, ID_LBLED_CLASS};
    int cnt;

    switch (Message)
    {
        case WM_INITDIALOG:
            {
                int count;

                build_label_selector ( GetDlgItem (hDlg, ID_LBLED_CLASS),
                                        NULL);

                for ( count = 0; count < 2; count++)
                {
                    if (lblEdVals[count])
                    {
                        SetWindowText (GetDlgItem (hDlg, entID[count]),
                                        lblEdVals[count]);
                    }
                }

                if (lblEdVals[2])
                {
                    SendMessage (GetDlgItem (hDlg, ID_LBLED_CLASS),
                                 CB_SELECTSTRING,
                                 (WPARAM)(-1),
                                 (LPARAM)lblEdVals[2]);
                }
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                int cnt;
                
                case IDCANCEL:
                    for (cnt = 0; cnt < 3; cnt++)
                    {
                        if (lblEdVals[cnt])
                        {
                            free (lblEdVals[cnt]);
                            lblEdVals[cnt] = NULL;
                        }
                    }

                    return EndDialog (hDlg, 0);
                case IDOK:
                    for (cnt = 0; cnt < 3; cnt++)
                    {
                        txtlen[cnt] = GetWindowTextLength (
                                                GetDlgItem (hDlg, entID[cnt]));

                        if ( (txtlen[cnt] == 0))
                        {
                            MessageBox (hDlg, "Please fill in ALL fields",
                                        "Warning!", MB_ICONWARNING | MB_OK);
                            return FALSE;
                        }

                        ++txtlen[cnt];  // Allow for terminating NULL
                    }
                    
                    if ( ! (lblEdVals[0] = od_memset (hDlg, txtlen[0])))
                    {
                        return FALSE;
                    }

                    GetWindowText (GetDlgItem (hDlg, ID_LBLED_NAME),
                                       lblEdVals[0], txtlen[0]);
                    
                    if ( ! (lblEdVals[1] = od_memset (hDlg, txtlen[1])))
                    {
                        if (lblEdVals[0])
                        {
                            free (lblEdVals[0]);
                            lblEdVals[0] = NULL;
                        }

                        return FALSE;
                    }
                    
                    GetWindowText (GetDlgItem (hDlg, ID_LBLED_ADDR),
                                       lblEdVals[1], txtlen[1]);

                    if ( ! (lblEdVals[2] = od_memset (hDlg, txtlen[2])))
                    {
                        for (cnt = 0; cnt < 3; cnt++)
                        {
                            if (lblEdVals[cnt])
                            {
                                free (lblEdVals[cnt]);
                                lblEdVals[cnt] = NULL;
                            }
                        }
                        
                        return FALSE;
                    }
                    
                    GetWindowText (GetDlgItem (hDlg, ID_LBLED_CLASS),
                                       lblEdVals[2], txtlen[2]);
                    lblEdVals[2][1] = '\0';
                    
                    if (lblEdVals[0] && lblEdVals[1] && lblEdVals[2])
                    {
                        //HWND lblview = GetParent (hDlg);
                        HWND lblview = O9Dis.lblfile.l_store;
                        LV_ITEM lvitm;
                        int fldindex;

                        ZeroMemory (&lvitm, sizeof (lvitm));
                        lvitm.mask = LVIF_TEXT;
                        lvitm.iItem = GetListingSelRow (lblview);

                        // If no selection, append to end

                        if (lvitm.iItem == -1)
                        {
                            lvitm.iItem = ListView_GetItemCount (lblview);
                        }
                        else
                        {
                            ++(lvitm.iItem);
                        }

                        /* add entry to Label tree_store */
                        lvitm.pszText = lblEdVals[0];
                        ListView_InsertItem (lblview, &lvitm);
                        fldindex = 0;
                        
                        for ( lvitm.iSubItem = 1;
                              lvitm.iSubItem < 4;
                              lvitm.iSubItem++)
                        {
                            if (lvitm.iSubItem != 1)
                            {
                                lvitm.pszText = lblEdVals[fldindex];
                            }
                            else
                            {
                                lvitm.pszText = "equ";
                            }
                            
                            ListView_SetItem (lblview, &lvitm);
                            ++fldindex;
                        }
                    }

                    return EndDialog (hDlg, 0);
            }
    }

    return FALSE;
}
/* **************************************************** *
 * lbl_edit_line() : create dialog for editing/adding   *
 *          line (row) to the Label list_store          *
 * Returns: button response from dialog                 *
 * **************************************************** */

//static void
//lbl_edit_line(HWND hLblWnd, char **label, char **addr, char **class)
//{
//    DialogBox (GetModuleHandle (NULL), IDD_LBLEDIT, hLblWnd, LblEditProc);
//}

/* ************************************************************ *
 * lbl_insert_line() : insert a  line in the label entry list   *
 * ************************************************************ */

void
lbl_insert_line (FILEINF *fdat)
{
    HWND lblview = fdat->l_store;

    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_LBLEDIT), lblview,
                        LblEditProc);

}
        
/* ************************************************************ *
 * lbl_delete_line() : delete a line in the label entry list    *
 * ************************************************************ */

void
lbl_delete_line (HWND lblview)
{
    int selected = GetListingSelRow (lblview);

    if (selected != -1)
    {
        ListView_DeleteItem (lblview, selected);
    }
}

/* ************************************************ *
 * lbl_properties() : edit an existing label entry  *
 * ************************************************ */

void
lbl_properties (HWND lblView)
{
    int selected = GetListingSelRow (lblView);

    if (selected != -1)
    {
        DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_LBLEDIT),
                                        lblView, LblEditProc);
    }
}
