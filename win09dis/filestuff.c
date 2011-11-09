/* ******************************************************************** *
 * filestuff.c - handles file I/O, selection, etc                       $
 * $Id::                                                                $
 * ******************************************************************** */

// Disable Security warnings in Microsoft SDK
#ifndef MINGW32
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include "win09dis.h"
#include <commctrl.h>
#include <shlobj.h>
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

static char *ff_CMD =
                "Command files\0*.cmd\0Label files\0*.lbl\0All Files\0*.*\0",
            *ff_LBL =
                "Label files\0*.lbl\0Command files\0*.cmd\0All Files\0*.*\0",
            *ff_MISC =
                "All Files\0*.*\0Command files\0*.cmd\0Label files\0*.lbl\0";
HMENU ListWinPopup;
HMENU LblWinPopup;

void
filestuff_cleanup (void)
{
    if (ListWinPopup)
    {
        DestroyMenu (ListWinPopup);
        ListWinPopup = NULL;
    }

    if (LblWinPopup)
    {
        DestroyMenu (LblWinPopup);
        LblWinPopup = NULL;
    }
}

/* ******************************************* *
 * File selection stuff                        *
 * ******************************************* */

///* *********************************************
// * ending_slash() append a "/" to the end of a *
// * directory pathname if not already there     *
// * *********************************************/
//
//char *
//ending_slash (const char * dirnm)
//{
//    if (!dirnm)
//    {
//        return strdup ("");
//    }
//
//    if (dirnm[strlen (dirnm) - 1] == DIRCHR)
//    {
//        return strdup (dirnm);
//    }
//    else
//    {
//        return g_strconcat (dirnm, DIRSTR, NULL);
//    }
//}

/* ************************************************************ *
 * menuInsertTxt () - Inserts a text menu item into a menu      *
 * Passed : (1) mnu - the menu into which to insert the item    *
 *          (2) minfo- the MENUITEMINFO structore to use        *
 *          (3) pos - the position to insert the item           *
 *          (4) mask - the fMask masks                          *
 *          (5) id   - the ID of the item                       *
 *          (6) itemdata - application-specific data            *
 *          (7) txt  - the text to display                      *
 * ************************************************************ */

BOOL
menuInsertTxt (HMENU mnu, MENUITEMINFO *minfo, int pos,
                int mask, int id, DWORD itemdata, char *txt)
{
    minfo->fMask = mask;
    minfo->fType = MFT_STRING;
    minfo->wID = id;
    minfo->dwItemData = itemdata;
    minfo->dwTypeData = txt;
    
    return InsertMenuItem (mnu, pos, TRUE, minfo);
}

/* ******************************************************** *
 * sepInsert() - Insert a separator into the menu           *
 * Passed - (1) mnu - the menu into which to insert item    *
 *          (2) minfo - the MENUITEMINFO structure          *
 *          (3) pos - the position                          *
 * ******************************************************** */

BOOL
sepInsert (HMENU mnu, MENUITEMINFO *minfo, int pos)
{
    minfo->fMask = MIIM_TYPE;
    minfo->fType = MFT_SEPARATOR;
    return InsertMenuItem (mnu, pos, TRUE, minfo);
}

/* **************************************************************** *
 * lv_get_cursor_selection() - Returns the item when the cursor is  *
 *      anywhere on the line.  I can't seem to be able to get the   *
 *      item if the cursor is not positioned over the first column  *
 *      so I use ListView_HitTest() with the x pos just a bit to    *
 *      the right of the window.                                    *
 * Returns: row or -1 on failure.                                   *
 * **************************************************************** */

int
lv_get_cursor_selection (HWND listviewWnd)
{
    LV_HITTESTINFO ht;
    RECT clrect;

    GetWindowRect (listviewWnd, &clrect);

    ht.flags = LVHT_ONITEM | LVHT_TORIGHT;
    ht.iItem = 0;
    GetCursorPos (&(ht.pt)); // Retrieve cursor position on the SCREEN
    ht.pt.x = 5;        // Position x coordinate just a bit to the right
    ht.pt.y -= clrect.top;  // Subtract y coordinate of window

    return ListView_HitTest (listviewWnd, &ht);

}

static HMENU
CreateListWinPopup (void)
{
    HMENU popmenu;
    MENUITEMINFO mii;
    int pos = 1;
    UINT itmMask = MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_TYPE;

    if ( ! (popmenu = CreatePopupMenu () ))
    {
        return NULL;
    }

    ZeroMemory ( &mii, sizeof (MENUITEMINFO));
    mii.cbSize = sizeof (MENUITEMINFO);
    mii.fState = MFS_ENABLED;

    if ((menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LSTP_SRCH, 0,
                            "Search Listing")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                         ID_LSTP_OPEN, 0,
                         "Open Listing")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LBLREN, 0,
                            "Rename Label")) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_DEFBNDS, 0,
                            "Define Bounds")) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                         ID_SETAMODE, 0,
                         "Set Addressing Mode")) &&
            (sepInsert (popmenu, &mii, pos++))) 
    {
        return popmenu;
    }
    else
    {
        DestroyMenu (popmenu);
        return NULL;
    }
}

/* ******************************************************************** *
 * onListRowRButtonPress() - Handles right-button click on a row in the *
 *      Listing display                                                 *
 * This function is called in response to the WM_CONTEXTMENU message    *
 * ******************************************************************** */

BOOL
onListRowRButtonPress (HWND parent, HWND lvWnd, int xpos, int ypos)
{
    if ( ! ListWinPopup)
    {
        ListWinPopup = CreateListWinPopup ();

        if ( ! ListWinPopup)
        {
            return FALSE;
        }
    }
    
    ListView_SetItemState (lvWnd, lv_get_cursor_selection (lvWnd),
            LVIS_SELECTED, LVIS_SELECTED);

    TrackPopupMenu (ListWinPopup, TPM_LEFTALIGN, xpos, ypos, 0, lvWnd, NULL);
    return TRUE;
}

static HMENU
CreateLblWinPopup (HWND parent)
{
    HMENU popmenu;
    MENUITEMINFO mii;
    int pos = 1;
    UINT itmMask = MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_TYPE;

    if ( ! (popmenu = CreatePopupMenu () ))
    {
        MessageBox (parent, "Failed to create Popup Menu", "Error!",
                    MB_ICONERROR | MB_OK);
        return NULL;
    }

    ZeroMemory ( &mii, sizeof (MENUITEMINFO));
    mii.cbSize = sizeof (MENUITEMINFO);
    mii.fState = MFS_ENABLED;

    if ((menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LBLP_SRCH, 0,
                            "Search Labels")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                         ID_LBLP_OPEN, 0,
                         "Open Label File")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LBLP_SAVE, 0,
                            "Save Lbl file")) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LBLP_SAVEAS, 0,
                            "Save Lbl file as...")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                            ID_LBLP_INSRT, 0,
                            "Insert new line after")) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                         ID_LBLP_DEL, 0,
                         "Delete Line")) &&
            (sepInsert (popmenu, &mii, pos++)) &&
            (menuInsertTxt (popmenu, &mii, pos++, itmMask,
                         ID_LBLP_PROP, 0,
                         "Properties")))
    {
        return popmenu;
    }
    else
    {
        MessageBox (parent, "Failed to insert a popup menu subitem", "Error!",
                MB_ICONERROR | MB_OK);
        DestroyMenu (popmenu);
        return NULL;
    }
}

/* ******************************************************************** *
 * onLblRowRButtonPress() - Handles right-button click on a row in the  *
 *      Labels window display                                           *
 * This function is called in response to the WM_CONTEXTMENU message    *
 * ******************************************************************** */

BOOL
onLblRowRButtonPress (HWND parent, HWND lvWnd, int xpos, int ypos)
{
    if ( ! LblWinPopup)
    {
        if ( ! (LblWinPopup = CreateLblWinPopup (parent)))
        {
            return FALSE;
        }
    }
    
    ListView_SetItemState (lvWnd, lv_get_cursor_selection (lvWnd),
            LVIS_SELECTED, LVIS_SELECTED);

    TrackPopupMenu (LblWinPopup, TPM_LEFTALIGN, xpos, ypos, 0, lvWnd, NULL);
    return TRUE;
}

/* ************************************************************ *
 * od_memset() - Convenience function for malloc():  allocates  *
 *      memory using malloc(), but issues an error message on   *
 *      failure.                                                *
 * ************************************************************ */

void *
od_memset (HWND hWnd, int memsize)
{
    void *mem;

    mem = malloc (memsize);

    if ( ! mem)
    {
        MessageBox (hWnd, "Cannot allocate requested memory",
                "Memory Error !", MB_ICONERROR | MB_OK);
    }

    return mem;
}

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

/* **************************************************************** *
 * browse_for_directory() - Browse for a folder name.               *
 *      We use the shell extension browser, as I don't know how to  *
 *      do it any other way.  It seems complicated, but hopefully,  *
 *      we have it right.  And hopefully, we got all the memory     *
 *      freed correctly...                                          *
 * **************************************************************** */

char *
browse_for_directory (HWND hWnd, FILEINF *fdat)
{
    BROWSEINFO bi;
    LPMALLOC g_pMalloc;
    LPITEMIDLIST pidlBrowse;
    char dispName[MAX_PATH + 1];

    if (SUCCEEDED(SHGetMalloc (&g_pMalloc)))
    {
    
        ZeroMemory (&bi, sizeof (bi));
        
        bi.hwndOwner = hWnd;
        bi.pszDisplayName = dispName;

        pidlBrowse = SHBrowseForFolder (&bi);

        if (pidlBrowse)
        {
            if ( ! SHGetPathFromIDList (pidlBrowse, dispName))
            {
                dispName[0] = '\0';
            }

            g_pMalloc->lpVtbl->Free (g_pMalloc, pidlBrowse);
        }
        else
        {
            dispName[0] = '\0';  // In case SHBrowseForFolder did something
        }
        
        g_pMalloc->lpVtbl->Release (g_pMalloc);
    }

    return (strlen (dispName)) ? dispName : NULL;
}

static char *
filetypefilter (int fty)
{
    switch (fty)
    {
    case FFT_CMD:
        return ff_CMD;
    case FFT_LBL:
        return ff_LBL;
    case FFT_MISC:
        return ff_MISC;
    default:
        return "All files\0*.*\0";
    }
}

/* **************************************************************** *
 * selectfile_open() - Select a file to open                        *
 * Passed:  (1) - Handle of parent window                           *
 *          (2) - Type (string for prompt                           *
 *          (3) - filtertype - the FFT_* type of file for extension *
 *                          match.                                  *
 *                                                                  *
 * Returns: Ptr to chosen filepath, or NULL if no choice made       *
 * Runs GetOpenFilename() Dialog to obtain a file choice            *
 * Allocates storage for the path string.  It is up to the caller   *
 * to set the proper pointer.                                       *
 * **************************************************************** */

char *
selectfile_open ( HWND hwnd,
                  const char *type,
                  int filtertype)
{
    char *filename = NULL;
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory (&ofn, sizeof (ofn));

    // NOTE: We may wish to make this structure global and simply
    // update it when calling.

    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = hwnd;
    
    ofn.lpstrFilter = filetypefilter (filtertype);

    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof (szFileName) - 1;
    ofn.lpstrTitle = type;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if ( ! GetOpenFileName (&ofn))
    {
        return NULL;
    }

    if ( ! (filename = _strdup (ofn.lpstrFile)))
    {
        MessageBox (hwnd, "Memory allocation failure", "Error!",
                MB_ICONERROR | MB_OK);
        return NULL;
    }

    strcpy (filename, ofn.lpstrFile);

    return filename;
}

/* ************************************************************ *
 * selectfile_save () -select a file to save                    *
 * Passed : (1) parent - Window which will own dialog           *
 *          (2) dest   - pointer to char * which will reference *
 *                       the filename                           *
 *                       WARNING !!! Dest MUST BE NULL as this  *
 *                       function will attempt to free () the   *
 *                       memory !!!!!                           *
 * Returns: Pointer to new name on selection or                 *
 *          NULL if no selection is made                        *
 * ************************************************************ */

/* ************************************************************ *
 * Memory is allocated for the selected pathname and is placed  *
 * in "dest" unless the path strings are the same.              *
 * If "dest" points to a string, this memory is freed.          *
 * ************************************************************ */

char *
selectfile_save (HWND parent, char **dest, int filtertype)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory (&ofn, sizeof (ofn));

    // NOTE: We may wish to make this structure global and simply
    // update it when calling.

    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = parent;
    ofn.lpstrFilter = filetypefilter (filtertype);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT; 

    if (GetSaveFileName (&ofn))
    {
        char *fnam;

        if ((*dest == NULL) || (strcmp (*dest, ofn.lpstrFile) != 0))
        {

            if ( ! (fnam = malloc (strlen (ofn.lpstrFile) + 1)))
            {
                MessageBox (parent, "Memory allocation failure", "Error!",
                        MB_ICONERROR | MB_OK);
                return NULL;
            }

            strcpy (fnam, ofn.lpstrFile);
            free_reference (dest);
            *dest = fnam;
        }

        return *dest;
    }

    return NULL;
}

/* ****************************** *
 * Text Buffer handling stuff     *
 * ****************************** */

/* ************************************************************ *
 * strstrip () - strips leading whitespaces and trailing        *
 *               returns and newlines from string.              *
 *               For leading whitespaces, the characters in the *
 *               string are shifted back to the begin of string *
 * Returns: The original pointer to the string begin            *
 *          This is so that this function can be used inline    *
 * ************************************************************ */

char *
strstrip (char *ptr)
{
    char *nxtchr;
    
    while ( (ptr[strlen (ptr) - 1] == '\r') ||
            (ptr[strlen (ptr) - 1] == '\x0a') ||
            (ptr[strlen (ptr) - 1] == ' '))
    {
        ptr[strlen (ptr) - 1] = '\0';
    }
    
    while ((*ptr == ' ') || (*ptr == '\r') || (*ptr == '\t'))
    {
        nxtchr = ptr;

        do
        {
            *nxtchr = nxtchr[1];
            ++nxtchr;
        } while (*nxtchr);
    }

    return ptr;
}

static int savestatus;

static BOOL CALLBACK
SaveAllQueryDlgProc (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                case ID_SAVEALL:
                case ID_SELECTEM:
                    savestatus = LOWORD(wParam);
                    EndDialog (hWnd, 0);
                default:
                    return FALSE;
            }
    }

    return FALSE;
}

/* ************************************************************ *
 * save_all_query() - notify user that multiple files (might)   *
 *      need saving returns choice of NONE, ALL, or USER SELECT *
 * ************************************************************ */

int
save_all_query (HWND hWnd)
{
    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_WINDOWQUIT),
                                            hWnd, SaveAllQueryDlgProc);
    return savestatus;
}

/* ******************************************************** *
 * doc_set_modified():                                      *
 *                                                          *
 *     sets the FILEINF->altered flag to value              *
 *     if it is already this value, do nothing              *
 * ******************************************************** */

void
doc_set_modified (FILEINF *doc, BOOL value)
{
    if (doc->altered != value)
    {
        doc->altered = value;
    }
}

/* ***************************************************** *
 * save_warn_OW() - Alert user that a single altered but *
 *     not-saved buffer is about to be overwritten       *
 * Returns: Result of MessageBox() query                 *
 * ***************************************************** */

int
save_warn_OW (HWND hWnd, char *filename, char *type, BOOL can_cancel)
{
    int MB_BUTTONSET;
    char *warnmsg;
    char *fn;
    const char *fmt =
        "About to discard altered %s file buffer\nDo you wish to save to %s\n";
    int retval;

    MB_BUTTONSET= can_cancel ? MB_YESNOCANCEL : MB_YESNO;

    if (filename)
    {
        fn = filename;
    }
    else {
        fn = "(Untitled)";
    }

    if (! (warnmsg = od_memset (hWnd,
                strlen (fmt) + strlen (type) * strlen (fn) + 20)))
    {
        return 0;
    }
    
//#ifdef MINGW32
    sprintf (warnmsg, fmt, type, fn);
/*#else
    sprintf_s (warnmsg, sizeof (warnmsg), fmt, type, fn);
#endif*/

    retval = MessageBox (hWnd, warnmsg, "Warning!",
                               MB_ICONWARNING | MB_BUTTONSET);
    free (warnmsg);
    return retval;
}

/* ************************************************ *
 * wrt_lbl_file() : save a label buffer to a file   *
 * ************************************************ */

static void
wrt_lbl_file (FILEINF *fdat, char **newfile)
{
    int itemCount;

    /* TODO: rename file if it exists? */

    if ((itemCount = ListView_GetItemCount (fdat->l_store)))
    {
        FILE *outfile;
        LV_ITEM lvi;
        char buf[LBL_NCOLS][100];  // Allow long strings for comments
        int col,
            itm;

        if ( ! (outfile = fopen (*newfile, "wb")))
        {
            const char *fmt = "Cannot open file %s for write!";
            char *mesg;
            int memreq = strlen (fmt) + strlen (*newfile) + 1;

            if ( ! (mesg = od_memset (fdat->l_store, memreq)))
            {
                return;
            }

#ifdef MINGW32
            sprintf (mesg, fmt, *newfile);
#else
            sprintf_s (mesg, memreq, fmt, *newfile);
#endif
            MessageBox (fdat->l_store, mesg, "Error!", MB_ICONERROR | MB_OK);
            free (mesg);
            return;
        }

        ZeroMemory (&lvi, sizeof (lvi));

        lvi.mask = LVIF_TEXT;
        lvi.cchTextMax = sizeof (buf[0]);

        for (itm = 0; itm < itemCount; itm++)
        {

            lvi.iItem = itm;

            for (col = 0; col < LBL_NCOLS; ++col)
            {
                lvi.iSubItem = col;
                lvi.pszText = buf[col];

                ListView_GetItem (fdat->l_store, &lvi);
            }

            if ( !strchr(buf[0], '*'))
            {
                fprintf (outfile,"%s equ %s %c\n",
                        buf[0],buf[2],*buf[3]);
            }
            else
            {
                fprintf (outfile, "%s %s\n", buf[0], buf[1]);
            }
        }
    
        fclose (outfile);
    }

    doc_set_modified (fdat, FALSE);
}

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

    if (fdat->altered == TRUE)
    {
        switch (save_warn_OW (GetParent (mytv), fdat->fname,
                                        "Label", TRUE))
        {
            case IDNO:      /* discard changes */
                break;
            case IDCANCEL:  /* cancel overwrite */
                return;
                break;
            case IDYES:     /* save changes */
                /* Note: in calling wrt_lbl_file, pass GtkWidget *window
                 * as a dummy - it won't be needed */
                lbl_save (fdat);
                break;
        }
    }

    /* clear out current list_store */

    ListView_DeleteAllItems (mytv);
    fdat->altered = FALSE;
}

/* ************************************************ *
 * save_text() : save a text buffer to a file       *
 * Passed : (1) Edit window containing text to save *
 *          (2) the name of the file to save to     *
 * ************************************************ */

static BOOL
save_text (HWND hEdit, char *newfile)
{

    HANDLE hFile;
    BOOL bSuccess = FALSE;
    DWORD dwTextLength;

    hFile = CreateFile (newfile, GENERIC_WRITE, 0, NULL,
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
            if (GetWindowText (hEdit, pszText, dwBufferSize))
            {
                DWORD dwWritten;

                if (WriteFile (hFile, pszText, dwTextLength, &dwWritten, NULL))
                {
                    bSuccess = TRUE;
                }
            }

            GlobalFree (pszText);
        }
    }

    CloseHandle (hFile);
    SendMessage (hEdit, EM_SETMODIFY, FALSE, 0);
    return bSuccess;
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
    if (SendMessage (fdat->l_store, EM_GETMODIFY, 0, 0) == TRUE)
    {
        switch (save_warn_OW (GetParent (fdat->l_store), 
                                 fdat->fname, "Command", TRUE))
        {
            case IDNO:         /* discard changes */
                break;
            case IDCANCEL: /* cancel overwrite */
                return FALSE;
                break;
            case IDYES: /* save changes */
                cmd_save (fdat);
                break;
        }
    }

        
    SendMessage (fdat->l_store, EM_SETSEL, 0, -1);
    SendMessage (fdat->l_store, WM_CLEAR, 0, 0);
    SendMessage (fdat->l_store, EM_SETMODIFY, FALSE, 0);

    return TRUE;
}

/* ******************************************************************** *
 * load_text() - Loads a text file and places it into the edit buffer   *
 * ******************************************************************** */

BOOL
load_text (FILEINF *fdat, char *newfile)
{
    register FILE *infile;
    char buffer[500];
    char *pszFileText;
    struct stat cmd_stat;
    BOOL bSuccess = FALSE;

    clear_text_buf (fdat);

    if (newfile && fdat)
    {
        free_reference ( &(fdat->fname));
        fdat->fname = newfile;
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
        doc_set_modified (fdat, FALSE);
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
mswin_load_text (FILEINF * fdat, char *newfile)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    clear_text_buf (fdat);

    if (newfile)
    {
        free_reference ( &(fdat->fname));
        fdat->fname = newfile;
    }

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
load_list_tree (FILEINF *fdat, HANDLE infile)
{
    char  buffer[500],
         *bufpt;
    int itemcount;
    DWORD rdCount;

    itemcount = 0;

    bufpt = buffer;
    while (ReadFile (infile, bufpt, 1, &rdCount, NULL))
    {
        // gcc complained about strpt being uninitialized, so
        // we initialize it...
        LV_ITEM lv;

        // ReadFile returns non-zero on EOF, so we need to check for this
        if (rdCount == 0)
        {
            break;
        }

        // Ignore carriage returns
        if ( *bufpt == '\x0d')
        {
            continue;
        }

        if ( *bufpt != '\x0a')
        {
            ++bufpt;
            continue;
        }

        // We Are at the end of a line

        *bufpt = '\0';
        
        // Skip past whitespaces
        
        bufpt = strstrip (buffer);

        if (strlen (buffer))
        {
            char *splits[LST_NCOLS];
            int   fldnum;
            unsigned int place;
            BOOL illeg;
            
            //MessageBox (NULL, buffer, "Info", MB_ICONERROR|MB_OK);
            /* Assure that there are enough fields */

            strcat (buffer,"\t\t\t\t\t\t\t");

            // Split fields into an array

            for (fldnum = 0; fldnum < LST_NCOLS; fldnum++)
            {
                // Substitute NULL for next tab (end of this field

                splits[fldnum] = bufpt;
                bufpt = strchr (bufpt, '\t');

                *(bufpt++) = '\0';
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
                bufpt = buffer;     // Non-printable line, so reset
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
                    char mesg[50];

#ifdef MINGW32
                    sprintf (mesg, "Failed to insert item - %d,%d",
                                  itemcount - 1, fldnum);
#else
                    sprintf_s (mesg, sizeof (mesg),
                                    "Failed to insert item - %d,%d",
                                    itemcount - 1, fldnum);
#endif
                    MessageBox (fdat->l_store, mesg, "Error!",
                               MB_ICONERROR | MB_OK);
                }
            for (fldnum = 0; fldnum < LST_NCOLS; fldnum++)
            {
                ListView_SetItemText (fdat->l_store, itemcount - 1,
                            fldnum, splits[fldnum]); 
            }   // for (fldnum...
        }

        bufpt = buffer;     // Set up to get next line
    }       // while (fgets...

    doc_set_modified (fdat, (BOOL)FALSE);
}

/* ******************************************************************** *
 * fname_replace () - Replaces the fname if the new name is different   *
 * NOTE: the newname MUST be a name that can be freed                   *
 * ******************************************************************** */

void
fname_replace (FILEINF *fdat, char *newname)
{
    if (newname != NULL)
    {
        if ((fdat->fname == NULL) || (strcmp (fdat->fname, newname) != 0))
        {
            free_reference (&fdat->fname);
            fdat->fname = newname;
        }
        else
        {   // New name is the same as the old name.. we don't need new name
            free (newname);
        }
    }
}

/* ************************************************************** *
 * do_lblfileload() - load the label file into the GtkTreeView    *
 * ************************************************************** */

void
do_lblfileload (FILEINF *fdat, char *newfile)
{
    FILE *infile;
    char buffer[500];
    char *bufend;
    int itemcount = 0;
    LV_ITEM lv;

    bufend = &buffer[sizeof(buffer)];
    ZeroMemory (&lv, sizeof (lv));
    lv.mask = LVIF_TEXT;

    /* Now open the file and read it */

    if ( ! (infile = fopen (newfile, "rb")))
    {
        char *emsg;
        const char *fmt = "Cannot open file %s for read!";

        if ( !(emsg = od_memset (fdat->l_store,
                        strlen (newfile) + strlen (fmt) + 10)))
        {
            return;
        }

        sprintf (emsg, fmt, newfile);
        MessageBox (NULL, emsg, "Error!", MB_ICONERROR | MB_OK);
        free (emsg);
        return;
    }

    list_store_empty (fdat);
    free_reference (&(fdat->fname));
    fdat->fname = strstrip (newfile);

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

                        if (tmppt == NULL)      // Error!  Not enough fields
                        {                       // This is probably not a
                            fclose (infile);    // Label file !!!
                            MessageBox (fdat->l_store,
                                    "End of line encountered before 4th field\nThis is probably not a Label File",
                                    "Abort !!!",
                                    MB_ICONERROR | MB_OK);
                            return;
                        }
                        
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
                    amode_add_from_string (&buffer[sspos]);
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
                char mesg[50];

#ifdef MINGW32
                sprintf (mesg, "Failed to insert item - %d,%d",
                              itemcount, fldnum);
#else
                sprintf_s (mesg, sizeof (mesg), "Failed to insert item - %d,%d",
                              itemcount, fldnum);
#endif
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

/* **************************************** *
 * sysfailed() - report failure of system() *
 *     call                                 *
 * **************************************** */

void
sysfailed (HWND hWnd, char *msg)
{
    MessageBox (hWnd, msg, "Error!", MB_ICONERROR | MB_OK);
}


/* **************************************************** *
 * run_disassembler  - a callback from the menu does a  *
 *                     disassembly pass                 *
 * Passed: action, the global variables                 *
 * **************************************************** */

void
run_disassembler (HWND hWnd, glbls *hbuf)
{
    char *cmdline = NULL;
    char tmpstr[MAX_PATH + 50];
    BOOL PipeIt = FALSE;
    int memneed = MAX_PATH;

    // Calculate approximate memory needed

    /* We MUST have a binary file to disassemble */
    
    if ( ! O9Dis.binfile.fname )
    {

        MessageBox (hWnd, "WARNING!\r\nNo program file specified\r\nPlease configure in menu item \"Options\"", "Error - No Program",
                MB_ICONERROR | MB_OK);
        free (cmdline);
        return;
    }

    if (strlen (O9Dis.binfile.fname))
    {
        memneed += strlen (O9Dis.binfile.fname);
    }

    if ( (O9Dis.cmdfile.fname) && (strlen (O9Dis.cmdfile.fname)))
    {
        memneed += strlen (O9Dis.cmdfile.fname);
    }

    if ( (O9Dis.defsfile.fname) && (strlen (O9Dis.defsfile.fname)))
    {
        memneed += strlen (O9Dis.defsfile.fname);
    }

    if ( (O9Dis.asmout.fname) && (strlen (O9Dis.asmout.fname)))
    {
        memneed += strlen (O9Dis.asmout.fname);
    }

    if ( (O9Dis.list_file.fname) && (strlen (O9Dis.list_file.fname)))
    {
        memneed += strlen (O9Dis.list_file.fname);
    }

    if ( !(cmdline = od_memset (hWnd, memneed)))
    {
        return;
    }

    {
        strcpy (cmdline, "os9disasm ");
        strcat (cmdline, O9Dis.binfile.fname);
    }

    if (O9Dis.cmdfile.fname)
    {
#ifdef MINGW32
        sprintf (tmpstr, " -c=%s", O9Dis.cmdfile.fname);
#else
        sprintf_s (tmpstr, sizeof (tmpstr), " -c=%s", O9Dis.cmdfile.fname);
#endif
        strcat (cmdline, tmpstr);
    }

    if ((O9Dis.defsfile.fname) && (strlen(O9Dis.defsfile.fname)))
    {
#ifdef MINGW32
        sprintf (tmpstr, " -d=%s", O9Dis.defsfile.fname);
#else
        sprintf_s (tmpstr, sizeof (tmpstr), " -d=%s", O9Dis.defsfile.fname);
#endif
        strcat (cmdline, tmpstr);
    }
   
    if (RsDos.set)
    {
        strcat (cmdline, " -x=c");
    }

//    if (write_obj)
//    {
        if ((O9Dis.asmout.fname) && (strlen (O9Dis.asmout.fname)))
        {
            if (strlen(O9Dis.asmout.fname))
            {
#ifdef MINGW32
                sprintf (tmpstr, " -o=%s", O9Dis.asmout.fname);
#else
                sprintf_s (tmpstr, sizeof (tmpstr),
                                " -o=%s", O9Dis.asmout.fname);
#endif
                strcat (cmdline, tmpstr);
            }
        }
//    }
    
    if (CpuType.set)
    {
        strcat (cmdline, " -3");
    }

    if (UpCase.set)
    {
        strcat (cmdline, " -u");
    }

    if (ShowZeros.set)
    {
        strcat (cmdline, " -z");
    }

    if (PgWdth.set)
    {
#ifdef MINGW32
        sprintf (tmpstr, " -pw=%d", PgWdth.set);
#else
        sprintf_s (tmpstr, sizeof (tmpstr), " -pw=%d", PgWdth.set);
#endif
        strcat (cmdline, tmpstr);
    }

    if (PgDpth.set)
    {
#ifdef MINGW32
        sprintf (tmpstr, " -pd=%d", PgDpth.set);
#else
        sprintf_s (tmpstr, sizeof (tmpstr), " -pd=%d", PgDpth.set);
#endif
        strcat (cmdline, tmpstr);
    }


    switch (write_list) {
        case LIST_FILE:
            if (O9Dis.list_file.fname)
            {
                if (strlen (O9Dis.list_file.fname))
                {
#ifdef MINGW32
                    sprintf (tmpstr, " > %s", O9Dis.list_file.fname);
#else
                    sprintf_s (tmpstr, sizeof (tmpstr), " > %s",
                                O9Dis.list_file.fname);
#endif
                    strcat (cmdline, tmpstr);
                }
            }
            break;
        case LIST_NONE:
            /* handle bit-bucket output */
            /* g_string_append (CmdL, "-q");*/
            break;
        case LIST_GTK:
            strcat (cmdline, " -g");
            PipeIt = TRUE;
            break;
    }

    if ( ! (PipeIt))   /* Output is to stdout or a file */
    {
        int retval;
        
        switch (retval = system (cmdline))
        {
            case 0: /* sucess */
                break;
            case -1:  /* failed fork */
                MessageBox (hWnd, "Could not fork \"os9disasm\"\r\nIs \"os9disasm\" in your path?", "Fork Error!", MB_ICONERROR | MB_OK);;
                //sysfailed(msg);
                break;
            default:
                MessageBox (hWnd, "Error in executing \"os9disasm\"\r\nPlease check console for error messages", "Error!", MB_ICONERROR | MB_OK);
                //sysfailed (msg);
        }
    }
    else        /* else we're piping to the GUI */
    {
//        FILE *infile;
        HANDLE pRead,
               pWrite;
        PROCESS_INFORMATION pinf;
        SECURITY_ATTRIBUTES sa;
        
        sa.nLength = sizeof (sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        
        /* Now open the file and read it */
        if ( ! CreatePipe (&pRead, &pWrite, &sa, 1024))
        {
            MessageBox (hWnd, cmdline, "Read Error on Command...",
                              MB_ICONERROR | MB_OK);
            return;
        }
        else
        {
            STARTUPINFO odis_si;
            ZeroMemory (&odis_si, sizeof (odis_si));
            odis_si.cb = sizeof (odis_si);
            odis_si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            odis_si.hStdOutput = pWrite;
            odis_si.wShowWindow = SW_HIDE;
            //Need hstdInput & hStdError??

            ZeroMemory (&pinf, sizeof (pinf));
            
            if ( ! CreateProcess (NULL, // lpApplicationName - let cmdline
                        cmdline,  // lpCommandLine
                        NULL,     // lpProcessAttributes - Security
                        NULL,     // lpThreadAttributes
                        TRUE,     // Inherit all inheritable HANDLEs
                        0,        // dwCreationFlags
                        NULL,     // lpEnvironment - Use "my" environment
                        NULL,     // lpDirectory - use this one
                        &odis_si,    // lpStartupInfo - appearance
                        &pinf))
            {
                MessageBox (hWnd, "Could not create 'os9disasm' process",
                        "Error!", MB_ICONERROR | MB_OK);
                return;
            }

//            hTmpFile = CreateFile ("hout.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

//            if (hTmpFile == INVALID_HANDLE_VALUE)
//            {
//                MessageBox (hWnd, "Cannot Create temp file", "Error",
//                        MB_ICONERROR | MB_OK);
//                // Any cleanup???

//                return;
//            }
            
            CloseHandle (pWrite);
            
            list_store_empty (&(hbuf->list_file));
            load_list_tree (&hbuf->list_file, pRead);
//            pclose (infile);
            CloseHandle (pinf.hProcess);
            CloseHandle (pinf.hThread);
//            CloseHandle (hTmpFile);
            CloseHandle (pRead);
        }
    }

    free (cmdline);
}

/* Disassemble listing to file */

void
dasm_list_to_file_cb (HWND hWnd, glbls *hbuf)
{
    int old_write = write_list;     // Temp save write_list

    selectfile_save (hWnd, &(O9Dis.list_file.fname), FFT_MISC);
    //set_fname (hbuf, &O9Dis.list_file.fname);
    write_list = LIST_FILE;     // Substitute outpath to file
    run_disassembler (hWnd, hbuf);
    write_list = old_write;     // Restore original outpath
}

/* ******************************************** *
 * Callbacks for file selection                 *
 * Passed:  (1) GtkAction *action,              *
 *          (2) address of global data pointer  *
 * ******************************************** */

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

/* ******************************************************** *
 * load_listing() - A callback from the menu(s).            *
 *          This is the "top-level" function for loading    *
 *          a listing                                       *
 * ******************************************************** */

void
load_listing (FILEINF *fdat)
{
    char *tmpname;
    HANDLE hTmpFile;

    tmpname = selectfile_open (fdat->l_store, "Prog Listing", FFT_MISC);

    if (tmpname == NULL)
    {
        return;    // We had either "Cancel" or failure
    }

    /* Call list_store_empty - if it's a listing, fdat->altered shouldn't
     * be set to TRUE (hopefully)
     */

    list_store_empty (fdat);

    /* Now open the file and read it */
    hTmpFile = CreateFile (tmpname,
                           GENERIC_READ, FILE_SHARE_READ,
                           0, OPEN_EXISTING,
                           FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, 
                           NULL);

    if (hTmpFile == INVALID_HANDLE_VALUE)
    {
        char ttl[50];
        PVOID errmsg;
        DWORD ernum;

        ernum = GetLastError();
#ifdef MINGW32
        sprintf (ttl, "Cannot Open Listing File - Error #%ld", ernum);
#else
        sprintf_s (ttl, sizeof (ttl), "Cannot Open Listing File - Error #%ld",
                        ernum);
#endif
        FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                ernum,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPSTR) &errmsg,
                0, NULL);
        MessageBox (fdat->l_store, errmsg, ttl, MB_ICONERROR | MB_OK);
        LocalFree (errmsg);

        return;
    }
    
    fname_replace (fdat, tmpname);

    load_list_tree (fdat, hTmpFile);
    CloseHandle (hTmpFile);
    //fclose (infile);
}

/* **************************************************************** *
 * load_cmdfile() - Callback for Button click to load command file  *
 * **************************************************************** */

void
load_cmdfile (FILEINF *fdat)
{
    char *tmpname;

    tmpname = selectfile_open (fdat->l_store, "Command file", FFT_CMD);

    if (tmpname != NULL)
    {
        //free_reference (&(fdat->fname));
        //fdat->fname = tmpname;
        load_text (fdat, tmpname);
    }
}

void
load_lblfile (FILEINF *fdat)
{
    char *filetmp;
    
    filetmp = selectfile_open (fdat->l_store, "Label File", FFT_LBL);

    if (filetmp != NULL)
    {
        do_lblfileload (fdat, filetmp);
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
    char *tmpname;
    
    if (GetWindowTextLength (fdat->l_store) > 2) // Only an MS-Dos EOL set
    {
        tmpname = selectfile_save (fdat->l_store, &(fdat->fname), FFT_CMD);

        if (tmpname != NULL)
        {
            save_text (fdat->l_store, tmpname);
            SendMessage (fdat->l_store, EM_SETMODIFY, FALSE, 0);
        }

        // handle fname

    // Don't think we need this -- selectfile_save did it, I think
//    fname_replace (fdat, tmpname);
    }
    else
    {
        MessageBox (fdat->l_store, "Insufficient data to save...",
                "No Data", MB_ICONEXCLAMATION | MB_OK);
    }
}

void
cmd_save (FILEINF *fdat)
{

    if (GetWindowTextLength (fdat->l_store) > 2) // Only an MS-Dos EOL set
    {
        // If we don't yet have a filename, we must use Save As...
        if (fdat->fname == NULL)
        {
            cmd_save_as (fdat);
        }
        else {
            save_text (fdat->l_store, fdat->fname);
            SendMessage (fdat->l_store, EM_SETMODIFY, FALSE, 0);
        }
    }
    else
    {
        MessageBox (fdat->l_store, "Insufficient data to save...",
                "No Data", MB_ICONEXCLAMATION | MB_OK);
    }
}

void
lbl_save_as (FILEINF *fdat)
{
    char *tmpname;
    
    tmpname = selectfile_save (fdat->l_store, &(fdat->fname), FFT_LBL);
    
    if (tmpname != NULL)       /* Leak? */
    {
        wrt_lbl_file (fdat, &tmpname);
        fdat->altered = FALSE;
    }
}

void
lbl_save (FILEINF *fdat)
{
    // If we have not yet established a filename, go to Save As..

    if( fdat->fname == NULL)
    {
        lbl_save_as (fdat);
    }
    else {
        wrt_lbl_file (fdat, &fdat->fname);
        fdat->altered = FALSE;
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
opt_put (FILEINF *fi, char *hdr, FILE *fp)
{
    if (fi->fname && strlen(fi->fname))
    {
        fprintf (fp, "%s%s\n", hdr, fi->fname);
    }
}

/* ******************************************************************** *
 * fix_opt_path () Massages the input options line, bumping pointer to  *
 *   file path past whitespaces, removing newline characters, and       *
 *   stores value into designated place (freeing string previously      *
 *   referred to if applicable.                                         *
 * ******************************************************************** */

static void
fix_opt_path (char **pthptr,    // something->fname..
               char *pthnm,     // the pathname string
               BOOL *flag,      // An option flag (sometimes present)
               char *hdr)       // the begin of the string - to be bypassed
{
    char *start = pthnm;

    start += strlen (hdr);
    
    if (flag)
    {
        *flag = TRUE;
    }

    strstrip (start);

    if (pthptr)
    {
        free_reference (pthptr);
        *pthptr = _strdup (start);
    }
}

/* **************************************************************** *
 * opt_get_val() - check for a match in the HDR and store the value *
 *          of matched                                              *
 * PASSED: hdr string to check against buffer holding the loaded    *
 *          line address to store value if matched                  *
 * Returns: value if matched, NULL if no match                      *
 * **************************************************************** */

static int
opt_get_val ( char *ctrl, char *buf, int *addr)
{
    char *start = buf;

    if (!strncmp (ctrl, buf, strlen (ctrl)))
    {
        start += strlen (ctrl);
        strstrip (start);
        sscanf (start, "%d", addr);
        return *addr;
    }

    return 0;
}

void
opts_save (HWND hWnd, glbls *hbuf)
{
    FILE *optfile;
    char *fname;
    char msg[100];

    selectfile_save (hWnd, &fname, FFT_MISC);
#ifdef MINGW32
    sprintf (msg, "Could not open options file '%s'", fname);
#else
    sprintf_s (msg, sizeof (msg), "Could not open options file '%s'", fname);
#endif
    
    if (!(optfile = fopen (fname, "w")))
    {
        char *msg;
        int errnum = GetLastError();

        FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, errnum,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&msg, 0,
                NULL);
        MessageBox (hWnd, msg, "File Open Failed!", MB_ICONERROR | MB_OK);
        LocalFree (msg);
        free (fname);
        return;
    }

    free (fname);

    opt_put (&O9Dis.binfile, OPT_BIN, optfile);
    opt_put (&O9Dis.cmdfile, OPT_CMD, optfile);
    opt_put (&O9Dis.lblfile, OPT_LBL, optfile);
    opt_put (&O9Dis.defsfile, OPT_DEF, optfile);
    opt_put (&O9Dis.asmout, OPT_OBJ, optfile);
    fprintf (optfile, "%s%d\n", OPT_RS, RsDos.set);
    fprintf (optfile, "%s%d\n", OPT_CPU, CpuType.set);
    fprintf (optfile, "%s%d\n", OPT_UPC, UpCase.set);
    fprintf (optfile, "%s%d\n", OPT_PGW, PgWdth.set);
    fprintf (optfile, "%s%d\n", OPT_PGD, PgDpth.set);
    fprintf (optfile, "%s%d ", OPT_LST, write_list);
    
    switch (write_list)
    {
        case LIST_FILE:
            if (O9Dis.list_file.fname)
            {
                if (strlen (O9Dis.list_file.fname))
                {
                    fprintf (optfile, "%s\n", O9Dis.list_file.fname);
                }
            }
            break;
        default:
            fputc('\n', optfile);
    }

    fclose (optfile);
}

void
opts_load (HWND hwnd, glbls *hbuf)
{
    FILE *optfile;
    char buf[MAX_PATH];
    char *tmpfile;
    
    if ( !(tmpfile = selectfile_open (hwnd, "n Options File", FFT_MISC)))
    {
        return;
    }
    
#ifdef MINGW32
    optfile = fopen (tmpfile, "rb");
    sprintf (buf, "Could not open options file %s", buf);
#else
    fopen_s ( &optfile, tmpfile, "rb");
    sprintf_s (buf, sizeof (buf), "Could not open options file %s", buf);
#endif
    
    if ( ! optfile)
    {
        MessageBox (hwnd, buf, "File Open Failed!", MB_ICONERROR | MB_OK);
        return;
    }

    while (fgets (buf, sizeof (buf), optfile))
    {
        
        if ( ! strncmp (OPT_BIN, buf, strlen(OPT_BIN)))
        {
            fix_opt_path (&(O9Dis.binfile.fname), buf, NULL, OPT_BIN);
            menu_do_dis_sensitize (hwnd);
            continue;
        }

        if ( ! strncmp (OPT_CMD, buf, strlen (OPT_CMD)))
        {
            fix_opt_path (NULL, buf, NULL, OPT_CMD);
//            hbuf->filename_to_return = strdup (O9Dis.cmdfile.fname);
            load_text (&(hbuf->cmdfile), _strdup (&buf[strlen (OPT_CMD)]));
            continue;
        }

        if ( ! strncmp (OPT_LBL, buf, strlen (OPT_LBL)))
        {
            fix_opt_path (NULL, buf, NULL, OPT_LBL);
//            hbuf->filename_to_return = strdup (hbuf->lblfile.fname);
            do_lblfileload (&(hbuf->lblfile),
                            _strdup (&buf[strlen (OPT_LBL)]));
            continue;
        }

        if ( ! strncmp (OPT_DEF, buf, strlen(OPT_DEF)))
        {
            fix_opt_path (&(O9Dis.defsfile.fname), buf, NULL, OPT_DEF);
            continue;
        }

        if ( ! strncmp (OPT_OBJ, buf, strlen (OPT_OBJ)))
        {
            fix_opt_path (&(O9Dis.asmout.fname), buf, NULL, OPT_OBJ);
            continue;
        }

        if ( ! strncmp (OPT_LST, buf, strlen (OPT_LST)))
        {
            char *splits[2];
            BOOL failed = FALSE;

            splits[0] = buf;
            
            splits[0] += strlen (OPT_LST);
            splits[0] = strstrip (splits[0]);
            splits[1] = splits[0];

            while (*splits[1] != ' ')
            {
                ++(splits[1]);

                if (*splits[1] == '\0')
                {
                    failed = TRUE;
                    break;
                }
            }

            if (failed)
            {
                continue;
            }

            *splits[1] = '\0';

            if (splits[0])
            {
#ifdef MINGW32
                sscanf (splits[0], "%d", &write_list);
#else
                sscanf_s (splits[0], "%d", &write_list);
#endif
            }

            if (write_list == LIST_FILE)
            {
               
                if (splits[1])
                {
                    char *nambegin;
                    free_reference (&O9Dis.list_file.fname);
                    nambegin = strstrip (splits[1]);

                    if (strlen (nambegin))
                    {
                        O9Dis.list_file.fname = _strdup (nambegin);
                    }
                }
            }

            continue;
        }

        if (!opt_get_val (OPT_RS, buf, &RsDos.set))
        {
            if (!opt_get_val (OPT_CPU, buf, &CpuType.set))
            {
                if (!opt_get_val (OPT_UPC, buf, &UpCase.set))
                {
                    if (!opt_get_val (OPT_PGW, buf, &PgWdth.set))
                    {
                        /* Here, quietly test the last one and just
                         * ignore the line if this does not match
                         */
                        opt_get_val (OPT_PGD, buf, &PgDpth.set);
                    }
                }
            }
        }
    }
}

