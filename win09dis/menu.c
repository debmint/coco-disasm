/* ************************************************************ *
 * menu.c - menu-related items : create, etc                    $
 *                                                              $
 * $Id::                                                        $
 * ************************************************************ */


//#include <stdlib.h>
//#include <string.h>
#include <stdio.h>
#include "win09dis.h"

HINSTANCE hInst;

RECT wnbx;

DLGPROC CALLBACK
AboutDlgProc ( HWND hwndDlg,	    // handle to dialog box
               UINT uMsg,	        // message
               WPARAM wParam,	    // first message parameter
               LPARAM lParam)	    // second message parameter
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            char vrsion[20];
            
            // Insert informational text into text window
            // We do this now in order to avoid updating verion
            // in rc file.

            char *msgary[7] = {"  -- G09Dis V ",
                               vrsion,
                               "  A graphical front-end to \"os9disasm\"\r\n",
                               "  for Windows\r\n",
                               "  This version is strictly BETA!!!",
                               NULL};
            char msg[250],
                **msgptr;

            msg[0] = '\0';
            msgptr = msgary;
            sprintf (vrsion, "%s --\r\n\r\n", PACKAGE_VERSION);

            while (*msgptr)
            {
                strcat (msg, *(msgptr++));
            }

            SetWindowText (GetDlgItem (hwndDlg,IDC_ABTTXT), msg);

        }
        break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog (hwndDlg, IDOK);
                    break;
            }

            break;
        default:
            return (DLGPROC)FALSE;
    }

    return (DLGPROC)TRUE;
}

void
menu_do_dis_sensitize (HWND hWnd)
{
    HMENU menuMain,
          optsMenu;
    UINT enableFlag = MF_GRAYED;

    if ((O9Dis.binfile.fname) && (strlen (O9Dis.binfile.fname) > 0))
    {
        enableFlag = MF_ENABLED;
    }

    if ((menuMain = GetMenu (hWnd)))
    {
        if ((optsMenu = GetSubMenu (menuMain, 2)))
        {
            EnableMenuItem (optsMenu, 0, MF_BYPOSITION | enableFlag);
            EnableMenuItem (optsMenu, 1, MF_BYPOSITION | enableFlag);
        }
        else
        {
            MessageBox (hWnd, "Failed to get Options SubMenu", "Error !!",
                    MB_ICONERROR | MB_OK);
        }
    }
    else
    {
        MessageBox (hWnd, "Could not obtain Main Menu", "Error !!",
                    MB_ICONERROR | MB_OK);
    }
}

//static void
//cmd_new_cb (GtkAction * action, glbls * hbuf)
//{
//    clear_text_buf(&(hbuf->cmdfile));
//}
//
//static void
//lbl_new_cb (GtkAction * action, glbls * hbuf)
//{
//    list_store_empty(&(hbuf->lblfile));
//}
//
//static void
//tip_toggle(GtkToggleAction *ta, glbls *hbuf)
//{
//    if( gtk_toggle_action_get_active(ta) )
//    {
//        gtk_tooltips_enable(ttip);
//        g_print("Tooltips are now ON...\n");
//    }
//    else {
//        gtk_tooltips_disable(ttip);
//        g_print("Tooltips are now OFF...\n");
//    }
//}
//
///* Construct a GtkActionEntry */
//
///* Normal items */
//static GtkActionEntry entries[] = {
//    {"LblInsertLin", NULL, "Insert line after", NULL,
//        "Insert new line after", G_CALLBACK (lbl_insert_line)},
//    {"LblDeleteLin", GTK_STOCK_DELETE, "Delete", NULL,
//        "Delete Line", G_CALLBACK (lbl_delete_line)},
//
//    {"NameLabel", NULL, "Rename Label", NULL,
//        "Define a Label Name", G_CALLBACK (rename_label)},
//    {"BndsDefine", NULL, "Define Bounds", NULL,
//        "Define Data Boundary", G_CALLBACK (bnds_define_cb)},
//    {"AdrMode", NULL, "Set Addressing Mode", NULL,
//        "Change Addressing Mode", G_CALLBACK (adr_mode_cb)},
//    {"LblProps", NULL, "Properties", NULL,
//        "Properties of this label", G_CALLBACK (lbl_properties)},
//    {"QuitProg", GTK_STOCK_QUIT, "_Quit", "<control>Q",
//     "Quit program", G_CALLBACK(window_quit)},
//    {"HlpAbout", NULL, "About", NULL, NULL, G_CALLBACK (hlp_about)}
//};
//
///* Toggle items */
//static GtkToggleActionEntry toggle_entries[] = {
//    {"ShowTips", NULL, "Show Tooltips", NULL, "Show tooltips",
//        G_CALLBACK (tip_toggle), TRUE}
//};
//
//            /* Listing popup */
///*static GtkActionEntry list_group[] = {
//    {"NameLabel", NULL, "Rename Label", NULL,
//        "Define a Label Name", G_CALLBACK (rename_label)},
//    {"BndsDefine", NULL, "Define Bounds", NULL,
//        "Define Data Boundary", G_CALLBACK (bnds_define_cb)},
//    {"AdrMode", NULL, "Set Addressing Mode", NULL,
//        "Change Addressing Mode", G_CALLBACK (adr_mode_cb)}
//};*/
//
///* The XML description of the menubar */
//
//static const char *ui_description =
//    "<ui>"
//    "  <popup name='LabelPopUp'>"
//    "    <menuitem action='LblSearch'/>"
//    "    <separator/>"
//    "    <menuitem action='LblOpen'/>"
//    "    <separator/>"
//    "    <menuitem action='LblSave'/>"
//    "    <menuitem action='LblSaveAs'/>"
//    "    <separator/>"
//    "    <menuitem action='LblInsertLin'/>"
//    "    <menuitem action='LblDeleteLin'/>"
//    "    <separator/>"
//    "    <menuitem action='LblProps'/>"
//    "  </popup>"
//    "</ui>";
//
///* Returns a menubar widget made from the above menu */
//
///*extern GtkTreeSelection *list_selection;*/
//
//GtkWidget *
//get_menubar_menu (GtkWidget * main_window)
//{
//    GtkWidget *mnubar;
//    GtkActionGroup *action_group;
//    /*GtkActionGroup *list_action_grp;*/
//    /*GtkUIManager *ui_manager;*/
//    GtkAccelGroup *accel_group;
//    GError *error;
//
//    action_group = gtk_action_group_new ("MenuActions");
//    gtk_action_group_add_actions (action_group, entries,
//                                  G_N_ELEMENTS (entries), &O9Dis);
//    gtk_action_group_add_toggle_actions (action_group, toggle_entries,
//                                         G_N_ELEMENTS (toggle_entries),
//                                         &O9Dis);
//
///*    list_action_grp = gtk_action_group_new("ListPopActions");
//    gtk_action_group_add_actions (list_action_grp, list_group,
//                                  G_N_ELEMENTS (list_group),
//                                  &list_selection);*/
//    
//    ui_manager = gtk_ui_manager_new ();
//    gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
//    /*gtk_ui_manager_insert_action_group (ui_manager, list_action_grp, 0);*/
//
//    accel_group = gtk_ui_manager_get_accel_group (ui_manager);
//    gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);
//
//    error = NULL;
//    if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description,
//                                            -1, &error))
//    {
//        g_message ("building menus failed %s", error->message);
//        g_error_free (error);
//        exit (EXIT_FAILURE);
//    }
//
//    /* add tooltips */
//    ttip = gtk_tooltips_new();
//    gtk_tooltips_set_tip(ttip, gtk_ui_manager_get_widget( ui_manager,
//                         "/MainMenu/FileMenu/File_New/"),
//                         "Clears buffer for selected file",
//                         NULL);
//    gtk_tooltips_set_tip(ttip, gtk_ui_manager_get_widget(ui_manager,
//                         "/MainMenu/FileMenu/QuitProg"),
//                         "Quit the program",
//                         NULL);
//
//    gtk_tooltips_set_tip(ttip, gtk_ui_manager_get_widget(ui_manager,
//                         "/MainMenu/ToolMenu/DasmPrg"),
//                         "Disassemble the program, sending output to the Listing Window",
//                         NULL);
//
//    gtk_tooltips_set_tip(ttip, gtk_ui_manager_get_widget(ui_manager,
//                         "/MainMenu/ToolMenu/DasmToFile"),
//                         "Disassemble the program, sending output to a text file",
//                         NULL);
//
//    gtk_tooltips_set_tip(ttip, gtk_ui_manager_get_widget(ui_manager,
//                         "/MainMenu/OptionMenu/DisOptions"),
//                         "Set command-line options\n to pass to disassembler",
//                         NULL);
//
//    gtk_tooltips_enable(ttip);
//
//    /* ***************************** *
//     * Insensitize select menu items *
//     * ***************************** */
//
//    menu_do_dis_sensitize();
///*    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
//                                 "/MainMenu/FileMenu/SaveAsMenu/CmdSaveAs"),
//                             FALSE);
//    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
//                                 "/MainMenu/FileMenu/SaveAsMenu/LblSaveAs"),
//                             FALSE);*/
//    
//    mnubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
//    return mnubar;
//}

//declare the Window procedure where all messages will be handled
//LRESULT CALLBACK WndProc (HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);


