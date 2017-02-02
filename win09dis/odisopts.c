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

/* ******************************************************************** *
 *                                                                      $
 * odisopts.c - setup options to pass to assembler                      $
 *                                                                      $
 * NOTE:  This newer version will create the widgets within a single    $
 *        table rather than stacking hboxes within the dialog->vbox     $
 *                                                                      $
 * $Id::                                                                $
 * ******************************************************************** */


//#include <string.h>
#include "win09dis.h"
#include "commctrl.h"

BOOL initPageFillClass(HINSTANCE hinst);
void WINAPI OnOptDlgInit(HWND hwnDlg);
BOOL CALLBACK OptDlgProc(HWND hwnDlg, UINT Message, WPARAM wParam, LPARAM lParam);
void set_dis_opts_cb(HWND hWnd);

// Widget ID's
#define DLG_FILES 201
#define DLG_APPEAR 202

#define C_PAGES 2

static HWND optsTabCtl;
static HWND subWin[C_PAGES];
static TC_ITEM tie;

struct fontsel_data {
    char * oldfont,
         * newfont;
};

WNDCLASSEX OptsPageClass;

// The radio buttons for the Formatted Listing output destination

static HWND radio_gui,
            radio_none,
            radio_file;

/* ************************************************************ *
 * odisopt_new_file () - Retrieves the name of a file to load   *
 *    If a file is selected, and is not the same as what is     *
 *    already stored, the old reference is freed and the new    *
 *    file is referenced.  Else the new reference is freed      *
 * ************************************************************ */

static void
odisopt_new_file (HWND parent, FILEINF *fdat, char *prompt)
{
    char *filename;

    if ((filename = selectfile_open (parent, prompt, FFT_MISC)))
    {
        SetWindowText (fdat->o_entry, filename);
        // It seems that SetWindowText does not set MODIFIED to TRUE
        SendMessage (fdat->o_entry, EM_SETMODIFY, TRUE, 0);
    }
}

/* **************************************************************** *
 * PgFillWndProc() - Callback for the contailer window that is used *
 *      to fill a page and contain the controls.                    *
 * We tried to use a "STATIC" class, but it seemed that it passed   *
 * the button clicks into the tab control, and I could not see how  *
 * to retrieve them, without subclassing the tab control.  This     *
 * seems to be just as simple (I hope).                             *
 * **************************************************************** */

static LRESULT CALLBACK
PgFillWndProc (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            return DefWindowProc (hWnd, Msg, wParam, lParam);
        case WM_DESTROY:
            return DefWindowProc (hWnd, Msg, wParam, lParam);
        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                case 0:         // Menu Command - or - BN_CLICKED
                    switch (LOWORD(wParam))
                    {
                        case IDB_PROG:
                            odisopt_new_file (hWnd, &O9Dis.binfile,
                                                "Program File");
                            return 0;
                        case IDB_CMD:
                            odisopt_new_file (hWnd, &O9Dis.cmdfile,
                                    "Command File");
                            return 0;
                        case IDB_OUT:
                            odisopt_new_file (hWnd, &O9Dis.asmout,
                                    "Assembly source file");
                            return 0;
                        case IDB_DEFS:
                        {
                            char *dirname = browse_for_directory (hWnd,
                                                            &O9Dis.defsfile);
                            if (dirname)
                            {
                                fname_replace (&O9Dis.defsfile, dirname);
                                SetWindowText (O9Dis.defsfile.o_entry,
                                                dirname);
                            }
                            return 0;
                        }
                        case IDB_FILE:
                            odisopt_new_file (hWnd, &O9Dis.list_out,
                                    "Listing Output");
                            return 0;
                        default:
                            break;
                    }
                    return FALSE;
                case 1:         // Toolbar sent it
                    return FALSE;
                default:
                    return FALSE;
            }
            break;
        default:
            return DefWindowProc (hWnd, Msg, wParam, lParam);
    }

    return FALSE;
}

BOOL
initPageFillClass (HINSTANCE hinst)
{
    if ( OptsPageClass.lpszClassName == NULL)
    {
        WNDCLASSEX *wcp = &OptsPageClass;     // For readability

        ZeroMemory (wcp, sizeof (OptsPageClass));
        wcp->cbSize = sizeof (OptsPageClass);
        wcp->lpfnWndProc = PgFillWndProc;
        wcp->hInstance = hinst;
        wcp->hCursor = LoadCursor (NULL, IDC_ARROW);
        wcp->hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wcp->lpszClassName = "PageFiller";
    }

    if ( ! RegisterClassEx (&OptsPageClass))
    {
        MessageBox (NULL, "Could not register the Page-Filler Class",
                    "Error!", MB_ICONERROR | MB_OK);
        return FALSE;
    }

    return TRUE;
}

static HWND
create_groupbox (HWND parent, char *txt, int left, int top, int rt, int bot)
{
    return CreateWindow ("BUTTON", txt, WS_CHILD | BS_GROUPBOX | WS_VISIBLE,
            left, top, rt, bot, parent, NULL, GetModuleHandle (NULL), NULL);
}

#define EDTHI 25        // Height of an edit box

static void
BuildFilesTab (HWND dlgWnd)
{
    RECT tabRect;
    int tabWid,
        tabHigh,
        grpHi,
        boxWdth;
    int curTop = 10,
        rtLeft;         // Left side of groupboxes on right-hand side
    HWND grpBx,
         txtEntry;
    
    GetClientRect (dlgWnd, &tabRect);
    tabWid = tabRect.right - (12);
    tabHigh = tabRect.bottom - (60);

    subWin[0] = CreateWindowEx (0, "PageFiller", "",
                    WS_CHILD | WS_VISIBLE,
                    6, 30, tabWid, tabHigh,
                    dlgWnd, NULL, GetModuleHandle (NULL), NULL);

    GetClientRect (subWin[0], &tabRect);    // Dimensions of tab itself
    grpHi = (tabRect.bottom / 3) - 2;

            /* ************************************************* *
             * Now build the components that fit into the window *
             * ************************************************* */

    boxWdth = tabWid/2 - 3;
    rtLeft = (tabWid/2) + 3;

            // Program module to disassemble

    grpBx = create_groupbox (subWin[0], "Program to disassemble",
                    0, curTop, boxWdth, grpHi);

    O9Dis.binfile.o_entry = CreateWindowEx (0, "EDIT", "",
            WS_CHILD | WS_BORDER | WS_VISIBLE,
            10, grpHi - EDTHI - 50, boxWdth - 20, EDTHI,
            subWin[0], (HMENU)(int)IDT_PROG, GetModuleHandle (NULL), NULL);

    O9Dis.binfile.browse_button =
        CreateWindowEx (0, "BUTTON", "Browse",
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            boxWdth - 100, curTop + grpHi - 35, 80, 25,
            subWin[0], (HMENU)(int)IDB_PROG, GetModuleHandle (NULL), NULL);

    if (O9Dis.binfile.fname)
    {
        SetWindowText (O9Dis.binfile.o_entry, O9Dis.binfile.fname);
    }
    
            // --- Command file ------------------

    grpBx = create_groupbox (subWin[0], "Command File",
                    rtLeft, curTop, boxWdth, grpHi);

    O9Dis.cmdfile.o_entry =
        CreateWindowEx (0, "EDIT", "", WS_CHILD | WS_BORDER | WS_VISIBLE,
                rtLeft + 10, grpHi - EDTHI - 50, boxWdth - 20, EDTHI,
                subWin[0], (HMENU)(int)IDT_CMD, GetModuleHandle (NULL), NULL);

    O9Dis.cmdfile.browse_button = CreateWindowEx (0, "BUTTON", "Browse",
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            rtLeft + boxWdth - 100, curTop + grpHi - 35, 80, 25,
            subWin[0], (HMENU)(int)IDB_CMD, GetModuleHandle (NULL), NULL);

    if (O9Dis.cmdfile.fname)
    {
        SetWindowText (O9Dis.cmdfile.o_entry, O9Dis.cmdfile.fname);
    }
    
    curTop += grpHi + 15;
            // Output file ( -o option)

    grpBx = create_groupbox (subWin[0], "Assembly Source File",
                    0, curTop, boxWdth, grpHi);

    txtEntry = CreateWindowEx (0, "STATIC",
                "If this entry is blank, no output file will be generated",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 30, boxWdth - 20, grpHi/5,
                grpBx, NULL, GetModuleHandle (NULL), NULL);

    O9Dis.asmout.o_entry =
        CreateWindowEx (0, "EDIT", "",
            WS_CHILD | WS_BORDER | WS_VISIBLE,
            10, curTop + grpHi - EDTHI - 50, boxWdth - 20, EDTHI,
            subWin[0], (HMENU)(int)IDT_OUT, GetModuleHandle (NULL), NULL);

    O9Dis.asmout.browse_button = CreateWindowEx (0, "BUTTON", "Browse",
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            boxWdth - 90, curTop + grpHi - 35, 80, 25,
            subWin[0], (HMENU)(int)IDB_OUT, GetModuleHandle (NULL), NULL);

    if (O9Dis.asmout.fname)
    {
        SetWindowText (O9Dis.asmout.o_entry, O9Dis.asmout.fname);
    }
    
            // Formatted listing options (right-hand side)
    
    grpBx = create_groupbox (subWin[0], "Formatted File Listing",
                    (tabWid/2) + 3, curTop, boxWdth,
                    tabRect.bottom - curTop);

    radio_gui = CreateWindowEx (0, "BUTTON", "Build for GUI",
                WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE | WS_GROUP,
                rtLeft + 10, curTop + 20, boxWdth - 20, 30,
                subWin[0], (HMENU)(int)IDR_GUI, GetModuleHandle (NULL), NULL);

    radio_none = CreateWindowEx (0, "BUTTON", "No output",
                WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
                rtLeft + 10, curTop + 50, boxWdth - 20, 30,
                subWin[0], (HMENU)(int)IDR_QUIET, GetModuleHandle (NULL), NULL);

    radio_file = CreateWindowEx (0, "BUTTON", "Output to file/stdout",
                WS_CHILD | BS_AUTORADIOBUTTON | WS_VISIBLE,
                rtLeft + 10, curTop + 90, boxWdth - 20, 30,
                subWin[0], (HMENU)(int)IDR_FILE, GetModuleHandle (NULL), NULL);

    // Set the appropriate button

    {
        HWND radio_btn;
        
        switch (write_list)
        {
            case LIST_GTK:
                radio_btn = radio_gui;
                break;
            case LIST_NONE:
                radio_btn = radio_none;
                break;
            case LIST_FILE:
                radio_btn = radio_file;
                break;
            default:
                radio_file = LIST_GTK;
                radio_btn = radio_gui;
                break;
        }

        SendMessage (radio_btn, BM_CLICK, 0, 0);
    }
    
    O9Dis.list_out.o_entry = CreateWindowEx (0, "EDIT", "",
                WS_CHILD | WS_BORDER | WS_VISIBLE,
                rtLeft + 10, curTop + 120, boxWdth - 20, EDTHI,
                subWin[0], (HMENU)(int)IDT_FILE, GetModuleHandle (NULL), NULL);

    O9Dis.list_file.browse_button = CreateWindowEx (0, "BUTTON", "Browse",
                WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
                rtLeft + boxWdth - 90, curTop + 160, 80, 25,
                subWin[0], (HMENU)(int)IDB_FILE, GetModuleHandle (NULL), NULL);

    if (O9Dis.list_file.fname)
    {
        SetWindowText (O9Dis.list_file.o_entry, O9Dis.list_file.fname);
    }
    
    curTop += grpHi + 15;

            // Path to Defs file
    grpBx = create_groupbox (subWin[0], "Path to \"Defs\"",
                    0, curTop, boxWdth, tabRect.bottom - curTop);

    txtEntry = CreateWindowEx (0, "STATIC",
                "If this entry is blank, the default path will be used",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 30, boxWdth - 20, grpHi/5,
                grpBx, NULL, GetModuleHandle (NULL), NULL);

    O9Dis.defsfile.o_entry =
        CreateWindowEx (0, "EDIT", "", WS_CHILD | WS_BORDER | WS_VISIBLE,
            10, tabRect.bottom - EDTHI - 50, boxWdth - 20, EDTHI,
            subWin[0], (HMENU)(int)IDT_DEFS, GetModuleHandle (NULL), NULL);

    O9Dis.defsfile.browse_button = CreateWindowEx (0, "BUTTON", "Browse",
            WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
            boxWdth - 100, tabRect.bottom - 35, 80, 25,
            subWin[0], (HMENU)(int)IDB_DEFS, GetModuleHandle (NULL), NULL);

    if (O9Dis.defsfile.fname)
    {
        SetWindowText (O9Dis.defsfile.o_entry, O9Dis.defsfile.fname);
    }
    
}

/* **************************************************************** *
 * UpdateFileEntry() - Updates a FILEINF filename entry if needed.  *
 * If text is retrieved, the widget is set to UnModified state.     *
 * **************************************************************** */

static BOOL
UpdateFileEntry (HWND parent, FILEINF *fdat)
{

    if (SendMessage (fdat->o_entry, EM_GETMODIFY, 0, 0))
    {
        char *editstring = "";
        int txtlen = GetWindowTextLength (fdat->o_entry);
        
        if (txtlen)
        {
            if ( !(editstring = malloc (txtlen + 1)))
            {
                MessageBox (parent, "Cannot allocate memory for filename",
                            "ERROR!", MB_ICONERROR | MB_OK);
                return FALSE;
            }

            GetWindowText (fdat->o_entry, editstring, txtlen + 1);

            if ((fdat->fname == NULL) || (strcmp (fdat->fname, editstring)))
            {
                free_reference (&(fdat->fname));
                fdat->fname = editstring;
            }
            else
            {
                free (editstring);  // Same file - don't need to replace
            }
        }
        else
        {
            free_reference (&(fdat->fname));
        }

        SendMessage (fdat->o_entry, EM_SETMODIFY, FALSE, 0);
        return TRUE;
    }

    return FALSE;
}

/* ************************************************************ *
 * opts_apply() - Apply any changes made during the Set Options *
 *      session.                                                *
 * ************************************************************ */

static void
opts_apply (HWND parent)
{
    // Parse through all the widgets and apply changes

    // Get the string values

    UpdateFileEntry (parent, &(O9Dis.binfile));
    menu_do_dis_sensitize (GetParent (parent));
    UpdateFileEntry (parent, &(O9Dis.cmdfile));
    UpdateFileEntry (parent, &(O9Dis.lblfile));
    UpdateFileEntry (parent, &(O9Dis.asmout));
    UpdateFileEntry (parent, &(O9Dis.defsfile));
    UpdateFileEntry (parent, &(O9Dis.list_out));

    // Retrieve page width/depth values

    PgWdth.set = SendMessage (PgWdth.btn, UDM_GETPOS, 0, 0);
    PgDpth.set = SendMessage (PgDpth.btn, UDM_GETPOS, 0, 0);

    // Now the checkboxes

    CpuType.set = SendMessage (CpuType.btn, BM_GETCHECK, 0, 0);
    RsDos.set = SendMessage (RsDos.btn, BM_GETCHECK, 0, 0);
    ShowZeros.set = SendMessage (ShowZeros.btn, BM_GETCHECK, 0, 0);
    UpCase.set = SendMessage (UpCase.btn, BM_GETCHECK, 0, 0);

    // And the Radio Button State

    if (SendMessage (radio_gui, BM_GETCHECK, 0, 0))
    {
        write_list = LIST_GTK;
    }
    else if ( SendMessage (radio_none, BM_GETCHECK, 0, 0))
    {
        write_list = LIST_NONE;
    }
    else
    {
        write_list = LIST_FILE;
    }
}

/* ************************************************************ *
 * build_updown_widget() - Creates an updown set and places it  *
 *              in the specified position.                      *
 * ************************************************************ */

static HWND
build_updown_widget (HWND parent, int posX, int posY, int txtID, int updnID)
{
    HWND buddy,
         updownWin;

    buddy = CreateWindowEx (0, "EDIT", "",
                    WS_CHILD | WS_BORDER | ES_NUMBER | ES_RIGHT | WS_VISIBLE,
                    posX, posY, 60, 20,
                    parent, (HMENU)(int)txtID, GetModuleHandle (NULL), NULL);

    updownWin = CreateUpDownControl (
                    WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                    posX, posY, 30, 30, parent, IDUD_PGWDTH,
                    GetModuleHandle (NULL), buddy, 300, 40, 0);
    return updownWin;
}

static void
BuildAppearTab (HWND dlgWnd)
{
    RECT tabRect;
    int tabWid,
        tabHigh,
        curTop = 10;
    HWND grpBx,
         scrollBx;

    GetClientRect (dlgWnd, &tabRect);
    tabWid = tabRect.right - (12);
    tabHigh = tabRect.bottom - (60);

    subWin[1] = CreateWindowEx (0, "PageFiller", "",
                    WS_CHILD,
                    6, 30, tabWid, tabHigh,
                    dlgWnd, NULL, GetModuleHandle (NULL), NULL);
    
    GetClientRect (subWin[1], &tabRect);    // Dimensions of tab itself

    grpBx = create_groupbox (subWin[1], "Appearance",
                    0, curTop, tabRect.right, tabRect.bottom/3);

    // Page width

    scrollBx = create_groupbox (grpBx, "Page Width", 30, 20, 100, 50);
    PgWdth.btn = build_updown_widget (subWin[1],
                                    40, curTop + 40, IDT_PGWDTH, IDUD_PGWDTH);
    if (PgWdth.set == 0)    // Not yet initialized
    {
        PgWdth.set = 80;
    }

    SendMessage (PgWdth.btn, UDM_SETPOS, 0, PgWdth.set);

    //  Page Depth
    
    scrollBx = create_groupbox (grpBx, "Page Depth", 140, 20, 100, 50);
    PgDpth.btn = build_updown_widget (subWin[1], 140, curTop + 40,
                                                IDT_PGDPTH, IDUD_PGDPTH);

    if (PgDpth.set == 0)    // Not yet initialized
    {
        PgDpth.set = 66;
    }

    SendMessage (PgDpth.btn, UDM_SETPOS, 0, PgDpth.set);

    //  Fold to upper case

    UpCase.btn = CreateWindowEx (0, "BUTTON", "Fold to Upper Case",
                        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                        30, curTop + 80, 200, 20, subWin[1],
                        (HMENU)(int)IDCB_UPCASE, GetModuleHandle (NULL), NULL);

     SendMessage (UpCase.btn,BM_SETCHECK,
            UpCase.set ? BST_CHECKED : BST_UNCHECKED, 0);

    // Show Zero offsets

    ShowZeros.btn = CreateWindowEx (0, "BUTTON", "Show Zero Offsets",
                    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                    30, curTop + 110, 200, 20, subWin[1],
                    (HMENU)(int)IDCB_ZEROS, GetModuleHandle (NULL), NULL);

    SendMessage (ShowZeros.btn, BM_SETCHECK,
            ShowZeros.set ? BST_CHECKED : BST_UNCHECKED, 0);

    //  ----- Types GroupBox ----------

    curTop = tabRect.bottom/3 + 10;
    
    grpBx = create_groupbox (subWin[1], "Types",
                    0, curTop, tabRect.right,
                    tabRect.bottom/4);

    CpuType.btn = CreateWindowEx (0, "BUTTON", "CPU = 6309",
                        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                        30, curTop + 30, 200, 20, subWin[1], 
                        (HMENU)(int)IDCB_CPU, GetModuleHandle (NULL), NULL);

    SendMessage (CpuType.btn, BM_SETCHECK,
            CpuType.set ? BST_CHECKED : BST_UNCHECKED, 0);


    RsDos.btn = CreateWindowEx (0, "BUTTON", "RS-DOS Binary",
                    WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                    30, curTop + 70, 200, 20, subWin[1], 
                    (HMENU)(int)IDCB_RSDOS, GetModuleHandle (NULL), NULL);

    SendMessage (RsDos.btn, BM_SETCHECK,
            RsDos.set ? BST_CHECKED : BST_UNCHECKED, 0);
}

/* ************************************************************ *
 * OnOptDlgInit () - First stage of the initialization of the   *
 *      Options dialog.                                         *
 * ************************************************************ */

void WINAPI OnOptDlgInit (HWND hwnDlg)
{
    RECT rcTab;

    // Create the tab control

    GetClientRect (hwnDlg, &rcTab);
    rcTab.bottom -= 50;  // Allow space for buttons
    optsTabCtl = CreateWindow (WC_TABCONTROL, "",
                WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                3, 3, rcTab.right - 6, rcTab.bottom,
                hwnDlg, NULL, GetModuleHandle (NULL), NULL);

    if (optsTabCtl == NULL)
    {
        MessageBox (hwnDlg, "Error creating tab control for Option Set Dialog",
                "Error !!!", MB_ICONERROR | MB_OK);
        SendMessage (hwnDlg, WM_CLOSE, 0, 0);
        return;
    }

    // Insert the two pages into the tab control
    
    ZeroMemory (&tie, sizeof (tie));
    tie.mask = TCIF_TEXT | TCIF_IMAGE;
    tie.iImage = -1;
    tie.pszText = "Files";
    TabCtrl_InsertItem (optsTabCtl , 0, &tie);
    tie.pszText = "Appearance";
    TabCtrl_InsertItem (optsTabCtl , 1, &tie);

    BuildFilesTab (optsTabCtl);
    BuildAppearTab (optsTabCtl);
}

/* **************************************************** *
 * OptDlgProc() - Callback for main dialog              *
 * **************************************************** */

BOOL CALLBACK OptDlgProc (HWND hwnDlg, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            OnOptDlgInit (hwnDlg);
            //Initialize the current selection
            TabCtrl_SetCurSel (optsTabCtl, 0);
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD (wParam))
            {
                case WM_QUIT:
                    EndDialog (hwnDlg, 0);
                    return 0;
                case IDC_CANCEL:
                    EndDialog (hwnDlg,0);
                    return 0;
                case IDC_APPLY:
                    opts_apply (hwnDlg);
                    return 0;
                case IDC_OK:
                    opts_apply (hwnDlg);
                    EndDialog (hwnDlg, 0);
                    return 0;
            }
            break;
        case WM_NOTIFY:
            {
                NMHDR *hdr = (NMHDR *)lParam;

                switch (hdr->code)
                {
                    case TCN_SELCHANGING:
                        break;
                    case TCN_SELCHANGE:
                        {
                            int newWin = TabCtrl_GetCurSel (optsTabCtl);
                            int oldWin = newWin ? 0 : 1;

                            ShowWindow (subWin[oldWin], SW_HIDE);
                            ShowWindow (subWin[newWin], SW_SHOW);
                        }
                        return TRUE;
                    default:
                        return TRUE;
                }
            }
                
            break;
        case WM_CLOSE:
            EndDialog (hwnDlg, 0);
            break;
        case WM_DESTROY:
            EndDialog (hwnDlg, 0);
            break;
        default:
            return FALSE;
    }

    return FALSE;
}

/* ********************************************************** *
 *                                                            *
 *           Dialog to Set Disassembler Options               *
 *                                                            *
 *                 Called from GtkAction                      *
 * Passed:  GtkAction Widget, the global hbuf pointer         *
 *                                                            *
 * ********************************************************** */

void
set_dis_opts_cb (HWND hWnd)
{
    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_OPTS), hWnd,
                    OptDlgProc);
}

///* ******************************************************************** *
// * font_sel_cb () - Main (top-level) callback for modifying the font of *
// *          one of the views.                                           *
// * ******************************************************************** */

//static void
//font_select_cb (GtkButton *btn, struct fontsel_data *w_sel)
//{
//    w_sel->newfont = NULL;
//    w_sel->oldfont = pango_font_description_to_string (w_sel->style->font_desc);

//    /* Save the font name for now - we may wish to eliminate this step
//     * later */
//    w_sel->newfont = (gchar *)gtk_font_button_get_font_name (
//                                        GTK_FONT_BUTTON(w_sel->font_btn));
//    gtk_widget_modify_font (selected_view,
//                        pango_font_description_from_string (w_sel->newfont));
//}

///* ******************************************************************** *
// * color_sel_cb () - Callback for when a color selection button is      *
// *          pressed.                                                    *
// * ******************************************************************** */

//static void
//color_select_cb (GtkButton *btn, struct fontsel_data *w_sel)
//{
//    gboolean is_fg;
//    const gchar *btn_name = gtk_widget_get_name (GTK_WIDGET(btn));
//    GdkColor colr;

//    is_fg = (g_ascii_strcasecmp (btn_name, "foreground") == 0);
//    gtk_color_button_get_color (GTK_COLOR_BUTTON(btn), &colr);

//    if (is_fg) {
//        gtk_widget_modify_text (selected_view, GTK_STATE_NORMAL, &colr);
//    }
//    else {
//        gtk_widget_modify_base (selected_view, GTK_STATE_NORMAL, &colr);
//    }
//}

///* ******************************************************************** *
// * set_list_ptr() - Callback for when a Listing selection radio button  *
// *          is toggled.                                                 *
// *          If it became active, set "selected_view" to the proper      *
// *          window.  If it became inactive, do nothing.                 *
// *          Also sets the colors in the color selection buttons.        *
// * ******************************************************************** */

//static void
//set_list_ptr (GtkWidget *rb, struct fontsel_data *w_sel)
//{
//    if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(rb)))
//    {
//        selected_view = g_hash_table_lookup (w_sel->list_to_widg,
//                                             gtk_widget_get_name (rb));
//        w_sel->style = gtk_widget_get_style (selected_view);
//        gtk_font_button_set_font_name (GTK_FONT_BUTTON(w_sel->font_btn),
//                    pango_font_description_to_string(w_sel->style->font_desc));
//        gtk_color_button_set_color (GTK_COLOR_BUTTON(w_sel->cb_bg),
//                                    &(w_sel->style->base[0]));
//        gtk_color_button_set_color (GTK_COLOR_BUTTON(w_sel->cb_fg),
//                                    &(w_sel->style->text[0]));
//    }
//}

///* ******************************************************************** *
// * reset_lists() - Resets a list window to the state in which it was on *
// *      entry to the procedure.  This is called when the "Cancel"       *
// *      button is pressed in the Fonts/Colors select.                   *
// * ******************************************************************** */

//static void
//reset_lists (FILEINF *inf)
//{
//    GtkStyle *style = gtk_widget_get_style (inf->tview);
//    gchar * name = pango_font_description_to_string (style->font_desc);

//    if ( name && g_ascii_strcasecmp (name, inf->fontname))
//    {
//        gtk_widget_modify_font (inf->tview,
//                pango_font_description_from_string (inf->fontname));
//    }

//    if (inf->txtcolor && ! gdk_color_equal (inf->txtcolor, &(style->text[0])))
//    {
//        gtk_widget_modify_text (inf->tview, GTK_STATE_NORMAL, inf->txtcolor);
//    }

//    if (inf->bakcolor && ! gdk_color_equal (inf->bakcolor, &(style->base[0])))
//    {
//        gtk_widget_modify_base (inf->tview, GTK_STATE_NORMAL, inf->bakcolor);
//    }
//}

///* ******************************************************************** *
// * replace_gdkcolor () - replaces the GdkColor if it is different       *
// * ******************************************************************** */

//static void
//replace_gdkcolor (GdkColor **oldcolor, GdkColor *newcolor)
//{
//    GdkColor * delcolor = NULL;

//    if (*oldcolor && ! gdk_color_equal (*oldcolor, newcolor))
//    {
//        delcolor = *oldcolor;
//    }

//    if ( ! *oldcolor ||  ! gdk_color_equal (*oldcolor, newcolor))
//    {
//        *oldcolor = gdk_color_copy (newcolor);
//    }

//    /* Does the following work ??? */

//    if (delcolor)
//    {
//        gdk_color_free (delcolor);
//    }
//}

///* ******************************************************************** *
// * update_lists() - update the FILEINF data passed as a parameter       *
// *      to reflect the current font and {fore,back}ground colors        *
// * ******************************************************************** */

//void
//update_lists (FILEINF *inf)
//{
//    GtkStyle *style = gtk_widget_get_style (inf->tview);
//    gchar *name;

//    name = pango_font_description_to_string (style->font_desc);

//    if (inf->fontname) {
//        if (g_ascii_strcasecmp (inf->fontname, name)) {
//            g_free (name);
//        }
//    }
//    else {
//        inf->fontname = "";      /* To avoid segfault */
//    }

//    if (g_ascii_strcasecmp (inf->fontname, name)) {
//        inf->fontname = pango_font_description_to_string (style->font_desc);
//    }

//    replace_gdkcolor (&(inf->txtcolor), &(style->text[0]));
//    replace_gdkcolor (&(inf->bakcolor), &(style->base[0]));
//}

///* ******************************************************************** *
// * cbtn_add_lbl() - Finds the GtkFrame in the gtk_color_button and set  *
// *          the label to that passed as the second parameter.           *
// * ******************************************************************** */

//static void
//cbtn_add_lbl (GtkWidget *btn, gchar *label)
//{
//    while (! GTK_IS_FRAME (btn))
//    {
//        btn = gtk_bin_get_child (GTK_BIN(btn));
//    }

//    gtk_frame_set_label (GTK_FRAME(btn), label);
//}

///* ******************************************************************** *
// * dlg_set_tips() - Parses the GList of Buttons in the dialog and sets  *
// *          tooltips for them.                                          *
// * ******************************************************************** */

//void
//dlg_set_tips (GtkWidget *btn, GtkDialog *dialog, GtkTooltips *tips)
//{
//    font_tips = gtk_tooltips_new();

//    switch (gtk_dialog_get_response_for_widget (dialog, btn))
//    {
//        case GTK_RESPONSE_OK:
//            gtk_tooltips_set_tip (font_tips, btn,
//                    "Exit session, keeping all changes", NULL);
//            break;
//        default:    /* Cancel */
//            gtk_tooltips_set_tip (font_tips, btn,
//                    "End session, rejecting all changes made\nduring session.\nAll windows will revert back to\ntheir state on entry into this session", NULL);
//    }
//}

///* ******************************************************************** *
// * fonts_main_dialog () - Callback for main menu choice to select a     *
// *      different font and/or color for the listing, commands, or       *
// *      labels display(s).                                              *
// *      This dialog will be destroyed at the end of usage, since it     *
// *      should not be called often, surely only once in a session.      *
// * ******************************************************************** */

//void
//fonts_main_dialog (GtkAction *action, glbls *globals)
//{
//    GtkWidget *fonts_main_dialog,
//              * hbox_main,
//              * frame,
//              * tmp_vbox,
//              * tmp_hbox,
//              * radiolist = NULL;
//    struct fontsel_data fs_data;
//    gint response;
//    GList *btnlist;

//    /* Build the dialog */

//    fonts_main_dialog = gtk_dialog_new_with_buttons (
//                            "Choose Font",
//                            GTK_WINDOW(w_main),
//                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
//                            "Cancel", GTK_RESPONSE_CANCEL,
//                            "OK",     GTK_RESPONSE_OK,
//                            NULL);
//    btnlist = gtk_container_get_children (
//                GTK_CONTAINER(GTK_DIALOG(fonts_main_dialog)->action_area));
//    g_list_foreach (btnlist, (GFunc)dlg_set_tips, fonts_main_dialog);
//    hbox_main = gtk_hbox_new (FALSE, 10);

//    selected_view = globals->list_file.tview;
//    fs_data.style = gtk_widget_get_style (selected_view);
//    /* Listings radio group on left-hand side */

//    /* Build a list_to_widg hash */
//    fs_data.list_to_widg = g_hash_table_new (g_str_hash, g_str_equal);
//    g_hash_table_insert (fs_data.list_to_widg, "listing",
//                                               globals->list_file.tview);
//    g_hash_table_insert (fs_data.list_to_widg, "cmds",
//                                               globals->cmdfile.tview);
//    g_hash_table_insert (fs_data.list_to_widg, "labels",
//                                               globals->lblfile.tview);
//    tmp_vbox = gtk_vbox_new (FALSE, 5);
//    gtk_container_set_border_width (GTK_CONTAINER(tmp_vbox), 10);
//    radiolist = gtk_radio_button_new_with_label (NULL, "Listings");
//    gtk_widget_set_name (radiolist, "listing");
//    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));
//    radiolist = gtk_radio_button_new_with_label_from_widget (
//                                                GTK_RADIO_BUTTON(radiolist),
//                                                "Commands");
//    gtk_widget_set_name (radiolist, "cmds");
//    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));
//    radiolist = gtk_radio_button_new_with_label_from_widget (
//                                                GTK_RADIO_BUTTON(radiolist),
//                                               "Labels");
//    gtk_widget_set_name (radiolist, "labels");
//    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));

//    /* This may be a temporary thing... */
//    /* Following is not necessary, but the list is originally set up in
//     * reverse order, so we pick the last button in the list to gt the first */

//    frame = gtk_frame_new ("Listings to alter");
//    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
//    gtk_container_set_border_width (GTK_CONTAINER(frame), 10);
//    gtk_container_add (GTK_CONTAINER(frame), tmp_vbox);
//    gtk_box_pack_start_defaults (GTK_BOX(hbox_main), frame);

//    /* Select Font/Color buttons on right-hand side */

//    /* The fonts button */
//    tmp_vbox = gtk_vbox_new (FALSE, 5);
//    gtk_container_set_border_width (GTK_CONTAINER(tmp_vbox), 10);
//    fs_data.font_btn = gtk_font_button_new ();
//    g_signal_connect (fs_data.font_btn, "font-set", G_CALLBACK(font_select_cb),
//                                               &fs_data);
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), fs_data.font_btn);

//    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), gtk_hseparator_new());

//    /* The colors buttons */

//    tmp_hbox = gtk_hbox_new (FALSE, 5);
//    gtk_container_set_border_width (GTK_CONTAINER(tmp_hbox), 10);

//    /* The background button */
//    fs_data.cb_bg = gtk_color_button_new ();
//    g_signal_connect (fs_data.cb_bg, "color-set", G_CALLBACK(color_select_cb),
//                                                  &fs_data);
//    gtk_color_button_set_title (GTK_COLOR_BUTTON(fs_data.cb_bg),
//                                "Background");
//    gtk_widget_set_name(fs_data.cb_bg, "background");
//    cbtn_add_lbl (fs_data.cb_bg, "Background");
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_hbox), fs_data.cb_bg);

//    /* The foreground button */
//    fs_data.cb_fg = gtk_color_button_new ();
//    g_signal_connect (fs_data.cb_fg, "color-set", G_CALLBACK(color_select_cb),
//                                                   &fs_data);
//    gtk_color_button_set_title (GTK_COLOR_BUTTON(fs_data.cb_fg),
//                                "Foreground");
//    gtk_widget_set_name(fs_data.cb_fg, "foreground");
//    cbtn_add_lbl (fs_data.cb_fg, "Foreground");
//    gtk_box_pack_start_defaults (GTK_BOX(tmp_hbox), fs_data.cb_fg);

//    frame = gtk_frame_new ("Select Font/Color");
//    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
//    gtk_container_set_border_width (GTK_CONTAINER(frame), 10);
//    gtk_container_add (GTK_CONTAINER(tmp_vbox), tmp_hbox);
//    gtk_container_add (GTK_CONTAINER(frame), tmp_vbox);

//    gtk_box_pack_start_defaults (GTK_BOX(hbox_main), frame);

//    /* Set up for the first radio button in the list-selection */

//    set_list_ptr (GTK_WIDGET(g_slist_last (gtk_radio_button_get_group (
//                            GTK_RADIO_BUTTON(radiolist)))->data), &fs_data);
//    gtk_color_button_set_color (GTK_COLOR_BUTTON(fs_data.cb_bg),
//                                &(fs_data.style->base[0]));
//    gtk_color_button_set_color (GTK_COLOR_BUTTON(fs_data.cb_fg),
//                                &(fs_data.style->text[0]));

//    /* Now pack all into the dialog vbox */
//    gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(fonts_main_dialog)->vbox),
//                                 hbox_main);
//    gtk_widget_show_all (hbox_main);

//    response = gtk_dialog_run (GTK_DIALOG(fonts_main_dialog));

//    switch (response)
//    {
//        case GTK_RESPONSE_OK:
//            update_lists (&(globals->list_file));
//            update_lists (&(globals->cmdfile));
//            update_lists (&(globals->lblfile));
//            break;
//        default:
//            reset_lists (&(globals->list_file));
//            reset_lists (&(globals->cmdfile));
//            reset_lists (&(globals->lblfile));
//    }

//    gtk_object_destroy (GTK_OBJECT(font_tips));
//    font_tips = NULL;
//    gtk_widget_destroy (fonts_main_dialog);
//}
