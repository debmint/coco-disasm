/* *********************************************** *
 *                                                 *
 * odisopts.c - setup options to pass to assembler *
 *                                                 *
 * $Id$ *
 * NOTE:  This newer version will create the       *
 *   widgets within a single table rather than     *
 *   stacking hboxes within the dialog->vbox       *
 *                                                 *
 * *********************************************** */

#include <gtk/gtk.h>
#include <string.h>
#include "g09dis.h"

struct ofile_widgets
{
    GtkWidget *o_entry;
    GtkWidget *browse_button;
};


/* create a new spin button */
GtkWidget *
newspin1 (int defval)
{
    GtkWidget *spin;

    spin = gtk_spin_button_new_with_range ((gdouble) 40, (gdouble) 200, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), (gdouble) defval);

    return spin;
}

static void
browse_files (GtkButton * button, GtkWidget * entry)
{
    selectfile_open (&O9Dis, "Prog Listing");

    if (O9Dis.filename_to_return)
    {
        gtk_entry_set_text (GTK_ENTRY (entry), O9Dis.filename_to_return);
    }

    free_filename_to_return (&(O9Dis.filename_to_return));
}

static GtkWidget *
build_entry_frame (gchar * title, GtkWidget * main_box, GtkWidget ** m_frame)
{
    GtkWidget *entry_box;
    GtkWidget *alignment1;

    *m_frame = gtk_frame_new (title);
    gtk_container_set_border_width (GTK_CONTAINER (*m_frame), 5);
    alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
    entry_box = gtk_vbox_new (TRUE, 5);        /* VBox to hold entry widgets */
    gtk_container_add (GTK_CONTAINER (*m_frame), alignment1);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 5, 5);
    gtk_container_add (GTK_CONTAINER (alignment1), entry_box);

    return entry_box;
}

static GtkWidget *
build_entry_entry (GtkWidget * entry_box, gchar * filnam)
{
    GtkWidget *entry;

    entry = gtk_entry_new ();
    gtk_widget_set_size_request( entry, 300, -1 );
    gtk_box_pack_start (GTK_BOX (entry_box), GTK_WIDGET (entry),
                        FALSE, FALSE, 5);

    if (filnam != NULL)
    {
        gtk_entry_set_text (GTK_ENTRY (entry), filnam);
    }

    return entry;
}

static GtkWidget *
build_browse_button (GtkWidget * entry_box, GtkWidget * entry)
{
    GtkWidget *browse_button;

    browse_button = gtk_button_new_with_label ("Browse");
    gtk_box_pack_end (GTK_BOX (entry_box), browse_button, FALSE, FALSE, 2);

    g_signal_connect (G_OBJECT (browse_button), "clicked",
                      G_CALLBACK (browse_files), entry);

    return browse_button;
}

static GtkWidget *
build_entry_box (gchar * title, GtkWidget * main_box,
                 gchar * filnam, GtkWidget **m_frame)
{
    GtkWidget *entry_box;
    GtkWidget *entry;

    entry_box = build_entry_frame (title, main_box, m_frame);
    entry = build_entry_entry (entry_box, filnam);
    build_browse_button (entry_box, entry);

    gtk_widget_show_all (*m_frame);

    return entry;
}

static void
on_ofile_tgle (GtkToggleButton * tbutton, struct ofile_widgets *srcf)
{
    gboolean cond;

    cond = gtk_toggle_button_get_active (tbutton);
    gtk_widget_set_sensitive (srcf->o_entry, cond);
    gtk_widget_set_sensitive (srcf->browse_button, cond);
}

static GtkWidget *
hbox_pack_in_box (GtkBox *bigbox)
{
    GtkWidget *hbx;
    
    hbx = gtk_hbox_new (TRUE, 10);
    gtk_box_pack_start (bigbox, hbx, FALSE, FALSE, 2);

    return hbx;
}

/* ********************************************************** *
 *                                                            *
 *           Dialog to Set Disassembler Options               *
 *                                                            *
 * ********************************************************** */

void
set_dis_opts_cb (GtkAction * action, glbls * hbuf)
{
    GtkWidget *dialog,
              *table,
              *bin_entry, *cmd_entry,
              *w_frame, *d_frame,
              *w_spin, *d_spin,
              *CPU_toggle, *UpCase_toggle,
              *ofile_toggle, *listing_toggle, *listing_box,
              *list_radio_file, *list_radio_gtk, *list_radio_none,
              *label, *vbx, *vbx2;
    GtkWidget *hsep1;
    GSList * list_radio_group = NULL;
    gint result;
    GtkTooltips *optips;

    GtkWidget *entry_box, *m_frame;
    struct ofile_widgets srcfile;
    struct ofile_widgets lstfile;

    dialog =
        gtk_dialog_new_with_buttons ("Command Line options for os9disasm",
                                     GTK_WINDOW (window),
                                     GTK_DIALOG_MODAL |
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                     NULL);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

    optips = gtk_tooltips_new();
    
    /* ********************** *
     * Create options widgets *
     * ********************** */

    /* Create a table into which to place two entry boxes side by side */

    table = gtk_table_new( 3, 2, FALSE );
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                       table, FALSE, FALSE, 2);
    
    /* ***** Program file to be disassembled ***** */
    
    bin_entry = build_entry_box ("Program Name",
                                 table, bin_file, &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                                0, 1, 0, 1,
                                0, 0,
                                5, 2);

    /* ***** Command file ( -c option ) ***** */
    
    cmd_entry = build_entry_box ("Command File",
                                  table, cmd_cmd, &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      1, 2, 0, 1,
                      0, 0,
                      5, 2);

    /* ***** output file ( -o option ) ***** */

    entry_box = build_entry_frame ("Assembly Source File",
                                   table, &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      0, 1, 1, 2,
                      0, 0,
                      5, 2);

    ofile_toggle = gtk_check_button_new_with_label ("Output .asm source file");
    gtk_box_pack_start (GTK_BOX (entry_box), ofile_toggle, FALSE, FALSE, 2);
    srcfile.o_entry = build_entry_entry (entry_box, obj_file);
    srcfile.browse_button = build_browse_button (entry_box, srcfile.o_entry);

    g_signal_connect (G_OBJECT (ofile_toggle), "toggled",
                      G_CALLBACK (on_ofile_tgle), &srcfile);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ofile_toggle),
                                  write_obj);
    
    on_ofile_tgle (GTK_TOGGLE_BUTTON (ofile_toggle), &srcfile);
    gtk_widget_show_all (m_frame);

    /* ***** send Listing to file ( > filename ) ***** */

    entry_box = build_entry_frame("Formatted File Listing ",
                                  table,
                                  &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      1, 2, 1, 2,
                      0, 0,
                      5, 2);
    
    vbx = gtk_vbox_new (FALSE, 2);
    gtk_widget_set_size_request(vbx,300,-1);
    list_radio_file = gtk_radio_button_new_with_label (NULL,
                                                       "Output to file/stdout");
    list_radio_group = gtk_radio_button_get_group (
                         GTK_RADIO_BUTTON(list_radio_file));
    g_signal_connect (G_OBJECT (list_radio_file), "toggled",
                      G_CALLBACK (on_ofile_tgle), &lstfile);

    /* set tooltip for list_radio_file label */
    gtk_tooltips_set_tip (optips, list_radio_file,
                          "Write output to file\nor stdout if filename is blank",
                          NULL);
    listing_box = gtk_vbox_new (FALSE, 2);
    lstfile.o_entry = build_entry_entry (listing_box, listing_output);
    lstfile.browse_button = build_browse_button (listing_box, lstfile.o_entry);
    /*lstfile.browse_button = gtk_button_new_with_label ("Browse");
    g_signal_connect (G_OBJECT (lstfile.browse_button), "clicked",
                      G_CALLBACK (browse_files), &lstfile);
    gtk_box_pack_start (GTK_BOX(listing_box), lstfile.browse_button,
                        FALSE, FALSE, 2);*/
    
    list_radio_none = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file), "No output");
    list_radio_gtk = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file),
                        "Build for GUI");
    
    /* Now determine which radio button to set active */
    
    {
        GtkToggleButton * tb;
        
        switch (write_list) {
            case LIST_FILE:
                tb = GTK_TOGGLE_BUTTON(list_radio_file);
                break;
            case LIST_GTK:
                tb = GTK_TOGGLE_BUTTON(list_radio_gtk);
                break;
            case LIST_NONE:
                tb = GTK_TOGGLE_BUTTON(list_radio_none);
                break;
            default:
                g_print("Internal error: invalid listing output!!!\n");
        }
        gtk_toggle_button_set_active(tb, TRUE);
    }
            
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_file, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX(vbx), listing_box, FALSE, FALSE, 2);
    hsep1 = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbx), hsep1,
                        FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_gtk, FALSE, FALSE, 2);
    hsep1 = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbx), hsep1,
                        FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_none, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(entry_box), vbx);
    gtk_widget_show_all (m_frame);
    gtk_widget_show_all(table);

    /* Begin page setup */
    w_frame = gtk_frame_new ("Page Width");
    w_spin = newspin1 (pgwdth);

    gtk_table_attach( GTK_TABLE(table), w_frame,
                      0, 1, 2, 3,
                      0, 0,
                      10, 5);
    
    gtk_container_set_border_width (GTK_CONTAINER (w_frame), 5);
    gtk_container_add (GTK_CONTAINER (w_frame), w_spin);
    gtk_widget_show_all (w_frame);

    d_frame = gtk_frame_new ("Page Depth");
    d_spin = newspin1 (pgdpth);
    gtk_table_attach( GTK_TABLE(table), d_frame,
                      1, 2, 2, 3,
                      0, 0,
                      5, 2);
    
    gtk_container_set_border_width (GTK_CONTAINER (d_frame), 5);
    gtk_container_add (GTK_CONTAINER (d_frame), d_spin);
    gtk_widget_show_all (d_frame);
    /* ***** end page setup ***** */

    /* ***** Now do toggle buttons ***** */
    hsep1 = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hsep1,
                        FALSE, FALSE, 4);
    gtk_widget_show (hsep1);
    CPU_toggle = gtk_check_button_new_with_label ("CPU = 6309");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), CPU_toggle,
                        FALSE, FALSE, 2);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (CPU_toggle),
                                  (gboolean) cputype);
    gtk_widget_show (CPU_toggle);

    UpCase_toggle = gtk_check_button_new_with_label ("Fold to Upper Case");
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), UpCase_toggle,
                        FALSE, FALSE, 2);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (UpCase_toggle),
                                  (gboolean) upcase);
    gtk_widget_show (UpCase_toggle);

    switch (result = gtk_dialog_run (GTK_DIALOG (dialog)))
    {
    case GTK_RESPONSE_OK:
    case GTK_RESPONSE_ACCEPT:
        pgdpth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d_spin));
        pgwdth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w_spin));

        if (bin_file != NULL)
        {
            g_free (bin_file);
        }
        
        bin_file = g_strdup (gtk_entry_get_text (GTK_ENTRY (bin_entry)));

        if (!strlen(bin_file))
        {
            g_free(bin_file);
            bin_file = NULL;
        }

        if (cmd_cmd != NULL)
        {
            g_free (cmd_cmd);
        }

        cmd_cmd = g_strdup (gtk_entry_get_text (GTK_ENTRY (cmd_entry)));

        if (!strlen(cmd_cmd))
        {
            g_free(cmd_cmd);
            cmd_cmd = NULL;
        }

        cputype =
            (gint)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (CPU_toggle));
        upcase =
            (gint)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (UpCase_toggle));

        if ((write_obj =
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ofile_toggle))))
        {
            if (obj_file != NULL)
            {
                g_free (obj_file);
            }
            obj_file =
                g_strdup (gtk_entry_get_text (GTK_ENTRY (srcfile.o_entry)));
        }

        /* Handle listing output */
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list_radio_file)))
        {
            write_list = LIST_FILE;
            if(listing_output != NULL)
            {
                g_free(listing_output);
            }
            listing_output =
                g_strdup (gtk_entry_get_text (GTK_ENTRY (lstfile.o_entry)));
        }
        else {
            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list_radio_gtk))) 
            {
                write_list = LIST_GTK;
            }
            else {  /* output = "none" */
                write_list = LIST_NONE;
            }
        }

        break;
    default:
        break;
    }

    gtk_widget_destroy (dialog);

}

