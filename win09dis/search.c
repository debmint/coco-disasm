/* ************************************************************************ *
 * search.c - Handles search routines for g09dis - the graphical interface  $
 *            for os9disasm.                                                $
 * $Id::                                                                    $
 * ************************************************************************ */

#include "win09dis.h"
#include <commctrl.h>
#include <string.h>
//#include <gtk/gtk.h>
//#include "g09dis.h"

static int numRadios;
static char *dlgTtl;
static char *oldcol,
            *oldsrchtxt;
static int oldexact;

// Temporary fix...
#ifndef HAVE_STRNCASECMP
#  define strncasecmp strncmp
#endif

///* struct srch_log: Keeps data for a treeview, for setting up a search. */
//
//struct srch_log
//{
//    GtkWidget * buttongrp;  /* The radio group for the treeviews column btns */
//    GtkWidget * radios_box; /* The vbox containing the buttons - to show/hide*/
//    FILEINF * tvdat;        /* The global data for the treeview */
//    gchar * last_srch;      /* The previous string searched for in this TV   */
//    GHashTable * radio_pos; /* Hash of names=>column position (to search)    */
//};
//
//static GtkTooltips *srch_tips;
//static GtkWidget * srch_dialog,
//                 * srch_entry;
//static GHashTable *allviews;
//static gboolean exact_match;  /* Toggle button for exact string match */

/* ************************************************************************ *
 * build_srch_radio_button_group () - Creates a list of radio buttons       *
 *      for search column choice and packs them into a GtkVBox.             *
 *      Also creates and sets up the log table which will hold data for     *
 *      subsequent calls to search this table.                              *
 * Returns: The VBox into which the radio buttons are packed.               *
 * ************************************************************************ */

static void 
build_srch_radio_button_group (HWND dlg)
{
//    struct srch_log * shown_radios;
    RECT parentBox,
         grpBox;
    int grpTop,
        grpHeight,
        grpLeft,
        grpWidth;
    HINSTANCE hInst = GetModuleHandle (NULL);

    GetWindowRect (dlg, &parentBox);
    GetWindowRect (GetDlgItem (dlg, ID_SRCH_RBBOX), &grpBox);
    grpTop = grpBox.top - parentBox.top - 30;
    grpHeight = grpBox.bottom - grpBox.top;
    grpWidth = grpBox.right - grpBox.left - 10;
    grpLeft = grpBox.left - parentBox.left;
    
    if (numRadios == 2)
    {
        SetWindowText (
            CreateWindowEx (0, "BUTTON", NULL,
                    WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | WS_GROUP,
                    grpLeft + 5, grpTop + (grpHeight/3), grpWidth, 10,
                    dlg, (HMENU)ID_RB_LABEL, hInst, NULL),
            "Label"); 
        SetWindowText (
            CreateWindowEx (0, "BUTTON", NULL,
                    WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
                    grpLeft + 5, grpTop + (2 * grpHeight/3), grpWidth, 10,
                    dlg, (HMENU)ID_RB_ADDR, hInst, NULL),
            "Address"); 
    }
    else
    {
        SetWindowText (
            CreateWindowEx (0, "BUTTON", NULL,
                    WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | WS_GROUP,
                    grpLeft + 5, grpTop + (grpHeight/4), grpWidth, 10,
                    dlg, (HMENU)ID_RB_ADDR, hInst, NULL),
            "Address"); 

        SetWindowText (
            CreateWindowEx (0, "BUTTON", NULL,
                    WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
                    grpLeft + 5, grpTop + (grpHeight/2), grpWidth, 10,
                    dlg, (HMENU)ID_RB_LABEL, hInst, NULL),
            "Label"); 
        
        SetWindowText (
            CreateWindowEx (0, "BUTTON", NULL,
                    WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
                    grpLeft + 5, grpTop + (3 * (grpHeight/4)), grpWidth, 10,
                    dlg, (HMENU)ID_RB_OPERAND, hInst, NULL),
            "Operand"); 
    }
}

static void
search_fwd (HWND mainDlg)
{
    LV_FINDINFO lvfi;
    char srchstr[100];
    int indx;

    ZeroMemory (&lvfi, sizeof (lvfi));
    lvfi.flags = LVFI_STRING | LVFI_PARTIAL;
    lvfi.psz = srchstr;

    if (GetWindowText (GetDlgItem (mainDlg, ID_SRCH_TXT),
                        srchstr, sizeof (srchstr)))
    {
        if ((indx = ListView_FindItem (O9Dis.list_file.l_store, -1, &lvfi))
                        == -1)
        {
            MessageBox (mainDlg, "No match found", "Error",
                    MB_ICONERROR | MB_OK);
        }
    }
}
    

/* ************************************************************************ *
 * do_search () - The generic search function for all the treeviews         *
 *      The main dialog is created on first call to search.                 *
 *      A vbox (cols_vbox) is packed with a vbox of column choice radio     *
 *      buttons for each treeview to search.                                *
 *      The appropriate set is shown on entry with gtk_widget_show(), and   *
 *      is again hidden before exit.                                        *
 * Passed : (1) - The Dialog box which retrievs search parameters           *
 *          (2) - The title for the dialog.                                 *
 *          (3) _ The Name of the radiogroup to display.                    *
 * ************************************************************************ */

static void
do_search (HWND dlg, HWND lView, BOOL fwd)
{
    BOOL exact;
    char srchTxt[50];
    int incr,
        startIdx,
        numItems;
    int row,
        endrow,
        col;

    numItems = ListView_GetItemCount (lView);

    if ( (numItems == 0))
    {
        return;
    }

    GetWindowText (GetDlgItem (dlg, ID_SRCH_TXT), srchTxt, sizeof (srchTxt));

    if (strlen (srchTxt) == 0)
    {
        MessageBox (dlg, "You must have some search text", "Error!",
                MB_ICONERROR | MB_OK);
        return;
    }
    
    exact = (SendMessage (GetDlgItem (dlg, ID_CBEXACT),
                                        BM_GETCHECK, 0, 0) == BST_CHECKED);
    incr =  fwd ? 1 : -1;
    startIdx = lv_get_cursor_selection (dlg);

    endrow = (incr == 1) ? (numItems - 1) : 0;

    // If no selection, start at beginning

    if (startIdx == -1)
    {
        startIdx = 0;
    }

    row = startIdx;
    
    // Get checked button

    if ((SendMessage (GetDlgItem (dlg, ID_RB_LABEL), BM_GETCHECK, 0, 0)
                                                    == BST_CHECKED))
    {
        col = (numRadios == 3) ? LST_LBL : LBL_LBL;
    }
    else if ((SendMessage (GetDlgItem (dlg, ID_RB_ADDR), BM_GETCHECK, 0, 0)
                                                    == BST_CHECKED))
    {
        col = (numRadios == 3) ? LST_ADR : LBL_ADDR;
    }
    else  // Operand - only in listing
    {
        col = LST_OPER;
    }

    row += incr;    // Start 1 past the current selection

    if (row == numItems)
    {
        row = 0;
    }
    else if ( row == -1)
    {
        row = numItems - 1;
    }
    
    while (1)
    {
        char data[100];

        ListView_GetItemText (lView, row, col, data, sizeof (data));

        if ((exact) && (! strcmp (data, srchTxt)))
        {
            break;
        }
        else
        {
            if ( ! (strncasecmp (data, srchTxt, strlen (srchTxt))))
            {
                break;
            }
        }

        // If we've looped back to the start with no match, quit

        if ( row == startIdx)
        {
            MessageBox (lView, "No match found", "Notice",
                        MB_ICONINFORMATION | MB_OK);
            return;
        }

        // Prepare for looping in case at end of list;
        if (row == endrow)
        {
            row = (endrow == 0 ? numItems : -1);
        }

        row += incr;
    }

    ListView_EnsureVisible (lView, row, FALSE);
    ListView_SetItemState (lView, row, LVIS_SELECTED, LVIS_SELECTED);
}

/* ******************************************************************** *
 * SrchDlgProc() - Window Procedure for Search Dialog                   *
 * ******************************************************************** */

static BOOL CALLBACK
SrchDlgProc (HWND hSrchDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static HWND lView;

    switch (Message)
    {
        case WM_INITDIALOG:
            lView = (HWND)lParam;
            build_srch_radio_button_group (hSrchDlg);
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog (hSrchDlg, 0);
                    return FALSE;
                case ID_SRCHBKWD:
                case ID_SRCHFWD:
                    do_search (hSrchDlg, lView, (LOWORD(wParam) == ID_SRCHFWD));
                    EndDialog (hSrchDlg, 0);
                    return FALSE;
            }
    }

    return FALSE;
}

/* **************************************************************** *
 * listing_srch () - callback for Action group entry to search the  *
 *          listing for a value.  This funcion does the setup for   *
 *          calling do_search(), the generic search function.       *
 * **************************************************************** */

void
listing_srch (FILEINF *fdat)
{
    numRadios = 3;
    dlgTtl = "Search in Listings"; 
    DialogBoxParam (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_SRCHDLG),
                                    fdat->l_store, SrchDlgProc, 
                                    (LPARAM)(fdat->l_store));
}

/* ******************************************************************** *
 * labels_srch () - callback for Action group entry to search the       *
 *          labels list for a value.  This funcion does the setup for   *
 *          calling do_search(), the generic search function.           *
 * ******************************************************************** */

void
labels_srch (FILEINF *fdat)
{
    numRadios = 2;
    dlgTtl = "Search in Labels"; 
    DialogBoxParam (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_SRCHDLG),
                                    fdat->l_store, SrchDlgProc,
                                    (LPARAM)(fdat->l_store));
}
