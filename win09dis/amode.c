/* **************************************************************** *
 * amode.c - handles editing functions for editing addressing modes $
 *      a breakout  from dasmedit.c                                 $
 * $Id::                                                            $
 * **************************************************************** */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "win09dis.h"
#include <commctrl.h>
/*#define GOT_DTBL    */            /*  We loaded the data in dasmedit.c */
#include "../os9disasm/dtble.h"

// Temporary fix
#ifndef HAVE_STRCASECMP
#  define strcasecmp strcmp
#endif

GList *
g_list_find (GList *gl, void *str, int len)
{
    do
    {
        if (len)
        {
            if ( !strncmp (gl->data, str, len))
            {
                return gl;
            }
        }
        else
        {
            if ( !strcmp (gl->data, str))
            {
                return gl;
            }
        }
    } while ((gl = gl->next));

    return NULL;
}

GList *
g_list_alloc (GList *prev, void *data)
{
    GList *pos;
    
    if ((pos = calloc (sizeof (GList), 1)))
    {
        pos->data = data;

        if (prev)
        {
            prev->next = pos;
            pos->prev = prev;
        }

        return pos;
    }

    return NULL;
}

GList *
g_list_append (GList *lbegin, void *data)
{
    GList *pos = lbegin;

    if (lbegin)
    {
        while (pos->next)
        {
            pos = pos->next;
        }

        g_list_alloc (pos, data);
        return lbegin;
    }
    else
    {
        return g_list_alloc (NULL, data);
    }
}

/* **************************************************************** *
 * g_list_del() - Delete a GList element.                           *
 * Passed : (1) - Ptr to first element of list                      *
 *          (2) - Ptr to element to delete.                         *
 * Returns: original begin element if deleted element is not the    *
 *          first element, else the next element after the deleted  *
 *          ( which is the new begin)                               *
 * **************************************************************** */

GList *
g_list_del (GList *begin, GList *deleteme)
{
    if (deleteme->prev)
    {
        deleteme->prev->next = deleteme->next;
    }
    else
    {
        begin = deleteme->next;
    }

    if (deleteme->next)
    {
        deleteme->next->prev = deleteme->prev;
    }

    free (deleteme);
    return begin;
}
//static GList *offset_list;  /* Convenience list of offsets used */

extern GList *amode_list;

/* ******************************************************** *
 * amode_init() - populate the addressing modes list with   *
 *          the standard label classes                      *
 * NOTE: a g_strdup() of the constant string is used so it  *
 *       can be deleted if desired                          *
 * ******************************************************** */

GList *
amode_init()
{
    char *amode_dflt[] = {"$ - Hexadecimal constant",
                          "& - Decimal constant",
                          "@ - Decimal<10 or hex>=10",
                          "^ - ASCII constant",
                          "% - Binary bit mode",
                          "L - Program code",
                          "D - Data variable",
                          "X - External reference",
                          "E - Error code definition",
                          "S - Set/GetStat call",
                          "P - Process descriptor (P$XX)",
                          "! - System function",
                          "A - File access codes",
                          "C - Condition Code register mask",
                          "I - I/O channel (stdin, etc)",
                          "T - Module type/language mask",
                          NULL};
    char **cur =amode_dflt;

    while (*cur) {
        amode_list = g_list_append (amode_list, strdup (*cur));
        ++cur;
    }

    return amode_list;
}

/* ************************************************************ *
 * callback for click on "add" button to add an addressing mode *
 * ************************************************************ */

static void
add_amode_man (HWND hDlg, HWND editwin)
{
    int txtlen = GetWindowTextLength (editwin);
    char *newmode;
//    GString *mstr;
//    mstr = g_string_new (g_strdup(gtk_entry_get_text (GTK_ENTRY(entry))));
//    gchar *message = NULL; 

    if ( txtlen == 0)
    {
        MessageBox (hDlg, "No text in text entry box", "Error!",
                        MB_ICONERROR|MB_OK);
        return;
    }
    
    if ( ! (newmode = od_memset (hDlg, txtlen + 1)))
    {
        return;
    }
    
    GetWindowText (editwin, newmode, txtlen);
    
    if ( ! isalpha (newmode[0]))
    {
        MessageBox (hDlg, "The first character MUST be an ALPHA", "Error!",
                MB_ICONERROR|MB_OK);
        free (newmode);     // We don't need it in this case, we're aborting
        return;
    }
    
    if ( ! amode_list)    /* never been initialized? */
    {
        amode_init();
    }

    amode_list = g_list_append (amode_list, newmode);
//    amode_list = g_list_append (amode_list, mstr->str);
}

/* ******************************************************** *
 * amode_add_from_string () -  Add an amode to amode_list   *
 *     provided by a string  (usually loaded from a file    *
 * ******************************************************** */
void
amode_add_from_string (char *ptr)
{
    char *str;
    char c;
    GList *glptr;

    /* Work with a copy since ptr is temporary, plus we're going to modify
     * it */
    
    str = strdup (ptr);
    
    /* Get rid of any leading and trailing whitespaces */
    
    strstrip (str);
    
    if (strlen (str) == 0)
    {
        free (str);
        return;
    }
    
    /* In case amode_list has never been initialized */
    
    if (!amode_list)
    {
        amode_init();
    }

    /* Try to see if this entry has been already been inserted *
     * We use our own version in windows which allows one to   *
     * limit the string length                                 */
    
    if ( (glptr = g_list_find (amode_list, str, 1)))
    {       // Replace if the description is different
        if (strcmp (glptr->data, str))
        {
            free (glptr->data);
            glptr->data = str;
        }
    }
    else
    {
        amode_list = g_list_append (amode_list, str);
    }
}

/* **************************************************** *
 * amodes_from_file() - get list of amodes from a file  *
 * **************************************************** */

static void
amodes_from_file (HWND parent)
{
    char *filename;
    
    filename = selectfile_open (parent, "Label Descriptor", FFT_MISC);

    if (filename)
    {
        FILE *infile;

        if ((infile = fopen (filename, "rb")))
        {
            char buf[500];

            while ( fgets (buf, sizeof(buf), infile))
            {
                char *pt = buf;
                
                strstrip (buf);

                if ( ! strncmp (pt, "*\\", 2))
                {
                    pt = &(pt[2]);
                    amode_add_from_string (pt);
                }
            }
            
            fclose (infile);
        }
        
        /* last thing... */
        free (filename);
    }
}

static void
delete_combo_changed (HWND dlg, HWND combo)
{
    char *selectedstr;
    int index,
        txtLen;

    index = SendMessage (combo, CB_GETCURSEL, 0, 0);

    if (index != CB_ERR)
    {
        char msg[200];

        txtLen = SendMessage (combo, CB_GETLBTEXTLEN, index, 0);

        if ( ! (selectedstr = od_memset (dlg, txtLen + 1)))
        {
            return;
        }

        SendMessage (combo, CB_GETLBTEXT, index, (LPARAM)selectedstr);

        sprintf (msg, "Delete:\r\n'%s'\r\nOK?", selectedstr);

        if ((MessageBox (dlg, msg, "Confirm",
                            MB_ICONQUESTION|MB_YESNO|MB_APPLMODAL) == IDYES))
        {
            GList *delEnt = g_list_find (amode_list, selectedstr,
                                strlen (selectedstr));
            amode_list = g_list_del (amode_list, delEnt);
        }

        free (selectedstr);
    }
}

/* ************************************************ *
 * delete_amode_cb () - dialog to choose an amode   *
 *      entry to delete                             *
 * ************************************************ */

//static void
//delete_amode_cb (HWND topwin)
//{
//    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_AM_DEL), topwin,
//                                                            AmodeEditDlgProc); 
//    combo = build_label_selector(NULL, FALSE);
//    g_signal_connect (combo, "changed", G_CALLBACK(delete_combo_changed),
//                      dialog);
//    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), combo,
//                        FALSE, FALSE, 5);
//
//    gtk_widget_show_all (dialog);
//
//    /* Nothing returned, all processing done within dialog */
//    
//    gtk_dialog_run (GTK_DIALOG(dialog));
//    gtk_widget_destroy(dialog);
//}

static BOOL CALLBACK
DeleteAmodeDlgProc (HWND hDelDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            build_label_selector ( GetDlgItem (hDelDlg, ID_AM_DEL_COMBO),
                                       NULL);
            return TRUE;
        case WM_COMMAND:
            switch LOWORD(wParam)
            {
                case ID_AM_DEL_COMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        delete_combo_changed (hDelDlg, (HWND)lParam);
                        EndDialog (hDelDlg, 0);
                    }

                    return FALSE;
                case IDCANCEL:
                    EndDialog (hDelDlg, 0);
                    break;
            }
            break;
    }

    return FALSE;
}

/* ************************************************************ *
 * AmodeEditDlgProc() - Window procedure for Amode Edit dialog  *
 * ************************************************************ */

static BOOL CALLBACK
AmodeEditDlgProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch LOWORD(wParam)
            {
                case ID_AM_EDCLEAR:
                    SendMessage (GetDlgItem (hDlg, ID_AM_ADD),
                                 EM_SETSEL, 0, -1);
                    SendMessage (GetDlgItem (hDlg, ID_AM_ADD), WM_CUT, 0, 0);
                    return FALSE;
                case ID_AM_EDAPPLY:
                    add_amode_man (hDlg, GetDlgItem (hDlg, ID_AM_ADD));
                    return FALSE;
                case ID_AM_EDOPEN:
                    amodes_from_file (hDlg);
                    return FALSE;
                case ID_AM_EDDEL:
                    DialogBox (GetModuleHandle (NULL),
                               MAKEINTRESOURCE(IDD_AM_DEL), hDlg,
                               DeleteAmodeDlgProc); 
                    return FALSE;
                case ID_AM_EDCLOSE:
                case IDCANCEL:  //Dialog box converts WM_CLOSE to IDCANCEL
                    EndDialog (hDlg, 0);
                    return FALSE;
                default:
                    return FALSE;
            }
            break;
    }

    return FALSE;
}

/* ************************************************************ *
 * amode_list_edit_cb() - add/delete or otherwise modify the    *
 *          addressing mode list                                *
 * ************************************************************ */

void
amode_list_edit_cb (HWND topwin)
{
    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_AMODEEDIT), topwin,
                                                AmodeEditDlgProc);
}

char *
get_addressing_mode (HWND hWnd, char *opcode, char *mnemonic, char *oprand)
{
    struct lkuptbl *T;
    int opcode_val,
        mymode,
        sz;
    char *pseudcodes[] = {"rmb", "fcb", "fcc", "fcs", "fdb", NULL};
    char **pt;
    BOOL is_pseud = FALSE;

    if ( ! strncmp(opcode,"10",2) )
    {
        T = Pre10;
        sscanf (&opcode[2], "%02x", &opcode_val);
        sz = G_N_ELEMENTS(Pre10);
    }
    else {
        if (! strncmp(opcode,"11",2) )
        {
            T = Pre11;
            sscanf (&opcode[2], "%02x", &opcode_val);
            sz = G_N_ELEMENTS(Pre11);
        }
        else {
            T = Byte1;
            sscanf (opcode, "%02x", &opcode_val);
            sz = G_N_ELEMENTS(Byte1);
        }
    }

    pt = pseudcodes;

    while (*pt)
    {
        if ( ! strcmp (*(pt++), mnemonic))
        {
            is_pseud = TRUE;
            break;
        }
    }

    while (((T->cod & 0xff) != opcode_val) ) {
        if (--sz == 0 )
        {
            return NULL;
        }

        ++T;
    }

    mymode = T->amode;

    if ( ! is_pseud &&  strncmp (mnemonic, T->mnem, strlen(mnemonic) ) )
    {
        char msg[200];

        sprintf (msg,
              "The standard mnemonic %s $(%x)\ndoes not match %s in the file",
              T->mnem, T->cod, mnemonic);
        MessageBox (hWnd, msg, "Syntax Error!", MB_ICONERROR | MB_OK);
    }

      /* ********************************************************** *
       * All indexed modes are chunked together as XIDX (x indexed) *
       * the actual register needs to be identified                 *
       * ********************************************************** */

      if (mymode == AM_XIDX )
      {
          char *reg;

          if ( !(reg = strchr (oprand, ',')))
          {
              MessageBox (hWnd, "No comma found in operand !!!",
                      "Syntax Error!", MB_ICONERROR | MB_OK);
              /* if no comma, we have a PROBLEM! */
              return NULL;
          }
          
          ++reg;

          /* take care of ",-R" and ",--R" */
          
          while ( *reg == '-' ) {
              ++reg;
          }

          switch (toupper(*reg) ) {
              case 'Y':
                  return "Y";
              case 'S':
                  return "S";
              case 'U':
                  return "U";
              case 'X':
                  return "X";
              case 'P':
                  return "R";
              default:
                  MessageBox (hWnd, "Lookup for Index Register FAILED !",
                          "Error!", MB_ICONERROR | MB_OK);
                  return NULL;  /* PROBLEM!!! */
          }
      }
      
      
      switch (mymode) {
          case AM_DRCT:
              return "D";
          case AM_REL:
              return "R";
          case AM_BYTI:
              return "#1";
          case AM_DIMM:
              return "#D";
          case AM_YIMM:
              return "#Y";
          case AM_XIMM:
              return "#X";
          case AM_SIMM:
              return "#S";
          case AM_UIMM:
              return "#U";
          case AM_EXT:
              return "E";
          default:
              return NULL;
      }
      
}

/* ******************************************************************** *
 * on_adr_mode_response () - Callback for when "OK" button is clicked   *
 *      in addressing mode setup                                        *
 * Passed:  (1) dialog box containing widgets                           *
 *          (2) the response button clicked                             *
 *          the structure containing ptrs to the entry widgets          *
 * ******************************************************************** */

static void
on_adr_mode_response (HWND amDlg)
{
    char line[100],
         tmpstr[50];

    strcpy (line, "> ");

    if ( ! GetWindowText (GetDlgItem (amDlg, IDSAM_AM), tmpstr,
                                        sizeof (line) - strlen (line) - 1))
    {
        return; // No valid addressing mode, abort
    }

    tmpstr[1] = '\0';
    strcat (line, tmpstr);

    // Offset (if any)

    GetWindowText (GetDlgItem (amDlg, IDSAM_OFST), tmpstr,
                                      sizeof (tmpstr)- 1);
    if (strlen (tmpstr))
    {
        if (strlen (tmpstr) < (sizeof (line) - strlen (line) - 1))
        {
            sprintf (&(line[strlen (line)]), " (%s)", tmpstr);
        }
        else
        {
            return;
        }
    }

    /* Address */
    if ( ! GetWindowText (GetDlgItem (amDlg, IDSAM_RNGE), tmpstr,
                                      sizeof (tmpstr)- 1))
    {
        return; // No valid addressing mode, abort
    }

    if (strlen (tmpstr) > (sizeof (line) - strlen (line) - 4))
    {
        return;     // Not enough space 
    }

    sprintf (&(line[strlen (line)]), " %s\r\n", tmpstr);
    txtbuf_insert_line (O9Dis.cmdfile.l_store, line);
    /*doc_set_modified( &O9Dis.cmdfile, TRUE);*/
}

/* Try to guess a usable Addressing Mode from the mnemonic */

char *
guess_addr_mode (char *mnem)
{
    char *guess;
    
    if ( ! strcasecmp (mnem, "equ") )
    {
        guess = "X";
    }
    else {
        if ( ! strcasecmp (mnem, "rmb"))
        {
            guess = "D";
        }
        else {
            guess = "L";
        }
    }
    return guess;
}

static BOOL CALLBACK
SetAmodeProc (HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            {
                HWND listview = O9Dis.list_file.l_store;
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

                 // Get addressing mode for current command

                if ( ! ( cmd_mode =
                            get_addressing_mode (listview,
                                            row_data[LST_OPCO],
                                            row_data[LST_MNEM],
                                            row_data[LST_OPER])) )
                {
                    cmd_mode = "L";
                }

                SetWindowText (GetDlgItem (hWndDlg, IDSAM_CLASS), cmd_mode);

                // Addressing Mode dropdown list

                build_label_selector ( GetDlgItem (hWndDlg, IDSAM_AM),
                                       NULL);

                SetWindowText ( GetDlgItem (hWndDlg, IDSAM_RNGE),
                                row_data[LST_ADR]);
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    return EndDialog (hWndDlg, 0);
                case IDOK:
                    on_adr_mode_response (hWndDlg);
                    return EndDialog (hWndDlg, 0);
            }
    }

    return FALSE;
}

/* ************************************************************ *
 * adr_mode_cb() - callback function to handle addressing mode  *
 * ************************************************************ */

void
adr_mode_cb (HWND dlgWnd, FILEINF *fdat)
{
    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_SETAMODE),
                    fdat->l_store, SetAmodeProc);
}

