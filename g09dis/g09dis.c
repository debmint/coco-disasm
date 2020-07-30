/* **************************************************************** *
 * g09dis.c - GTK frontend for os9disasm.  This is the mainline     $
 *      file for the system.  This frontend uses the commandline    $
 *      component of os9disasm.                                     $
 *                                                                  $
 * $Id::                                                            $
 * **************************************************************** */


#define MAIN
#define WWIDTH 750
#define WHEIGHT 540

/*GtkWidget *tview;*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "g09dis.h"

/*GtkWidget *window;*/
GtkTextBuffer *buffer;
GString *odlist;

static GtkWidget *listPopup, *lblPopup;

char *text;

static void
cmdbuf_changed_cb (GtkTextBuffer *buffer, FILEINF *cmdbuf)
{
    doc_set_modified (cmdbuf, TRUE);
}

/* ************************************************************ *
 * Create and set up a text window ( cmd file )                 *
 *   Passed:   the box into which new window goes ptr to valid  *
 *             FILEINF data structure                           *
 * ************************************************************ */

static GtkWidget *
build_cmd_window (GtkWidget * mainbox, FILEINF * fi)
{
    GtkWidget *s_win;

    /* Create a scrolled window */
    
    s_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_set_border_width (GTK_CONTAINER (s_win), 2);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (s_win),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    /* Create a textview and add to scrolled window */
    
    fi->tview = gtk_text_view_new ();
    gtk_widget_set_name (GTK_WIDGET (fi->tview), "txtwin");
    /*gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (s_win),
                                           fi->tview);*/
    gtk_container_add(GTK_CONTAINER(s_win), fi->tview);

    /* Add text buffer to textview */
    
    fi->tbuf = gtk_text_buffer_new (NULL);
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (fi->tview), fi->tbuf);
    
    gtk_widget_show (s_win);
    
    g_signal_connect (fi->tbuf, "changed",
                      G_CALLBACK(cmdbuf_changed_cb), &O9Dis.cmdfile);
    gtk_widget_show (fi->tview);

    return (s_win);

}

/* **************************************************** *
 * build_list_window() - Creates a Window containing a  *
 *                 Treeview for the listing             *
 * Passed:  (1) Container for Treeview window           *
 *          (2) Associated FILEINF structure            *
 * Returns: The Window widget                           *
 * **************************************************** */

static GtkWidget *
build_list_window (GtkWidget * mainbox, FILEINF * fi)
{
    GtkWidget *view, *s_win;
    GtkCellRenderer *renderer;

    s_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (s_win),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    view = gtk_tree_view_new ();
    gtk_widget_set_name (GTK_WIDGET (view), "txtwin");

    /* Create columns */

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 -1, NULL,
                                                 renderer, 
                                                 "text", LST_LIN, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_ADR, "Addr",
                                                 renderer,
                                                 "text", LST_ADR, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_OPCO, "OPC",
                                                 renderer,
                                                 "text", LST_OPCO, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_PBYT, "Pbytes",
                                                 renderer,
                                                 "text", LST_PBYT, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_LBL, "Label",
                                                 renderer,
                                                 "text", LST_LBL, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_MNEM, "Mnem",
                                                 renderer,
                                                 "text", LST_MNEM, NULL);
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                 LST_OPER, "Oper",
                                                 renderer,
                                                 "text", LST_OPER, NULL);
    
    fi->l_store = gtk_list_store_new (LST_NCOLS, G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_STRING,
                                                 G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (view),
                             GTK_TREE_MODEL (fi->l_store));

    g_signal_connect (G_OBJECT(view), "button-press-event",
                      G_CALLBACK(onListRowButtonPress), listPopup);

    gtk_container_add (GTK_CONTAINER (s_win), view);

    /* save treeview to global storage */
    fi->tview = view;
    
    return s_win;
}

/* **************************************************************** *
 * new_lbl_win () - create a window with a TreeView for label file  *
 * Passed:  &Os9Dis.lblfile.tview                                   *
 * Returns: The new scrolled window Widget                          *
 *                                                                  *
 * **************************************************************** */

static GtkWidget *
new_lbl_win (GtkWidget **view)
{
    GtkWidget *s_win;
    GtkCellRenderer *renderer;

    s_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (s_win),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    *view = gtk_tree_view_new ();
    gtk_widget_set_name (GTK_WIDGET (*view), "lblwin");

    /* ************** *
     * Create columns *
     * ************** */

    /* Label column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (*view),
                                                 -1, "Label", renderer,
                                                 "text", LBL_LBL, NULL);
    /* "equ" column - this will always be the string "equ" */
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (*view),
                                                 -1, "       ", renderer,
                                                 "text", LBL_EQU, NULL);
    /* Address column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (*view),
                                                 -1, "Address", renderer,
                                                 "text", LBL_ADDR, NULL);
    /* Class column - a single character */
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (*view),
                                                 -1, "Class", renderer,
                                                 "text", LBL_CLASS, NULL);
    /* TODO : Perhaps add an additional comment column?? */

    O9Dis.lblfile.l_store = gtk_list_store_new (LBL_NCOLS,
                                                G_TYPE_STRING, G_TYPE_STRING,
                                                G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (*view),
                             GTK_TREE_MODEL (O9Dis.lblfile.l_store));

    g_signal_connect (G_OBJECT(*view), "button-press-event",
                      G_CALLBACK(onListRowButtonPress), lblPopup);

    gtk_container_add (GTK_CONTAINER (s_win), *view);

    return s_win;
}

/* set default values for variables */
static void
odis_init (void)
{
    LastPath = NULL;
    pgwdth = 80;
    pgdpth = 66;
}

/* **************************************************** *
 * window_quit() - cleanup before exit                  *
 * - checks for and notifies user about modified files  *
 * **************************************************** */

gboolean
window_quit()
{
    gboolean savelbl = FALSE,
             savecmd = FALSE,
             savenone = FALSE,
             cmdfile_altered
                 = gtk_text_buffer_get_modified (O9Dis.cmdfile.tbuf);
    
    if(O9Dis.lblfile.altered
            && cmdfile_altered)
    {
        switch (save_all_query() ) {
            case SAVALL_NONE:
                savenone = TRUE;
                break;
            case SAVALL_ALL:
                savelbl = savecmd = TRUE;
                break;
            case SAVALL_SOME:
                switch (save_warn_OW ("Command", O9Dis.cmdfile.fname, FALSE)) {
                    case GTK_RESPONSE_YES:
                        savecmd = TRUE;
                        break;
                    default:
                        break;
                }
                switch (save_warn_OW ("Label", O9Dis.lblfile.fname, FALSE)) {
                    case GTK_RESPONSE_YES:
                        savelbl = TRUE;
                        break;
                    default:
                        break;
                }
                break;
        }
    }

    if ( ! savenone )
    {
        if (cmdfile_altered && !savecmd)
        {
            if (save_warn_OW ("Command", O9Dis.cmdfile.fname, FALSE) ==
                    GTK_RESPONSE_YES)
            {
                savecmd = TRUE;
            }
        }

        if (O9Dis.lblfile.altered && !savelbl)
        {
            if (save_warn_OW ("Label", O9Dis.lblfile.fname, FALSE) ==
                    GTK_RESPONSE_YES)
            {
                savelbl = TRUE;
            }
        }
    }

    if (savecmd)
    {
        cmd_save (NULL, &O9Dis);
    }

    if (savelbl)
    {
        lbl_save (NULL, &O9Dis);
    }
    
    gtk_main_quit ();

    return TRUE;
}

GtkWidget *
create_main_window(gchar *home)
{
    GtkBuilder *builder;
    gchar *gladesrc;
    gladesrc = g_strconcat(home, home[strlen(home) - 1] == '/' ? "" : "/",
            ".config/g09dis/glade/gdis.glade", NULL);
    builder = gtk_builder_new_from_file(gladesrc);
    w_main = GTK_WIDGET(gtk_builder_get_object(builder, "w_main"));
    listPopup = GTK_WIDGET(gtk_builder_get_object(builder,"mnu_lstpopup"));
    lblPopup = GTK_WIDGET(gtk_builder_get_object(builder,"mnu_lblpopup"));
    O9Dis.mnuToolDasm = GTK_WIDGET(gtk_builder_get_object(builder,"distogui"));
    O9Dis.mnuToolDasmFile = GTK_WIDGET(gtk_builder_get_object(builder,"distofile"));
    gtk_builder_connect_signals(builder, &O9Dis);
    g_free(gladesrc);
    return GTK_WIDGET(gtk_builder_get_object(builder, "vbox_main"));
}

/* ************************************ *
 * The main entry point for the program *
 * ************************************ */

int
main (int argc, char *argv[])
{
    GtkWidget *main_vbox;
    GtkWidget *work_area;
    GtkWidget *panedv;
    GtkWidget *fw;
    gchar *ho;

    /* Go set defaults, etc */
    
    odis_init ();

    /* Initialize GTK */

    if (!(ho=getenv ("HOME")))
    {
        if (!(ho=getenv ("home")))
        {
            ho = ".";   /* Assign a default value */
        }
    }

    gtk_init (&argc, &argv);
    main_vbox = create_main_window(ho);

    
    work_area = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), work_area, TRUE, TRUE, 5);
    gtk_widget_show (work_area);

    /*      create List window */
    
    list_win = build_list_window (main_vbox, &O9Dis.list_file);
    fw = gtk_frame_new ("Program Listing");
    gtk_container_add (GTK_CONTAINER (fw), list_win);
    gtk_paned_pack1 (GTK_PANED (work_area), /*list_win */ fw, TRUE, TRUE);
    
    panedv = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_pack2 (GTK_PANED (work_area), panedv, TRUE, TRUE);

    /* cmd window to top of right-hand window */
    
    cmd_win = build_cmd_window (work_area, &O9Dis.cmdfile);
    fw = gtk_frame_new ("Command File");
    gtk_frame_set_label_align (GTK_FRAME (fw), 1.0, 0.5);
    gtk_container_add (GTK_CONTAINER (fw), cmd_win);
    gtk_paned_pack1 (GTK_PANED (panedv), /*cmd_win */ fw, TRUE, TRUE);

    /* and the label file window to the bottom of right window */
    
    lbl_win = new_lbl_win (&O9Dis.lblfile.tview);
    fw = gtk_frame_new ("Label Files");
    gtk_frame_set_label_align (GTK_FRAME (fw), 1.0, 0.5);
    gtk_container_add (GTK_CONTAINER (fw), lbl_win);
    gtk_paned_pack2 (GTK_PANED (panedv), fw, TRUE, TRUE);

    /* Show the widgets */

    gtk_widget_show_all (w_main);

    /* Initialize font and colors */
    update_lists (&O9Dis.list_file);
    update_lists (&O9Dis.cmdfile);
    update_lists (&O9Dis.lblfile);

    /* Finished - now enter continuous loop */
    gtk_main ();

    return 0;
}
