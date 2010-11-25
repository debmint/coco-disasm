#include <string.h>
#include <stdio.h>

// Define MAIN so that variables in win09dis.h will be defined from this file
#define MAIN
#include "win09dis.h"
#include <commctrl.h>

#define IDC_LIST_WIN 501
#define IDC_CMD_WIN  502
#define IDC_LBL_WIN  503

WNDPROC lpfnListViewWndProc;    // Original ListView Window proc

HWND topwindow;
const char g_szClassName[] = "windisWindowClass";

/* *********************************************************************** *
 * buildsubwindow () - function to build the listing, cmdfile, & labelfile *
 *                     windows inside the main window.                     *
 * Passed:  hwnd - the handle of the main window.                          *
 *          left, right, top, bottom - the coordinates of the new size for *
 *                                     the subwindow                       *
 *          winid - the ID of the subwindow                                *
 *          wintype - String name for the window (for error message)       *
 * *********************************************************************** */

HWND buildsubwindow (HWND hwnd, LPCSTR class_name, DWORD style_add,
                     int left, int top, int right, int bottom,
                     HMENU winID, char *wintype)
{
    HWND childwin;
    childwin = CreateWindowEx (WS_EX_CLIENTEDGE, class_name, "",
            WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL
            | WS_HSCROLL  | style_add, 
            left, top, right, bottom,
            hwnd, winID, GetModuleHandle (NULL), NULL);

    if (childwin == NULL)
    {
        char emsg[100];

        sprintf (emsg, "Could not create %s window", wintype);
        MessageBox (hwnd, emsg, "Error", MB_OK | MB_ICONERROR);
    }

    return childwin;
}

void
not_implemented (HWND hwindow)
{
    MessageBox (hwindow,
                "This function has not yet been implemented\n",
                "Not Implemented",
                MB_ICONSTOP | MB_APPLMODAL | MB_OK);
}

void
listview_insert_cols (HWND listview, int totcols, char **titles)
{
    LV_COLUMN colinfo;
    int cpos;

    ZeroMemory (&colinfo, sizeof (colinfo));
    
    colinfo.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    colinfo.cx = MAINWINWIDTH/(totcols * 2);

    for (cpos = 0; cpos < totcols; cpos++)
    {
        colinfo.pszText = titles[cpos];
        colinfo.iSubItem = cpos + 1;

        if (ListView_InsertColumn (listview,
                               cpos + 1, &colinfo) == -1)
        {
            char msg[80];
            sprintf (msg, "%s%d\r\n", 
                     "Failed to insert column #", cpos + 1);
            MessageBox (listview, msg, "Error!",
                    MB_ICONERROR | MB_OK);
        }
    }
}

/* ******************************************************************** *
 * window_quit() - Clean up before exit                                 *
 *          Checks for and notifies user about modified files           *
 * ******************************************************************** */

BOOL
window_quit (HWND hWnd)
{
    BOOL savelbl = FALSE,
         savecmd = FALSE,
         cmd_altered = SendMessage (O9Dis.cmdfile.l_store, EM_GETMODIFY, 0, 0);
    
    // First, check if BOTH are altered

    if ((cmd_altered) && (O9Dis.lblfile.altered))
    {
        switch (save_all_query (hWnd))
        {
            case IDCANCEL:
                return FALSE;
            case ID_SAVEALL:
                savelbl = savecmd = TRUE;
                break;
            default:    // ID_SELECTEM: We pick the individual files below...
                break;
        }
    }

    if ((cmd_altered) && ! (savecmd))
    {
        switch (save_warn_OW (hWnd, O9Dis.cmdfile.fname,
                                    "Command", FALSE))
        {
            case IDYES:
                savecmd = TRUE;
                break;
            default:    // The only other response is "NO"
                break;
        }
    }

    if ((O9Dis.lblfile.altered) && ! (savelbl))
    {
        switch (save_warn_OW (hWnd, O9Dis.lblfile.fname,
                                    "Label", FALSE))
        {
            case IDYES:
                savelbl = TRUE;
                break;
            default:    // The only other response is "NO"
                break;
        }
    }

    if ( ! (savecmd) && ! (savelbl))
    {
        return FALSE;
    }

    if (savecmd)
    {
        cmd_save (&(O9Dis.cmdfile));
    }

    if (savelbl)
    {
        lbl_save (&(O9Dis.lblfile));
    }
    return TRUE;
}

/* ******************************************************************** *
 * SubListViewProc() - Subclass procedure for ListViews.                *
 * PURPOSE:  Handle popup menu processes. The popup menus pass their    *
 *      WM_COMMAND messages to the parent window, (the ListViews, in    *
 *      this case.  ListViews pass the WM_COMMAND to their parent       *
 *      windows, but it appears they pass only Edit commands.  We'll    *
 *      process them in this callback.                                  *
 * ******************************************************************** */

static LRESULT CALLBACK
SubListViewProc (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                // Listing popup
                case ID_LSTP_SRCH:
                    listing_srch (&(O9Dis.list_file));
                    return 0;
                case ID_LSTP_OPEN:
                    load_listing (&(O9Dis.list_file));
                    return 0;
                case ID_LBLREN:
                    rename_label (hWnd, &(O9Dis.list_file));
                    return 0;
                case ID_DEFBNDS:
                    bnds_define_cb (hWnd, &(O9Dis.list_file));
                    return 0;
                case ID_SETAMODE:
                    adr_mode_cb (hWnd, &(O9Dis.list_file));
                    return 0;
                    // Label window popup
                case ID_LBLP_SRCH:
                    labels_srch (&(O9Dis.lblfile));
                    return 0;
                case ID_LBLP_OPEN:
                    load_lblfile (&(O9Dis.lblfile));
                    return 0;
                case ID_LBLP_SAVE:
                    lbl_save (&(O9Dis.lblfile));
                    return 0;
                case ID_LBLP_SAVEAS:
                    lbl_save_as (&(O9Dis.lblfile));
                    return 0;
                case ID_LBLP_INSRT:
                    lbl_insert_line (&(O9Dis.lblfile));
                    return 0;
                case ID_LBLP_DEL:
                    lbl_delete_line (O9Dis.lblfile.l_store);
                    return 0;
                case ID_LBLP_PROP:
                    lbl_properties (O9Dis.lblfile.l_store);
                    return 0;
            }
    }
    
    return CallWindowProc (lpfnListViewWndProc, hWnd, Message, wParam, lParam);
}

/* ==================================================================== *
 * WndProc() - Callback for Main Menu actions                           *
 *      This callback handles messages from the main window and all     *
 *      messages from the Listing, command, and labels windows          *
 * ==================================================================== */

LRESULT CALLBACK
WndProc ( HWND hwnd,
          UINT Message,
          WPARAM wParam,
          LPARAM lParam)
{
    switch (Message)
    {
        case WM_CREATE:
        {
            int wHalf, hHalf;
            char *list_ttls[7] =
                {"Line", "Addr", "OPC", "Pbytes", "Label", "Mnem", "Oper"};
            char *lbl_ttls[LBL_NCOLS] = {"Label", "equ", "Addr", "Class"};

            wHalf = MAINWINWIDTH/2;
            hHalf = MAINWINHEIGHT/2;

            // Listing window in left half

            O9Dis.list_file.l_store = buildsubwindow (hwnd, WC_LISTVIEW,
                               LVS_REPORT || LVS_SINGLESEL || LVS_SHOWSELALWAYS,
                               0, 0, wHalf - 5, MAINWINHEIGHT,
                               (HMENU)IDC_LIST_WIN, "Listing");
            listview_insert_cols (O9Dis.list_file.l_store,
                                  LST_NCOLS, list_ttls);

            // Subclass listview to process popup window messages

            lpfnListViewWndProc =
                (WNDPROC)SetWindowLong (O9Dis.list_file.l_store,
                                        GWL_WNDPROC,
                                        (DWORD)SubListViewProc);

            // Cmdfile window - top right half

            O9Dis.cmdfile.l_store = buildsubwindow (hwnd, "EDIT", ES_MULTILINE,
                            wHalf, 0, wHalf - 5, hHalf - 5,
                            (HMENU)IDC_CMD_WIN, "CmdFile");

            // Labels window bottom of right half

            O9Dis.lblfile.l_store = buildsubwindow (hwnd, WC_LISTVIEW,
                            LVS_REPORT,
                            wHalf, hHalf, wHalf - 5, hHalf - 5,
                            (HMENU)IDC_LBL_WIN, "LabelFile");
            listview_insert_cols (O9Dis.lblfile.l_store,
                                  LBL_NCOLS, lbl_ttls);
            // SubClass labels window
            SetWindowLong (O9Dis.lblfile.l_store,
                                GWL_WNDPROC,
                                (DWORD)SubListViewProc);
        }
        break;

        case WM_SIZE:
        {
            HWND hSubWin;
            RECT box;
            int hwid,hhi;

            GetClientRect (hwnd, &box);
            hwid = (box.right)/2;
            hhi  = (box.bottom)/2;

            hSubWin = GetDlgItem (hwnd, IDC_LIST_WIN);
            SetWindowPos (hSubWin, NULL,
                          0, 0, hwid - 5, box.bottom,
                          SWP_NOZORDER);

            hSubWin = GetDlgItem (hwnd, IDC_CMD_WIN);
            SetWindowPos (hSubWin, NULL,
                          hwid, 0, hwid - 5, hhi - 5,
                          SWP_NOZORDER);

            hSubWin = GetDlgItem (hwnd, IDC_LBL_WIN);
            SetWindowPos (hSubWin, NULL,
                          hwid, hhi, hwid - 5, hhi - 5,
                          SWP_NOZORDER);
        }
        break;
        
        case WM_COMMAND:
            switch (LOWORD (wParam))
            {
                case ID_LSTNGNEW:
                    list_store_empty (&(O9Dis.list_file));
                    break;
                case ID_CMDNEW:
                    clear_text_buf (&O9Dis.cmdfile);
                    break;
                case ID_LBLNEW:
                    list_store_empty (&(O9Dis.lblfile));
                    break;
                case ID_LSTNGOPEN:
                    load_listing (&(O9Dis.list_file));
                    break;
                case ID_CMDOPEN:
                    load_cmdfile (&(O9Dis.cmdfile));
                    break;
                case ID_LBLOPEN:
                    load_lblfile (&(O9Dis.lblfile));
                    break;
                case ID_CMDSAVE:
                    cmd_save (&(O9Dis.cmdfile));
                    break;
                case ID_LBLSAVE:
                    lbl_save (&(O9Dis.lblfile));
                    break;
                case ID_CMDSAVEAS:
                    cmd_save_as (&(O9Dis.cmdfile));
                    break;
                case ID_LBLSAVEAS:
                    lbl_save_as (&(O9Dis.lblfile));
                    break;
                case ID_DASMPROG:
                    run_disassembler (hwnd, &O9Dis);
                    break;
                case ID_DASMTOFILE:
                    dasm_list_to_file_cb (hwnd, &O9Dis);
                    break;
                case ID_AMLISTEDIT:
                    amode_list_edit_cb (hwnd);
                    break;
                case ID_DISOPTS:
                    set_dis_opts_cb (hwnd);
                    break;
                case ID_OPTSLOAD:
                    opts_load (hwnd, &(O9Dis));
                    break;
                case ID_OPTSSAVE:
                    opts_save (hwnd, &(O9Dis));
                    break;
                case ID_HLPABOUT:
                    {
                        DialogBox (GetModuleHandle (NULL),
                                   MAKEINTRESOURCE(IDD_ABOUT),
                                   hwnd, (DLGPROC)AboutDlgProc);  
                    }
                    break;
                case WM_QUIT:
                    window_quit (hwnd);
                    PostQuitMessage(0);
                    return 0;
            }

            return 0;
        case WM_NOTIFY:
            {
                NMHDR *pnmh = (NMHDR *)lParam;

                switch (pnmh->idFrom)
                {
                    case IDC_LBL_WIN:
                        switch (pnmh->code)
                        {
                            case LVN_ITEMCHANGED:
                                if ( ! O9Dis.lblfile.altered)
                                {
                                    O9Dis.lblfile.altered = TRUE;
                                }
                                break;
                            default:
                                break;
                        }
                        return 0;
                }
            }
            return DefWindowProc (hwnd, Message, wParam, lParam);
        case WM_CONTEXTMENU:
            switch (GetWindowLong ((HWND)wParam, GWL_ID))
            {
                case IDC_LBL_WIN:
                    onLblRowRButtonPress (hwnd, (HWND)wParam,
                                           LOWORD(lParam), HIWORD(lParam));
                    return 0;
                case IDC_LIST_WIN:
                    onListRowRButtonPress (hwnd, (HWND)wParam,
                                            LOWORD(lParam), HIWORD(lParam));
                    return 0;
                default:
                    return DefWindowProc (hwnd, Message,
                                            wParam, lParam);
            }

            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_CLOSE:
            window_quit (hwnd);
            PostQuitMessage (0);
            return 0;
        default:
            return DefWindowProc (hwnd, Message, wParam, lParam);
    }

    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

    InitCommonControls(); 
    
    if ( ! RegisterClassEx (&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Win09Dis",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, MAINWINWIDTH, MAINWINHEIGHT,
        NULL, NULL, hInstance, NULL);

    topwindow = hwnd;
    if (hwnd == NULL)
    {
        MessageBox (NULL, "Window Creation failed", "Error!",
                    MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if ( ! initPageFillClass (hInstance))
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    
    return Msg.wParam;
}
