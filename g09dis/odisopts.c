/* ******************************************************************** *
 *                                                                      $
 * odisopts.c - setup options to pass to assembler                      $
 *                                                                      $
 * NOTE:  This newer version will create the widgets within a single    $
 *        table rather than stacking hboxes within the dialog->vbox     $
 *                                                                      $
 * $Id::                                                                $
 * ******************************************************************** */


#include <string.h>
#include <glib/gprintf.h>
#include "g09dis.h"

struct fontsel_data {
    gchar * oldfont,
          * newfont;
    GtkWidget * font_btn,
              * cb_fg,
              * cb_bg;
    GHashTable * list_to_widg;
    GtkStyle * style;
};

static GtkWidget * selected_view;
static GtkTooltips *optips;
static GtkTooltips *font_tips;  /* Created/destroyed each call to fonts dlg */

/* ************************************************ *
 * newspin1() - create a new spin button            *
 *    Passed: default value for button              *
 *    Returns: the spin button widget ptr           *
 * ************************************************ */

GtkWidget *
newspin1 (int defval)
{
    GtkWidget *spin;

    spin = gtk_spin_button_new_with_range ((gdouble) 40, (gdouble) 200, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), (gdouble) defval);

    return spin;
}

/* ************************************************************ *
 * browse_files() - get a filename using selectfile_open ()     *
 *                                                              *
 *   Passed: (1) - The button pressed                           *
 *           (2) - The associated FILE_WIDGET *                 *
 *   Returns: nothing                                           *
 * ************************************************************ */

static void
browse_files (GtkButton * button, FILE_WIDGET *wp)
{
    if (wp->is_read)
    {
        selectfile_open ( &O9Dis, wp->dialog_ttl, TRUE,
                          gtk_entry_get_text (GTK_ENTRY(wp->o_entry)));
    }
    else
    {
        selectfile_save ( &O9Dis,
                          gtk_entry_get_text(GTK_ENTRY(wp->o_entry)),
                          wp->dialog_ttl);
    }


    if ((O9Dis.filename_to_return && strlen (O9Dis.filename_to_return)))
    {
        gtk_entry_set_text (GTK_ENTRY(wp->o_entry), O9Dis.filename_to_return);
        
        if (LastPath)
        {
            g_free (LastPath);
        }

        LastPath = g_strdup(O9Dis.filename_to_return);
    }

    free_filename_to_return (&(O9Dis.filename_to_return));
}

/* **************************************************** *
 * browse_dirs() - same as browse_files() except        *
 *                 that it returns a directory          *
 * **************************************************** */

static void
browse_dirs (GtkButton * button, FILE_WIDGET *wp)
{
    selectfile_open (&O9Dis, wp->dialog_ttl, FALSE, NULL);

    if (O9Dis.filename_to_return)
    {
        gtk_entry_set_text (GTK_ENTRY (wp->o_entry), O9Dis.filename_to_return);
    }

    free_filename_to_return (&(O9Dis.filename_to_return));
}


/* **************************************************************** *
 * build_entry_frame () - Build a frame with a vbox inside it       *
 *                                                                  *
 *    Passed:   (1) title for dialog parent box for this new widget *
 *              (2) address to store frame ptr                      *
 *    Returns:  ptr to the entry vbox                               *
 *                                                                  *
 * **************************************************************** */

static GtkWidget *
build_entry_frame (const gchar *title, GtkWidget **m_frame)
{
    GtkWidget *entry_vbox;

    *m_frame = gtk_frame_new (title);
    gtk_container_set_border_width (GTK_CONTAINER (*m_frame), 10);

    entry_vbox = gtk_vbox_new (TRUE, 3);    /* VBox to hold entry widgets */
    gtk_container_set_border_width(GTK_CONTAINER(entry_vbox), 10);
    gtk_container_add (GTK_CONTAINER (*m_frame), entry_vbox);

    return entry_vbox;
}

/* **************************************************************** *
 * build_entry_entry() - create an entry widget  and pack it        *
 *                       inside the provided v/h-box                *
 *    Passed:  (1) - Container (v/h-box) to hold entry text (if     *
 *                   not NULL) to place into entry.                 *
 *             (2) - Filename                                       *
 *    Returns: the entry widget ptr                                 *
 *                                                                  *
 * **************************************************************** */

static GtkWidget *
build_entry_entry (GtkWidget * entry_box, gchar * filnam)
{
    GtkWidget *entry;

    entry = gtk_entry_new ();
    gtk_widget_set_size_request( entry, 300, -1 );
    gtk_box_pack_start (GTK_BOX (entry_box), GTK_WIDGET (entry),
                        FALSE, FALSE, 3);

    if (filnam != NULL)
    {
        gtk_entry_set_text (GTK_ENTRY (entry), filnam);
    }

    return entry;
}

/* ******************************************************************** *
 * build_browse_button () - create a browse button inside an alignment  *
 *                          to the right, placed inside the provided    *
 *                          entry_box, and connect a callback for       *
 *                          "clicked".                                  *
 *     Passed: (1) - The container into which to pack new button        *
 *             (2) - The entry widget to pass to callback as data.      *
 *             (3) - TRUE if browsing files, FALSE if dir               *
 *     Returns: The browse button widget                                *
 * ******************************************************************** */

static GtkWidget *
build_browse_button (GtkWidget * entry_box,
                     FILE_WIDGET *wp,
                     gboolean IsFile)
{
    GtkWidget *browse_button;
    GtkWidget *alignment;

    /* entry_box[alignment[browse_button]] */

    browse_button = gtk_button_new_with_label ("Browse");
    gtk_widget_set_size_request(browse_button, -1, -1);
    alignment = gtk_alignment_new(1, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(alignment), browse_button);
    gtk_box_pack_end (GTK_BOX (entry_box), alignment, FALSE, FALSE, 2);

    if (IsFile)
    {
        g_signal_connect (G_OBJECT (browse_button), "clicked",
                          G_CALLBACK (browse_files), wp);
    }
    else
    {
        g_signal_connect (G_OBJECT (browse_button), "clicked",
                          G_CALLBACK (browse_dirs), wp);
    }

    return browse_button;
}

/* **************************************************************** *
 * build_entry_box () - build a frame with an entry box and a       *
 *                      browse button inside                        *
 *      Passed:  (1) - Label for frame                              *
 *               (2) - Container into which new frame is stored     *
 *               (3) - Filname to insert into entry (if != NULL)    *
 *               (4) - Ptr to place to store the new frame          *
 *                                                                  *
 *      Returns: The GtkWidget for the entry box (for future ref)   *
 * **************************************************************** */

static GtkWidget *
build_entry_box (FILE_WIDGET *wp, GtkWidget * main_box, GtkWidget **m_frame)
{
    GtkWidget *entry_box;

    entry_box = build_entry_frame (wp->dialog_ttl, m_frame);
    wp->o_entry = build_entry_entry (entry_box, wp->fname);
    wp->browse_button = build_browse_button (entry_box, wp, TRUE);

    return wp->o_entry;
}

/* **************************************************************** *
 * on_ofile_tgl ()  - callback for checkbox toggles to              *
 *                enable/disable out/in file usage                  *
 *                                                                  *
 * Passed:  The toggle button (checkbox) widget                     *
 *          The corresponding struct ofile_widgets (as data ptr)    *
 *                                                                  *
 * Returns: Nothing                                                 *
 * **************************************************************** */

static void
on_ofile_tgle (GtkToggleButton * tbutton, struct ofile_widgets *srcf)
{
    gboolean cond;

    cond = gtk_toggle_button_get_active (tbutton);
    gtk_widget_set_sensitive (srcf->o_entry, cond);
    gtk_widget_set_sensitive (srcf->browse_button, cond);
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
set_dis_opts_cb (GtkAction *action, glbls *hbuf)
{
    GtkWidget *dialog,
              *table,
              *bin_entry, *cmd_entry,
              *w_frame, *d_frame,
              *w_spin, *d_spin,
              *CPU_toggle, *UpCase_toggle, *OS_toggle, *Zero_toggle,
              *ofile_toggle, *defs_toggle,
              *listing_box,
              *notebk,
              *entry_box, *m_frame,
              *list_radio_file, *list_radio_gtk, *list_radio_none,
              *aprbx,
              *hbx, *vbx, *frame, *lbl;       /* Generic temporary widgets */
    /*GtkWidget *hsep1;*/
    GSList * list_radio_group = NULL;
    gint result;
    int files_page, misc_page;  /* Save page numbers */

    dialog =
        gtk_dialog_new_with_buttons ("Command Line options for os9disasm",
                                     GTK_WINDOW (window),
                                     GTK_DIALOG_MODAL |
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                     GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                     NULL);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

    if ( ! optips)
    {
        optips = gtk_tooltips_new();
    }
    
    /* ********************** *
     *                        *
     * Create options widgets *
     *                        *
     * ********************** */

    /* **************************************** *
     * Create the "Files" Page entries          *
     * **************************************** */

    notebk = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                       notebk, FALSE, FALSE, 2);

    /* Create a table into which to place two *
     * entry boxes side by side               *
     */

    table = gtk_table_new( 4, 2, FALSE );

    if ((files_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebk),
                                               table,
                                               gtk_label_new("Files"))) == -1)
    {
    }
    
    /* Program file to be disassembled *
     */
    
    bin_entry = build_entry_box (prog_wdg, table, &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                                0, 1, 0, 1,
                                0, 0,
                                5, 2);

     /* Command file  */
    
    cmd_entry = build_entry_box (cmd_wdg, table, &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      1, 2, 0, 1,
                      0, 0,
                      5, 2);

     /*  output file ( -o option ) */

    entry_box = build_entry_frame ("Assembly Source File", &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      0, 1, 1, 2,
                      0, 0,
                      5, 2);

    ofile_toggle = gtk_check_button_new_with_label (
                                    "Generate .asm source file");
    gtk_tooltips_set_tip (optips, ofile_toggle,
           "Generate a source file that (should) assemble into a valid binary",
           NULL);
    gtk_box_pack_start (GTK_BOX (entry_box), ofile_toggle, FALSE, FALSE, 2);
    asmout_wdg->o_entry = build_entry_entry (entry_box,
                        asmout_wdg->fname ? asmout_wdg->fname : "output.a");
    asmout_wdg->browse_button = build_browse_button (entry_box,
                                                 asmout_wdg,
                                                 TRUE);

    g_signal_connect (G_OBJECT (ofile_toggle), "toggled",
                      G_CALLBACK (on_ofile_tgle), asmout_wdg);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ofile_toggle),
                                  write_obj);
    
    on_ofile_tgle (GTK_TOGGLE_BUTTON (ofile_toggle), asmout_wdg);
    gtk_widget_show_all (m_frame);

    /* 
     * path to defs files
     */

    entry_box = build_entry_frame ("\"Defs\" files", &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      0, 1, 2, 3,
                      0, 0,
                      5, 2);

    defs_toggle = gtk_check_button_new_with_label (
                                    "Specify (alternate) path");
    gtk_box_pack_start (GTK_BOX (entry_box), defs_toggle, FALSE, FALSE, 2);
    defs_wdg->o_entry = build_entry_entry (entry_box,
            defs_wdg->fname ? defs_wdg->fname : "/dd/defs");
    defs_wdg->browse_button = build_browse_button (entry_box,
                                                  defs_wdg,
                                                  FALSE);
    gtk_tooltips_set_tip (optips, defs_toggle,
           "Secify the path to where your standard label files are stored.",
           NULL);

    g_signal_connect (G_OBJECT (defs_toggle), "toggled",
                      G_CALLBACK (on_ofile_tgle), defs_wdg);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (defs_toggle),
                                  alt_defs);
    
    on_ofile_tgle (GTK_TOGGLE_BUTTON (defs_toggle), defs_wdg);
    gtk_widget_show_all (m_frame);
    
    /* send Listing to file ( > filename ) */

    entry_box = build_entry_frame("Formatted File Listing ", &m_frame);
    gtk_table_attach( GTK_TABLE(table), m_frame,
                      1, 2, 1, 3,
                      0, GTK_FILL,
                      5, 2);
    
    gtk_widget_show_all(m_frame);
    vbx = gtk_vbox_new (FALSE, 2);
    
    gtk_widget_set_size_request(vbx,300,-1);
    list_radio_file = gtk_radio_button_new_with_label (NULL,
                                                       "Output to file/stdout");
    list_radio_group = gtk_radio_button_get_group (
                         GTK_RADIO_BUTTON(list_radio_file));
    g_signal_connect (G_OBJECT (list_radio_file), "toggled",
                      G_CALLBACK (on_ofile_tgle), listing_wdg);

    /* set tooltip for list_radio_file label */
    
    gtk_tooltips_set_tip (optips, list_radio_file,
                          "Write the formatted listing to file\nor stdout if filename is blank.\nThis is the regular, space-separated listing",
                          NULL);
    listing_box = gtk_vbox_new (FALSE, 2);
    listing_wdg->o_entry = build_entry_entry (listing_box,
            listing_wdg->fname ? listing_wdg->fname : "output.List");
    listing_wdg->browse_button = build_browse_button (listing_box,
                                                 listing_wdg,
                                                 TRUE);
    
    list_radio_none = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file), "No output");
    gtk_tooltips_set_tip (optips, list_radio_none,
              "\"Quiet\" mode.  No listing is  generated.", NULL);
    list_radio_gtk = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file),
                        "Build for GUI");
    gtk_tooltips_set_tip (optips, list_radio_gtk,
                          "Generate listing directly into GUI window for manipulation.\nNo file is written", NULL);
    
    /* Now determine which radio button to set active */
    
    {
        GtkToggleButton * tb = GTK_TOGGLE_BUTTON(list_radio_gtk);
        
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
    
    /* Now pack them all into the vbox */
    
    gtk_box_pack_start (GTK_BOX (vbx), gtk_hseparator_new (),
                        FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_gtk, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbx), gtk_hseparator_new (),
                        FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_none, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbx), gtk_hseparator_new (),
                        FALSE, FALSE, 4);
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_file, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX(vbx), listing_box, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbx), gtk_hseparator_new (),
                        FALSE, FALSE, 4);
    gtk_container_add(GTK_CONTAINER(entry_box), vbx);
    gtk_widget_show_all (m_frame);
    
    /*gtk_widget_show_all(table);*/

    /* **************************************************** *
     * Now create the "Appearance" page  - First with a     *
     * vbox to contain all the elements                     *
     * **************************************************** */

    aprbx = gtk_vbox_new(0, 5);     /* Main vbox for appearance page */
    vbx = gtk_vbox_new(0,5);       /* Vbox for Page Setup widgets   */
    gtk_container_set_border_width(GTK_CONTAINER(vbx), 10);

    /* Begin page setup
     * First, create an hbox to set the page width/depth side by side
     */

    hbx = gtk_hbox_new(0, 2);

    /* Page Width */

    w_frame = gtk_frame_new ("Page Width");
    gtk_container_set_border_width (GTK_CONTAINER (w_frame), 5);
    w_spin = newspin1 (pgwdth);

    gtk_container_add (GTK_CONTAINER (w_frame), w_spin);
    gtk_box_pack_start(GTK_BOX(hbx), w_frame, FALSE, FALSE, 5);

    /* Page Depth */

    d_frame = gtk_frame_new ("Page Depth");
    gtk_container_set_border_width (GTK_CONTAINER (d_frame), 5);
    d_spin = newspin1 (pgdpth);

    gtk_container_add (GTK_CONTAINER (d_frame), d_spin);
    gtk_box_pack_start(GTK_BOX(hbx), d_frame, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbx), hbx, FALSE, FALSE, 5);

    /* ***** end page setup ***** */

    /* Fold to uppercase */
    
    UpCase_toggle = gtk_check_button_new_with_label ("Fold to Upper Case");
    gtk_tooltips_set_tip (optips, UpCase_toggle,
          "Labels, mnemonics, in source and listing are output in upper-case",            NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (UpCase_toggle),
                                  (gboolean) upcase);
    gtk_box_pack_start (GTK_BOX(vbx), UpCase_toggle, FALSE, FALSE, 2);

    /* Show zero offsets */

    Zero_toggle = gtk_check_button_new_with_label ("Show Zero Offsets");
    gtk_tooltips_set_tip (optips, Zero_toggle,
            "Show \"0,R\" in operand rather than \",R\"", NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Zero_toggle),
                                  (gboolean)showzeros);
    gtk_box_pack_start (GTK_BOX(vbx), Zero_toggle, FALSE, FALSE, 2);

    /*gtk_box_pack_start(GTK_BOX(vbx), gtk_hseparator_new(), FALSE, FALSE, 5);*/
    
    /* Frame to hold Appearance spinbuttons
     * Add Appearance vbx to frame, then frame to main aprbx */

    frame = gtk_frame_new(0);
    lbl = gtk_label_new(0);
    gtk_label_set_markup(GTK_LABEL(lbl), "<span face=\"Bitstream Sans Vera\"foreground=\"blue\" size=\"large\">Appearance</span>");
    gtk_frame_set_label_widget(GTK_FRAME(frame), lbl);
    gtk_frame_set_label_align (GTK_FRAME(frame), .10, .50);
    
    gtk_container_add(GTK_CONTAINER(frame), vbx);
    gtk_box_pack_start(GTK_BOX(aprbx), frame, FALSE, FALSE, 5);

    /* ******************************** *
     * Types options                    *
     * ******************************** */

    vbx = gtk_vbox_new(0,5);       /* Vbox for Types widgets   */
    gtk_container_set_border_width (GTK_CONTAINER(vbx), 10);

    /* do CPU type */
    CPU_toggle = gtk_check_button_new_with_label ("CPU = 6309");
    gtk_tooltips_set_tip (optips, CPU_toggle,
          "Disassemble 6309 instructions.\nIn normal mode, 6309-only instructions are not accepted\nand rendered as data",
                          NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (CPU_toggle),
                                  (gboolean) cputype);
    gtk_box_pack_start (GTK_BOX(vbx), CPU_toggle, FALSE, FALSE, 2);

    /* Disassemble RS-DOS code */
    
    OS_toggle = gtk_check_button_new_with_label ("RS-DOS binary");
    gtk_tooltips_set_tip (optips, OS_toggle,
		          "The file is an RS-DOS binary instead of OS9", NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (OS_toggle),
                                  (gboolean) isrsdos);
    gtk_box_pack_start (GTK_BOX(vbx), OS_toggle, FALSE, FALSE, 2);
    
    frame = gtk_frame_new(0);
    lbl = gtk_label_new(0);
    gtk_label_set_markup(GTK_LABEL(lbl), "<span face=\"Bitstream Sans Vera\"foreground=\"blue\" size=\"large\">Types</span>");
    gtk_frame_set_label_widget(GTK_FRAME(frame), lbl);
    gtk_frame_set_label_align (GTK_FRAME(frame), .10, .50);
    
    gtk_container_add(GTK_CONTAINER(frame), vbx);
    gtk_box_pack_start(GTK_BOX(aprbx), frame, FALSE, FALSE, 5);

    /* all widgets are now set up, now run dialog
     * after return, set all options according to states
     */
    
    if ((misc_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebk),
                                    aprbx,
                                    gtk_label_new("Appearance"))) == -1)
    {
    }

    gtk_widget_show_all(dialog);
    switch (result = gtk_dialog_run (GTK_DIALOG (dialog)))
    {
    case GTK_RESPONSE_OK:
    case GTK_RESPONSE_ACCEPT:
        pgdpth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d_spin));
        pgwdth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w_spin));

        if (prog_wdg->fname != NULL)
        {
            g_free (prog_wdg->fname);
        }
        
        prog_wdg->fname = g_strdup (gtk_entry_get_text (GTK_ENTRY (bin_entry)));

        if ((prog_wdg->fname) && !(strlen(prog_wdg->fname)))
        {
            g_free(prog_wdg->fname);
            prog_wdg->fname = NULL;
        }

        menu_do_dis_sensitize ();

        if (cmd_wdg->fname != NULL)
        {
            g_free (cmd_wdg->fname);
        }

        cmd_wdg->fname = g_strdup (gtk_entry_get_text (GTK_ENTRY (cmd_entry)));

        if (!strlen(cmd_wdg->fname))
        {
            g_free(cmd_wdg->fname);
            cmd_wdg->fname = NULL;
        }

        cputype = (gint)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (CPU_toggle));
        upcase = (gint)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (UpCase_toggle));
        isrsdos = (gboolean)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (OS_toggle));
        showzeros = (gboolean)
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Zero_toggle));

        if ((write_obj =
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ofile_toggle))))
        {
            if (asmout_wdg->fname != NULL)
            {
                g_free (asmout_wdg->fname);
            }
            asmout_wdg->fname =
                g_strdup (gtk_entry_get_text (GTK_ENTRY (asmout_wdg->o_entry)));
        }

        if ((alt_defs =
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (defs_toggle))))
        {
            if (defs_wdg->fname != NULL)
            {
                g_free (defs_wdg->fname);
            }
            defs_wdg->fname =
                g_strdup (gtk_entry_get_text (GTK_ENTRY (defs_wdg->o_entry)));
        }

        /* Handle listing output */
        
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list_radio_file)))
        {
            write_list = LIST_FILE;

            if(listing_wdg->fname != NULL)
            {
                g_free(listing_wdg->fname);
            }
            listing_wdg->fname =
                g_strdup (gtk_entry_get_text (GTK_ENTRY(listing_wdg->o_entry)));
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

/* ******************************************************************** *
 * font_sel_cb () - Main (top-level) callback for modifying the font of *
 *          one of the views.                                           *
 * ******************************************************************** */

static void
font_select_cb (GtkButton *btn, struct fontsel_data *w_sel)
{
    w_sel->newfont = NULL;
    w_sel->oldfont = pango_font_description_to_string (w_sel->style->font_desc);

    /* Save the font name for now - we may wish to eliminate this step
     * later */
    w_sel->newfont = (gchar *)gtk_font_button_get_font_name (
                                        GTK_FONT_BUTTON(w_sel->font_btn));
    gtk_widget_modify_font (selected_view,
                        pango_font_description_from_string (w_sel->newfont));
}

/* ******************************************************************** *
 * color_sel_cb () - Callback for when a color selection button is      *
 *          pressed.                                                    *
 * ******************************************************************** */

static void
color_select_cb (GtkButton *btn, struct fontsel_data *w_sel)
{
    gboolean is_fg;
    const gchar *btn_name = gtk_widget_get_name (GTK_WIDGET(btn));
    GdkColor colr;

    is_fg = (g_ascii_strcasecmp (btn_name, "foreground") == 0);
    gtk_color_button_get_color (GTK_COLOR_BUTTON(btn), &colr);

    if (is_fg) {
        gtk_widget_modify_text (selected_view, GTK_STATE_NORMAL, &colr);
    }
    else {
        gtk_widget_modify_base (selected_view, GTK_STATE_NORMAL, &colr);
    }
}

/* ******************************************************************** *
 * set_list_ptr() - Callback for when a Listing selection radio button  *
 *          is toggled.                                                 *
 *          If it became active, set "selected_view" to the proper      *
 *          window.  If it became inactive, do nothing.                 *
 *          Also sets the colors in the color selection buttons.        *
 * ******************************************************************** */

static void
set_list_ptr (GtkWidget *rb, struct fontsel_data *w_sel)
{
    if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(rb)))
    {
        selected_view = g_hash_table_lookup (w_sel->list_to_widg,
                                             gtk_widget_get_name (rb));
        w_sel->style = gtk_widget_get_style (selected_view);
        gtk_font_button_set_font_name (GTK_FONT_BUTTON(w_sel->font_btn),
                    pango_font_description_to_string(w_sel->style->font_desc));
        gtk_color_button_set_color (GTK_COLOR_BUTTON(w_sel->cb_bg),
                                    &(w_sel->style->base[0]));
        gtk_color_button_set_color (GTK_COLOR_BUTTON(w_sel->cb_fg),
                                    &(w_sel->style->text[0]));
    }
}

/* ******************************************************************** *
 * reset_lists() - Resets a list window to the state in which it was on *
 *      entry to the procedure.  This is called when the "Cancel"       *
 *      button is pressed in the Fonts/Colors select.                   *
 * ******************************************************************** */

static void
reset_lists (fileinf *inf)
{
    GtkStyle *style = gtk_widget_get_style (inf->tview);
    gchar * name = pango_font_description_to_string (style->font_desc);

    if ( name && g_ascii_strcasecmp (name, inf->fontname))
    {
        gtk_widget_modify_font (inf->tview,
                pango_font_description_from_string (inf->fontname));
    }

    if (inf->txtcolor && ! gdk_color_equal (inf->txtcolor, &(style->text[0])))
    {
        gtk_widget_modify_text (inf->tview, GTK_STATE_NORMAL, inf->txtcolor);
    }

    if (inf->bakcolor && ! gdk_color_equal (inf->bakcolor, &(style->base[0])))
    {
        gtk_widget_modify_base (inf->tview, GTK_STATE_NORMAL, inf->bakcolor);
    }
}

/* ******************************************************************** *
 * replace_gdkcolor () - replaces the GdkColor if it is different       *
 * ******************************************************************** */

static void
replace_gdkcolor (GdkColor **oldcolor, GdkColor *newcolor)
{
    GdkColor * delcolor = NULL;

    if (*oldcolor && ! gdk_color_equal (*oldcolor, newcolor))
    {
        delcolor = *oldcolor;
    }

    if ( ! *oldcolor ||  ! gdk_color_equal (*oldcolor, newcolor))
    {
        *oldcolor = gdk_color_copy (newcolor);
    }

    /* Does the following work ??? */

    if (delcolor)
    {
        gdk_color_free (delcolor);
    }
}

/* ******************************************************************** *
 * update_lists() - update the fileinf data passed as a parameter       *
 *      to reflect the current font and {fore,back}ground colors        *
 * ******************************************************************** */

void
update_lists (fileinf *inf)
{
    GtkStyle *style = gtk_widget_get_style (inf->tview);
    gchar *name;

    name = pango_font_description_to_string (style->font_desc);

    if (inf->fontname) {
        if (g_ascii_strcasecmp (inf->fontname, name)) {
            g_free (name);
        }
    }
    else {
        inf->fontname = "";      /* To avoid segfault */
    }

    if (g_ascii_strcasecmp (inf->fontname, name)) {
        inf->fontname = pango_font_description_to_string (style->font_desc);
    }

    replace_gdkcolor (&(inf->txtcolor), &(style->text[0]));
    replace_gdkcolor (&(inf->bakcolor), &(style->base[0]));
}

/* ******************************************************************** *
 * cbtn_add_lbl() - Finds the GtkFrame in the gtk_color_button and set  *
 *          the label to that passed as the second parameter.           *
 * ******************************************************************** */

static void
cbtn_add_lbl (GtkWidget *btn, gchar *label)
{
    while (! GTK_IS_FRAME (btn))
    {
        btn = gtk_bin_get_child (GTK_BIN(btn));
    }

    gtk_frame_set_label (GTK_FRAME(btn), label);
}

/* ******************************************************************** *
 * dlg_set_tips() - Parses the GList of Buttons in the dialog and sets  *
 *          tooltips for them.                                          *
 * ******************************************************************** */

void
dlg_set_tips (GtkWidget *btn, GtkDialog *dialog, GtkTooltips *tips)
{
    tips = gtk_tooltips_new();

    switch (gtk_dialog_get_response_for_widget (dialog, btn))
    {
        case GTK_RESPONSE_OK:
            gtk_tooltips_set_tip (tips, btn,
                    "Exit session, keeping all changes", NULL);
            break;
        default:    /* Cancel */
            gtk_tooltips_set_tip (tips, btn,
                    "End session, rejecting all changes made\nduring session.\nAll windows will revert back to\ntheir state on entry into this session", NULL);
    }
}

/* ******************************************************************** *
 * fonts_main_dialog () - Callback for main menu choice to select a     *
 *      different font and/or color for the listing, commands, or       *
 *      labels display(s).                                              *
 *      This dialog will be destroyed at the end of usage, since it     *
 *      should not be called often, surely only once in a session.      *
 * ******************************************************************** */

void
fonts_main_dialog (GtkAction *action, glbls *globals)
{
    GtkWidget *fonts_main_dialog,
              * hbox_main,
              * frame,
              * tmp_vbox,
              * tmp_hbox,
              * radiolist = NULL;
    struct fontsel_data fs_data;
    gint response;
    GList *btnlist;

    /* Build the dialog */

    fonts_main_dialog = gtk_dialog_new_with_buttons (
                            "Choose Font",
                            GTK_WINDOW(window),
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            "Cancel", GTK_RESPONSE_CANCEL,
                            "OK",     GTK_RESPONSE_OK,
                            NULL);
    btnlist = gtk_container_get_children (
                GTK_CONTAINER(GTK_DIALOG(fonts_main_dialog)->action_area));
    g_list_foreach (btnlist, (GFunc)dlg_set_tips, fonts_main_dialog);
    hbox_main = gtk_hbox_new (FALSE, 10);

    selected_view = globals->list_file.tview;
    fs_data.style = gtk_widget_get_style (selected_view);
    /* Listings radio group on left-hand side */

    /* Build a list_to_widg hash */
    fs_data.list_to_widg = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_insert (fs_data.list_to_widg, "listing",
                                               globals->list_file.tview);
    g_hash_table_insert (fs_data.list_to_widg, "cmds",
                                               globals->cmdfile.tview);
    g_hash_table_insert (fs_data.list_to_widg, "labels",
                                               globals->lblfile.tview);
    tmp_vbox = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER(tmp_vbox), 10);
    radiolist = gtk_radio_button_new_with_label (NULL, "Listings");
    gtk_widget_set_name (radiolist, "listing");
    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));
    radiolist = gtk_radio_button_new_with_label_from_widget (
                                                GTK_RADIO_BUTTON(radiolist),
                                                "Commands");
    gtk_widget_set_name (radiolist, "cmds");
    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));
    radiolist = gtk_radio_button_new_with_label_from_widget (
                                                GTK_RADIO_BUTTON(radiolist),
                                               "Labels");
    gtk_widget_set_name (radiolist, "labels");
    g_signal_connect (radiolist, "toggled", G_CALLBACK(set_list_ptr), &fs_data);
    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), GTK_WIDGET(radiolist));

    /* This may be a temporary thing... */
    /* Following is not necessary, but the list is originally set up in
     * reverse order, so we pick the last button in the list to gt the first */

    frame = gtk_frame_new ("Listings to alter");
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER(frame), 10);
    gtk_container_add (GTK_CONTAINER(frame), tmp_vbox);
    gtk_box_pack_start_defaults (GTK_BOX(hbox_main), frame);

    /* Select Font/Color buttons on right-hand side */

    /* The fonts button */
    tmp_vbox = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER(tmp_vbox), 10);
    fs_data.font_btn = gtk_font_button_new ();
    g_signal_connect (fs_data.font_btn, "font-set", G_CALLBACK(font_select_cb),
                                               &fs_data);
    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), fs_data.font_btn);

    gtk_box_pack_start_defaults (GTK_BOX(tmp_vbox), gtk_hseparator_new());

    /* The colors buttons */

    tmp_hbox = gtk_hbox_new (FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER(tmp_hbox), 10);

    /* The background button */
    fs_data.cb_bg = gtk_color_button_new ();
    g_signal_connect (fs_data.cb_bg, "color-set", G_CALLBACK(color_select_cb),
                                                  &fs_data);
    gtk_color_button_set_title (GTK_COLOR_BUTTON(fs_data.cb_bg),
                                "Background");
    gtk_widget_set_name(fs_data.cb_bg, "background");
    cbtn_add_lbl (fs_data.cb_bg, "Background");
    gtk_box_pack_start_defaults (GTK_BOX(tmp_hbox), fs_data.cb_bg);

    /* The foreground button */
    fs_data.cb_fg = gtk_color_button_new ();
    g_signal_connect (fs_data.cb_fg, "color-set", G_CALLBACK(color_select_cb),
                                                   &fs_data);
    gtk_color_button_set_title (GTK_COLOR_BUTTON(fs_data.cb_fg),
                                "Foreground");
    gtk_widget_set_name(fs_data.cb_fg, "foreground");
    cbtn_add_lbl (fs_data.cb_fg, "Foreground");
    gtk_box_pack_start_defaults (GTK_BOX(tmp_hbox), fs_data.cb_fg);

    frame = gtk_frame_new ("Select Font/Color");
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER(frame), 10);
    gtk_container_add (GTK_CONTAINER(tmp_vbox), tmp_hbox);
    gtk_container_add (GTK_CONTAINER(frame), tmp_vbox);

    gtk_box_pack_start_defaults (GTK_BOX(hbox_main), frame);

    /* Set up for the first radio button in the list-selection */

    set_list_ptr (GTK_WIDGET(g_slist_last (gtk_radio_button_get_group (
                            GTK_RADIO_BUTTON(radiolist)))->data), &fs_data);
    gtk_color_button_set_color (GTK_COLOR_BUTTON(fs_data.cb_bg),
                                &(fs_data.style->base[0]));
    gtk_color_button_set_color (GTK_COLOR_BUTTON(fs_data.cb_fg),
                                &(fs_data.style->text[0]));

    /* Now pack all into the dialog vbox */
    gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(fonts_main_dialog)->vbox),
                                 hbox_main);
    gtk_widget_show_all (hbox_main);

    response = gtk_dialog_run (GTK_DIALOG(fonts_main_dialog));

    switch (response)
    {
        case GTK_RESPONSE_OK:
            update_lists (&(globals->list_file));
            update_lists (&(globals->cmdfile));
            update_lists (&(globals->lblfile));
            break;
        default:
            reset_lists (&(globals->list_file));
            reset_lists (&(globals->cmdfile));
            reset_lists (&(globals->lblfile));
    }

    gtk_object_destroy (GTK_OBJECT(font_tips));
    font_tips = NULL;               /* Flag it uninitialized */
    gtk_widget_destroy (fonts_main_dialog);
}
