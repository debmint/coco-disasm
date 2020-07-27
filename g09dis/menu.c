/* ************************************************************ *
 * menu.c - menu-related items : create, etc                    $
 * This version is based upon GtkActionEntry and GUIManager     $
 *                                                              $
 * $Id::                                                        $
 * ************************************************************ */


#include <stdlib.h>
#include <string.h>
#include "g09dis.h"

static void
hlp_about (GtkAction * action, glbls * hbuf)
{
    GtkWidget *dialog;
    gchar *msg;

    msg = g_strconcat ("  -- G09Dis V ",VERSION," --\n\n",
                       "  A graphical front-end to \"os9disasm\"\n",
                       "  This version is strictly BETA!!!",
                       NULL);

    dialog = gtk_message_dialog_new (GTK_WINDOW(w_main),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                     msg);

    gtk_dialog_run (GTK_DIALOG (dialog));
    g_free (msg);
    gtk_widget_destroy (dialog);
}

void
menu_do_dis_sensitize(void)
{
    gint binstat;

    binstat = (prog_wdg->fname) && (strlen(prog_wdg->fname) > 0);
    
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
                             "/MainMenu/ToolMenu/DasmPrg"),
                             binstat);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
                             "/MainMenu/ToolMenu/DasmToFile"),
                             binstat);
    
}
static void
listing_new_cb (GtkAction * action, glbls * hbuf)
{
    list_store_empty(&(hbuf->list_file));
}

static void
cmd_new_cb (GtkAction * action, glbls * hbuf)
{
    clear_text_buf(&(hbuf->cmdfile));
}

static void
lbl_new_cb (GtkAction * action, glbls * hbuf)
{
    list_store_empty(&(hbuf->lblfile));
}

static void
tip_toggle(GtkToggleAction *ta, glbls *hbuf)
{
    if( gtk_toggle_action_get_active(ta) )
    {
        sysfailed ("Tooltips are now ON...", FALSE);
    }
    else {
        sysfailed ("Tooltips are now OFF...", FALSE);
    }
}

/* Construct a GtkActionEntry */

/* Normal items */
static GtkActionEntry entries[] = {
    {"FileMenu", NULL, "_File"},
    {"ViewMenu", NULL, "_View"},
    {"ToolMenu", NULL, "_Tools"},
    {"OptionMenu", NULL, "_Options"},
    {"DisOptions0", NULL, "Disassembler Options0"},
    {"HelpMenu", NULL, "_Help"},
    {"File_New", GTK_STOCK_NEW, "_New"},
    {"OpenMenu", GTK_STOCK_OPEN, "_Open"},
    {"SaveAsMenu", GTK_STOCK_SAVE_AS, "Save _As"},
    {"FontsChange", NULL, "Fonts", NULL,
        "Change Font settings for labels, listing, and command displays",
        G_CALLBACK(fonts_main_dialog)},
    {"DisOptions", GTK_STOCK_EXECUTE, "Disassembler Options", NULL,
        "Set disassembler options/parameters", G_CALLBACK (set_dis_opts_cb)},
    {"AModeListEdit", NULL, "Modify Amode List", NULL,
        "Change Addressing Mode List", G_CALLBACK (amode_list_edit_cb)},
    {"DasmPrg", GTK_STOCK_EXECUTE, "Disassemble to GUI", NULL,
        "Disassemble program file", G_CALLBACK (run_disassembler)},
    {"DasmToFile", GTK_STOCK_EXECUTE, "Disassemble to File", NULL,
        "Disassemble, sending normal listing to file",
        G_CALLBACK(dasm_list_to_file_cb)},
    {"LstngNew", GTK_STOCK_NEW, "_Listing", "<shft><ctl>L",
        NULL, G_CALLBACK(listing_new_cb)},
    {"CmdNew", GTK_STOCK_NEW, "_Command File", "<shft><ctl>C",
        NULL, G_CALLBACK(cmd_new_cb)},
    {"LblNew", GTK_STOCK_NEW, "La_bel File", "<shft><ctl>B",
        NULL, G_CALLBACK(lbl_new_cb)},
    {"LstngSearch",GTK_STOCK_FIND, "Search Listing", NULL,
        "Search listings for item", G_CALLBACK (listing_srch)},
    {"LstngOpen", GTK_STOCK_OPEN, "Open _Listing", "<control>L",
     "Open a program listing", G_CALLBACK (load_listing)},
    {"CmdOpen", GTK_STOCK_OPEN, "Open _Command File", "<control>C",
        "Open the Command File", G_CALLBACK (load_cmdfile)},
    {"LblOpen", GTK_STOCK_OPEN, "Open La_bel File", "<control>B",
        "Open a Label File", G_CALLBACK (load_lblfile)},
    {"LblSearch",GTK_STOCK_FIND, "Search Labels", NULL,
        "Search label list for item", G_CALLBACK (labels_srch)},

    {"CmdSave", GTK_STOCK_SAVE, "Save CMDFILE", NULL,
        "Save Cmd file", G_CALLBACK (cmd_save),},
    {"CmdSaveAs", GTK_STOCK_SAVE_AS, "Save CMDFILE As...", NULL,
        "Save Cmd file as..",G_CALLBACK (cmd_save_as)}, 

    {"LblSave", GTK_STOCK_SAVE, "Save LBLFILE", NULL,
        "Save Lbl file", G_CALLBACK (lbl_save)},
    {"LblSaveAs", GTK_STOCK_SAVE_AS, "Save LBLFILE As..", NULL,
        "Save Label file as..", G_CALLBACK (lbl_save_as)},
    {"LblInsertLin", NULL, "Insert line after", NULL,
        "Insert new line after", G_CALLBACK (lbl_insert_line)},
    {"LblDeleteLin", GTK_STOCK_DELETE, "Delete", NULL,
        "Delete Line", G_CALLBACK (lbl_delete_line)},

    {"OptsSave", GTK_STOCK_SAVE, "Save Options to File", NULL,
        "Save Options File", G_CALLBACK (opts_save)},
    {"OptsLoad", GTK_STOCK_OPEN, "Load Options from File", NULL,
        "Load Options File", G_CALLBACK (opts_load)},
    
    {"NameLabel", NULL, "Rename Label", NULL,
        "Define a Label Name", G_CALLBACK (rename_label)},
    {"BndsDefine", NULL, "Define Bounds", NULL,
        "Define Data Boundary", G_CALLBACK (bnds_define_cb)},
    {"AdrMode", NULL, "Set Addressing Mode", NULL,
        "Change Addressing Mode", G_CALLBACK (adr_mode_cb)},
    {"LblProps", NULL, "Properties", NULL,
        "Properties of this label", G_CALLBACK (lbl_properties)},
    {"QuitProg", GTK_STOCK_QUIT, "_Quit", "<control>Q",
     "Quit program", G_CALLBACK(window_quit)},
    {"HlpAbout", NULL, "About", NULL, NULL, G_CALLBACK (hlp_about)}
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
    {"ShowTips", NULL, "Show Tooltips", NULL, "Show tooltips",
        G_CALLBACK (tip_toggle), TRUE}
};

            /* Listing popup */
/*static GtkActionEntry list_group[] = {
    {"NameLabel", NULL, "Rename Label", NULL,
        "Define a Label Name", G_CALLBACK (rename_label)},
    {"BndsDefine", NULL, "Define Bounds", NULL,
        "Define Data Boundary", G_CALLBACK (bnds_define_cb)},
    {"AdrMode", NULL, "Set Addressing Mode", NULL,
        "Change Addressing Mode", G_CALLBACK (adr_mode_cb)}
};*/

/* The XML description of the menubar */

static const char *ui_description =
    "<ui>"
    "  <menubar name='MainMenu'>"
    "    <menu action='FileMenu'>"
    "      <menu action='File_New'>"
    "        <menuitem action='LstngNew'/>"
    "        <menuitem action='CmdNew'/>"
    "        <menuitem action='LblNew'/>"
    "      </menu>"
    "      <menu action='OpenMenu'>"
    "        <menuitem action='LstngOpen'/>"
    "        <menuitem action='CmdOpen'/>"
    "        <menuitem action='LblOpen'/>"
    "      </menu>"
    "      <separator/>"
    "      <menu action='SaveAsMenu'>"
    "        <menuitem action='CmdSaveAs'/>"
    "        <menuitem action='LblSaveAs'/>"
    "      </menu>"
    "      <menuitem action='CmdSave'/>"
    "      <menuitem action='LblSave'/>"
    "      <separator/>"
    "      <menuitem action='QuitProg'/>"
    "    </menu>"
    "    <menu action='ViewMenu'>"
    "      <menuitem action='FontsChange'/>"
    "    </menu>"
    "    <menu action='ToolMenu'>"
    "      <menuitem action='DasmPrg'/>"
    "      <menuitem action='DasmToFile'/>"
    "      <separator/>"
    "      <menuitem action='AModeListEdit'/>"
    "    </menu>"
    "    <menu action='OptionMenu'>"
    "      <menuitem action='DisOptions'/>"
    "      <separator/>"
    "      <menuitem action='OptsLoad'/>"
    "      <menuitem action='OptsSave'/>"
    "      <separator/>"
    "      <menuitem action='ShowTips'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HlpAbout'/>"
    "    </menu>"
    "  </menubar>"
    "  <popup name='ListPopUp'>"
    "    <menuitem action='LstngSearch'/>"
    "    <separator/>"
    "    <menuitem action='LstngOpen'/>"
    "    <separator/>"
    "    <menuitem action='NameLabel'/>"
    "    <menuitem action='BndsDefine'/>"
    "    <menuitem action='AdrMode'/>"
    "    <separator/>"
    "  </popup>"
    "  <popup name='LabelPopUp'>"
    "    <menuitem action='LblSearch'/>"
    "    <separator/>"
    "    <menuitem action='LblOpen'/>"
    "    <separator/>"
    "    <menuitem action='LblSave'/>"
    "    <menuitem action='LblSaveAs'/>"
    "    <separator/>"
    "    <menuitem action='LblInsertLin'/>"
    "    <menuitem action='LblDeleteLin'/>"
    "    <separator/>"
    "    <menuitem action='LblProps'/>"
    "  </popup>"
    "</ui>";

/* Returns a menubar widget made from the above menu */

/*extern GtkTreeSelection *list_selection;*/

GtkWidget *
get_menubar_menu (GtkWidget * main_window)
{
    GtkWidget *mnubar;
    GtkActionGroup *action_group;
    /*GtkActionGroup *list_action_grp;*/
    /*GtkUIManager *ui_manager;*/
    GtkAccelGroup *accel_group;
    GError *error;

    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, entries,
                                  G_N_ELEMENTS (entries), &O9Dis);
    gtk_action_group_add_toggle_actions (action_group, toggle_entries,
                                         G_N_ELEMENTS (toggle_entries),
                                         &O9Dis);

/*    list_action_grp = gtk_action_group_new("ListPopActions");
    gtk_action_group_add_actions (list_action_grp, list_group,
                                  G_N_ELEMENTS (list_group),
                                  &list_selection);*/
    
    ui_manager = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
    /*gtk_ui_manager_insert_action_group (ui_manager, list_action_grp, 0);*/

    accel_group = gtk_ui_manager_get_accel_group (ui_manager);
    gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);

    error = NULL;
    if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description,
                                            -1, &error))
    {
        g_message ("building menus failed %s", error->message);
        g_error_free (error);
        exit (EXIT_FAILURE);
    }

    /* add tooltips */
    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget( ui_manager,
                         "/MainMenu/FileMenu/File_New/"),
                         "Clears buffer for selected file");
    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget(ui_manager,
                         "/MainMenu/FileMenu/QuitProg"),
                         "Quit the program");

    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget(ui_manager,
                         "/MainMenu/ToolMenu/DasmPrg"),
                         "Disassemble the program, sending output to the Listing Window");

    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget(ui_manager,
                         "/MainMenu/ToolMenu/DasmToFile"),
                         "Disassemble the program, sending output to a text file");

    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget(ui_manager,
                         "/MainMenu/OptionMenu/DisOptions"),
                         "Set command-line options\nto pass to disassembler.");
    gtk_widget_set_tooltip_text(gtk_ui_manager_get_widget(ui_manager,
                         "/MainMenu/ViewMenu/FontsChange"),
                         "Reset Font, foreground and background colors for the view windows");

    /* ***************************** *
     * Insensitize select menu items *
     * ***************************** */

    menu_do_dis_sensitize();
/*    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
                                 "/MainMenu/FileMenu/SaveAsMenu/CmdSaveAs"),
                             FALSE);
    gtk_widget_set_sensitive(gtk_ui_manager_get_widget(ui_manager,
                                 "/MainMenu/FileMenu/SaveAsMenu/LblSaveAs"),
                             FALSE);*/
    
    mnubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    return mnubar;
}
