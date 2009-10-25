/* ************************************************************************ *
 * search.c - Handles search routines for g09dis - the graphical interface  $
 *            for os9disasm.                                                $
 * $Id::                                                                    $
 * ************************************************************************ */

#include <gtk/gtk.h>
#include "g09dis.h"

/* struct srch_log: Keeps data for a treeview, for setting up a search. */

struct srch_log
{
    GtkWidget * buttongrp;  /* The radio group for the treeviews column btns */
    GtkWidget * radios_box; /* The vbox containing the buttons - to show/hide*/
    fileinf * tvdat;        /* The global data for the treeview */
    gchar * last_srch;      /* The previous string searched for in this TV   */
    GHashTable * radio_pos; /* Hash of names=>column position (to search)    */
};

static GtkWidget * srch_dialog,
                 * srch_entry;
static GHashTable *allviews;


/* ************************************************************************ *
 * box_add_new_button() - Appends a new button to a GtkBox                  *
 *      button group.                                                       *
 * Passed : (1) - GtkBox * into which to pack new button                    *
 *                (passed as GtkWidget *)                                   *
 *          (2) - Address of radio button group. - NULL to create a new grp *
 *          (3) - Label to append to button                                 *
 * ************************************************************************ */

static GtkWidget *
box_add_new_button (GtkWidget *box, GtkWidget *button, gchar *txt)
{
    GtkWidget *newbtn;

    if (button)
    {
        newbtn = gtk_radio_button_new_with_label_from_widget (
                                                GTK_RADIO_BUTTON(button), txt);
    }
    else
    {
        newbtn = gtk_radio_button_new_with_label (NULL, txt);
    }

    gtk_box_pack_start_defaults (GTK_BOX(box), newbtn);
    gtk_widget_show (newbtn);

    return newbtn;
}

/* ************************************************************************ *
 * build_listings_radio_button_group () - Creates a list of radio buttons   *
 *      for search column choice and packs them into a GtkVBox.             *
 *      Also creates and sets up the log table which will hold data for     *
 *      subsequent calls to search this table.                              *
 * Returns: The VBox into which the radio buttons are packed.               *
 * ************************************************************************ */

static GtkWidget *
build_listings_radio_button_group ()
{
    GtkWidget * buttons_vbox,
              * button;
    struct srch_log * shown_radios;

    buttons_vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_set_name (buttons_vbox, "listingcols");

    button = box_add_new_button (buttons_vbox, NULL, "Address");
    button = box_add_new_button (buttons_vbox, button, "Label");
    button = box_add_new_button (buttons_vbox, button, "Operand");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), TRUE);

    /* Build log table */

    g_hash_table_insert (allviews, "listingcols",
                                   g_malloc0 (sizeof (struct srch_log)));

    if ( ! (shown_radios = g_hash_table_lookup (allviews, "listingcols")))
    {
        abort_warn ("Fatal: Cannot allocate memory for srch_log");
    }

    shown_radios->radios_box = buttons_vbox;
    shown_radios->buttongrp = button;
    shown_radios->tvdat = &O9Dis.list_file;

    /* Create the column positions table */

    shown_radios->radio_pos = g_hash_table_new (g_str_hash, g_int_equal);
    g_hash_table_insert (shown_radios->radio_pos, "Address",
                                                 GINT_TO_POINTER(LST_ADR));
    g_hash_table_insert (shown_radios->radio_pos, "Label",
                                                 GINT_TO_POINTER(LST_LBL));
    g_hash_table_insert (shown_radios->radio_pos, "Operand",
                                                 GINT_TO_POINTER(LST_OPER));

    return buttons_vbox;
}

/* ************************************************************************ *
 * build_labels_radio_button_group () - Creates a list of radio buttons     *
 *      for search column choice and packs them into a GtkVBox.             *
 *      Also creates and sets up the log table which will hold data for     *
 *      subsequent calls to search this table.                              *
 * Returns: The VBox into which the radio buttons are packed.               *
 * ************************************************************************ */

static GtkWidget *
build_labels_radio_button_group ()
{
    GtkWidget * buttons_vbox,
              * button;
    struct srch_log * shown_radios;

    buttons_vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_set_name (buttons_vbox, "labelscols");

    button = box_add_new_button (buttons_vbox, NULL, "Label");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), TRUE);
    button = box_add_new_button (buttons_vbox, button, "Address");

    /* Build log table */

    g_hash_table_insert (allviews, "labelscols",
                                   g_malloc0 (sizeof (struct srch_log)));

    if ( ! (shown_radios = g_hash_table_lookup (allviews, "labelscols")))
    {
        abort_warn ("Fatal: Cannot allocate memory for srch_log");
    }

    shown_radios->radios_box = buttons_vbox;
    shown_radios->buttongrp = button;
    shown_radios->tvdat = &O9Dis.lblfile;

    /* Create the column positions table */

    shown_radios->radio_pos = g_hash_table_new (g_str_hash, g_int_equal);
    g_hash_table_insert (shown_radios->radio_pos, "Label",
                                                 GINT_TO_POINTER(LBL_LBL));
    g_hash_table_insert (shown_radios->radio_pos, "Address",
                                                 GINT_TO_POINTER(LBL_ADDR));

    return buttons_vbox;
}

/* ************************************************************************ *
 * do_search () - The generic search function for all the treeviews         *
 *      The main dialog is created on first call to search.                 *
 *      A vbox (cols_vbox) is packed with a vbox of column choice radio     *
 *      buttons for each treeview to search.                                *
 *      The appropriate set is shown on entry with gtk_widget_show(), and   *
 *      is again hidden before exit.                                        *
 * Passed : (1) - The TreeView pointer.                                     *
 *          (2) - The title for the dialog.                                 *
 *          (3) _ The Name of the radiogroup to display.                    *
 * ************************************************************************ */

void
do_search (GtkWidget *widg, gchar *title, gchar *radioname)
{
    struct srch_log * shown_radios;
    GtkTreePath * tpath = NULL,
                * stop_path = NULL;
    gint response;
    gchar * curnt_str;
    gint srch_col = -1;     /* Init to unreasonable value */

    /* If it's the first call to this function
     * create the search dialog and set it up
     */

    if ( ! srch_dialog)
    {
        GtkWidget * frame,
                  * cols_vbox,
                  * entry_hbox;

        if ( ! allviews)
        {
            allviews = g_hash_table_new (g_str_hash, g_str_equal);
        }

        srch_dialog = gtk_dialog_new_with_buttons (
                    NULL,
                    NULL,
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                    "Search Backward", 1,
                    "Search Forward", 2,
                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                    NULL);
        gtk_widget_hide_on_delete (srch_dialog);

        frame = gtk_frame_new ("Column to Search");
        gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);

        /* The VBox to contain all the buttons groups */

        cols_vbox = gtk_vbox_new (FALSE, 5);

        /* Build the Listings search data */

        gtk_box_pack_start_defaults (GTK_BOX(cols_vbox),
                                     build_listings_radio_button_group ());

        /* The Labels search data */

        gtk_box_pack_start_defaults (GTK_BOX(cols_vbox),
                                     build_labels_radio_button_group ());

        /* All the tables are created, now do generic stuff
         * Add the all-elements vbox to the frame */

        gtk_container_add (GTK_CONTAINER(frame), cols_vbox);
        gtk_widget_show (cols_vbox);
        gtk_box_pack_start_defaults ( GTK_BOX(GTK_DIALOG(srch_dialog)->vbox),
                                      frame);
        gtk_widget_show (frame);

        /* the search entry */

        entry_hbox = gtk_hbox_new (FALSE, 5);
        gtk_box_pack_start_defaults (GTK_BOX(entry_hbox),
                                     gtk_label_new ("Enter Search Text"));
        srch_entry = gtk_entry_new();
        gtk_box_pack_start_defaults (GTK_BOX(entry_hbox), srch_entry);
        gtk_widget_show_all (entry_hbox);
        gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(srch_dialog)->vbox),
                                     entry_hbox);
    }

    shown_radios = g_hash_table_lookup (allviews, radioname); 

    if (shown_radios->last_srch)
    {
        gtk_entry_set_text (GTK_ENTRY(srch_entry), shown_radios->last_srch);
    }

    gtk_widget_show (shown_radios->radios_box);
    response = gtk_dialog_run (GTK_DIALOG(srch_dialog));
    gtk_widget_hide (shown_radios->radios_box);
    gtk_widget_hide (srch_dialog);
    curnt_str = (gchar *)(gtk_entry_get_text (GTK_ENTRY(srch_entry)));

    switch (response)
    {
        GtkWidget    * treeview;
        GtkTreeModel * store;
        GtkTreeIter    iter;
        GSList       * list_fe;
        GtkTreeSelection *selection;
        gchar * srch_txt,
              * store_txt;

        case 1:
        case 2:

            /* Save search name if different from old */

            if ( shown_radios->last_srch)
            {
                if ( g_ascii_strcasecmp (curnt_str, shown_radios->last_srch))
                {
                    if ( shown_radios->last_srch)
                    {
                        g_free (shown_radios->last_srch);
                    }
                }
            }

            shown_radios->last_srch =
                g_strdup (gtk_entry_get_text (GTK_ENTRY(srch_entry)));

            srch_txt = shown_radios->last_srch; /* For convenience & speed */

            /* Get column number */
            list_fe = gtk_radio_button_get_group (
                    GTK_RADIO_BUTTON(shown_radios->buttongrp));

            while (list_fe)
            {
                if (gtk_toggle_button_get_active (
                            GTK_TOGGLE_BUTTON(list_fe->data)))
                {
                    srch_col = GPOINTER_TO_INT(
                            g_hash_table_lookup (shown_radios->radio_pos,
                                gtk_button_get_label (GTK_BUTTON(
                                        list_fe->data))));
                    break;
                }

                list_fe = g_slist_next (list_fe);
            }

            if ( srch_col == -1)
            {
                break;
            }

            /* Now do search */

            treeview = shown_radios->tvdat->tview;
            selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(treeview));

            if ( ! gtk_tree_selection_get_selected (selection, &store, &iter))
            {
                gtk_tree_model_get_iter_first (store, &iter);

                if (response == 1)  /* Reverse search */
                {
                    gtk_tree_model_iter_nth_child (
                                store,
                                &iter,
                                NULL,
                                gtk_tree_model_iter_n_children (
                                                            store, NULL) - 1);
                }
            }

            /* Iter we began with */
            stop_path = gtk_tree_model_get_path (store, &iter);

            for (;;)
            {
                //tpath = gtk_tree_model_get_path (store, &iter);

                /* Test the iter */
                gtk_tree_model_get (store, &iter, srch_col, &store_txt, -1);

                if ( ! g_ascii_strcasecmp (store_txt, srch_txt))
                {
                    gtk_tree_selection_select_iter (selection, &iter);
                    break;
                }

                if (response == 2)      /* Forward */
                {
                    if ( ! gtk_tree_model_iter_next (store, &iter))
                    {
                        gtk_tree_model_get_iter_first (store, &iter);
                    }

                    tpath = gtk_tree_model_get_path (store, &iter);
                }
                else if (response == 1) /* Reverse */
                {
                    tpath = gtk_tree_model_get_path (store, &iter);

                    if (gtk_tree_path_prev (tpath))
                    {
                        gtk_tree_model_get_iter (store, &iter, tpath);
                    }
                    else
                    {
                        gtk_tree_model_iter_nth_child (
                                        store,
                                        &iter,
                                        NULL,
                                        gtk_tree_model_iter_n_children (
                                                                store,
                                                                NULL) - 1); 
                    }
                }

                if ( ! gtk_tree_path_compare (stop_path, tpath))
                {
                    gtk_tree_path_free (tpath);
                    break;
                }

                gtk_tree_path_free (tpath);
                tpath = NULL;
            }

            break;
        default:
            break;
    }

    if (stop_path)
    {
        gtk_tree_path_free (stop_path);
    }

}

/* **************************************************************** *
 * listing_srch () - callback for Action group entry to search the  *
 *          listing for a value.  This funcion does the setup for   *
 *          calling do_search(), the generic search function.       *
 * **************************************************************** */

void
listing_srch (GtkAction *action, gpointer user_data)
{
    do_search (O9Dis.list_file.tview, "Search in Listings", "listingcols");
}

/* ******************************************************************** *
 * labels_srch () - callback for Action group entry to search the       *
 *          labels list for a value.  This funcion does the setup for   *
 *          calling do_search(), the generic search function.           *
 * ******************************************************************** */

void
labels_srch (GtkAction *action, gpointer user_data)
{
    do_search (O9Dis.lblfile.tview, "Search in Labels", "labelscols");
}
