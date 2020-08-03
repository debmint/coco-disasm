/* ******************************************************************** *
 *                                                                      $
 * odisopts.c - setup options to pass to assembler                      $
 *                                                                      $
 * NOTE:  This newer version will create the widgets within a single    $
 *        GtkGrid rather than stacking hboxes within the dialog->vbox   $
 *                                                                      $
 * $Id::                                                                $
 * ******************************************************************** */


#include <string.h>
#include <glib/gprintf.h>
#include "g09dis.h"

/* ************************************************ *
 * Create a new spin button                         *
 *    Passed: default value for button              *
 *    Returns: the spin button widget ptr           *
 * ************************************************ */

GtkWidget *
newspin1(int defval)
{
    GtkWidget *spin;

    spin = gtk_spin_button_new_with_range((gdouble)40, (gdouble)200, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), (gdouble)defval);

    return spin;
}

/* ******************************************** *
 * Get a filename using selectfile_open ()      *
 *                                              *
 *   Passed: (1) - The button pressed           *
 *           (2) - The associated FILE_WIDGET   *
 * ******************************************** */

static void
browse_files(GtkButton *button, FILE_WIDGET *wp)
{
    if (wp->is_read)
    {
        selectfile_open(&O9Dis, wp->dialog_ttl, TRUE,
                        gtk_entry_get_text (GTK_ENTRY(wp->o_entry)));
    }
    else
    {
        selectfile_save(&O9Dis,
                        gtk_entry_get_text(GTK_ENTRY(wp->o_entry)),
                        wp->dialog_ttl);
    }


    if ((O9Dis.filename_to_return && strlen(O9Dis.filename_to_return)))
    {
        gtk_entry_set_text (GTK_ENTRY(wp->o_entry), O9Dis.filename_to_return);
        
        if (LastPath)
        {
            g_free (LastPath);
        }

        LastPath = g_strdup(O9Dis.filename_to_return);
    }

    free_filename_to_return(&(O9Dis.filename_to_return));
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

    /* VBox to hold entry widgets */
    entry_vbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
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
build_entry_entry(GtkWidget *parent, gchar *filnam)
{
    GtkWidget *entry;

    entry = gtk_entry_new ();
    gtk_widget_set_size_request( entry, 300, -1 );
    gtk_box_pack_start(GTK_BOX(parent), GTK_WIDGET(entry),
                    FALSE, FALSE, 3);

    if (filnam != NULL)
    {
        gtk_entry_set_text(GTK_ENTRY(entry), filnam);
    }

    return entry;
}

/* ******************************************************************** *
 * Create a browse button inside an alignment to the right, placed      *
 * inside the provided entry_box, and connect a callback for "clicked". *
 *     Passed: (1) - The container into which to pack new button        *
 *             (2) - The entry widget to pass to callback as data.      *
 *             (3) - TRUE if browsing files, FALSE if dir               *
 *     Returns: The browse button widget                                *
 * ******************************************************************** */

static GtkWidget *
build_browse_button(GtkWidget *entry_box,
                  FILE_WIDGET *wp,
                     gboolean  IsFile)
{
    GtkWidget *browse_button;

    browse_button = gtk_button_new_with_label ("Browse");
    gtk_widget_set_size_request(browse_button, -1, -1);
    gtk_style_context_add_class(gtk_widget_get_style_context(browse_button), "browsebtn");
    gtk_box_pack_end (GTK_BOX (entry_box), browse_button, FALSE, FALSE, 2);

    g_signal_connect(G_OBJECT(browse_button), "clicked",
                          G_CALLBACK(IsFile ? browse_files : browse_dirs), wp);

    return browse_button;
}

/* **************************************************************** *
 * Build a frame with an entry box and a browse button inside       *
 *                                                                  * 
 *      Passed:  (1) - Label for frame                              *
 *               (2) - Container into which new frame is stored     *
 *               (3) - Filname to insert into entry (if != NULL)    *
 *               (4) - Ptr to place to store the new frame          *
 *                                                                  *
 *      Returns: The GtkWidget for the entry box (for future ref)   *
 * **************************************************************** */

static GtkWidget *
build_entry_box (FILE_WIDGET *wp, GtkWidget *main_box, GtkWidget **m_frame)
{
    GtkWidget *entry_box;

    entry_box = build_entry_frame (wp->dialog_ttl, m_frame);
    wp->o_entry = build_entry_entry (entry_box, wp->fname);
    wp->browse_button = build_browse_button (entry_box, wp, TRUE);

    return wp->o_entry;
}

/* ******************************************************************** *
 * Callback for checkbox toggles to enable/disable out/in file usage    *
 *                                                                      *
 * Passed:  The toggle button (checkbox) widget                         *
 *          The corresponding struct ofile_widgets (as data ptr)        *
 * ******************************************************************** */

static void
on_ofile_tgle (GtkToggleButton * tbutton, struct ofile_widgets *srcf)
{
    gboolean cond;

    cond = gtk_toggle_button_get_active (tbutton);
    gtk_widget_set_sensitive (srcf->o_entry, cond);
    gtk_widget_set_sensitive (srcf->browse_button, cond);
}

/* ********************************************************** *
 *           Dialog to Set Disassembler Options               *
 *                                                            *
 *                 Called from GtkAction                      *
 * Passed:  GtkAction Widget, the global hbuf pointer         *
 * ********************************************************** */

void
set_dis_opts_cb (GtkMenuItem *mi, glbls *hbuf)
{
    GtkWidget *dialog,
              *grid,
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
    gint result;
    int files_page, misc_page;  /* Save page numbers */

    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Command Line options for os9disasm");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(w_main));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    // Add buttons
    lbl = gtk_dialog_add_button(GTK_DIALOG(dialog), "CANCEL", GTK_RESPONSE_CANCEL);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "cancel");
    lbl = gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_ACCEPT);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "ok");

    notebk = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(
                        dialog))), notebk, FALSE, FALSE, 5);

    /* Create a grid into which to place two *
     * entry boxes side by side              */

    grid = gtk_grid_new();

    if ((files_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebk),
                                               grid,
                                               gtk_label_new("Files"))) == -1)
    {
    }
    
    /* Program file to be disassembled */
    
    bin_entry = build_entry_box(prog_wdg, grid, &m_frame);
    gtk_frame_set_label_align(GTK_FRAME(m_frame), 0.10, 0.50);
    gtk_grid_attach(GTK_GRID(grid), m_frame, 0, 0, 1, 1);

     /* Command file  */
    
    cmd_entry = build_entry_box(cmd_wdg, grid, &m_frame);
    gtk_frame_set_label_align(GTK_FRAME(m_frame), 0.10, 0.50);
    gtk_grid_attach(GTK_GRID(grid), m_frame, 1, 0, 1, 1);

     /*  output file ( -o option ) */

    entry_box = build_entry_frame("Assembly Source File", &m_frame);
    gtk_frame_set_label_align(GTK_FRAME(m_frame), 0.10, 0.50);
    gtk_grid_attach(GTK_GRID(grid), m_frame, 1, 1, 1, 1);

    ofile_toggle = gtk_check_button_new_with_label (
                                    "Generate .asm source file");
    gtk_widget_set_tooltip_text(ofile_toggle,
           "Generate a source file that (should) assemble into a valid binary");
    gtk_box_pack_start (GTK_BOX (entry_box), ofile_toggle, FALSE, FALSE, 2);
    asmout_wdg->o_entry = build_entry_entry(entry_box,
                        asmout_wdg->fname ? asmout_wdg->fname : "output.a");
    asmout_wdg->browse_button = build_browse_button(entry_box,
                                     asmout_wdg, TRUE);

    g_signal_connect(G_OBJECT(ofile_toggle), "toggled",
                    G_CALLBACK(on_ofile_tgle), asmout_wdg);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (ofile_toggle),
                                  write_obj);
    
    on_ofile_tgle(GTK_TOGGLE_BUTTON(ofile_toggle), asmout_wdg);
    gtk_widget_show_all(m_frame);

    /*  path to defs files */

    entry_box = build_entry_frame("\"Defs\" files", &m_frame);
    gtk_frame_set_label_align(GTK_FRAME(m_frame), 0.05, 0.50);
    gtk_grid_attach(GTK_GRID(grid), m_frame, 1, 2, 1, 1);

    defs_toggle = gtk_check_button_new_with_label (
                                    "Specify (alternate) path");
    gtk_box_pack_start (GTK_BOX (entry_box), defs_toggle, FALSE, FALSE, 2);
    defs_wdg->o_entry = build_entry_entry (entry_box,
            defs_wdg->fname ? defs_wdg->fname : "/dd/defs");
    defs_wdg->browse_button = build_browse_button (entry_box, defs_wdg, FALSE);
    gtk_widget_set_tooltip_text(defs_toggle,
           "Secify the path to where your standard label files are stored.");

    g_signal_connect (G_OBJECT (defs_toggle), "toggled",
                      G_CALLBACK(on_ofile_tgle), defs_wdg);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (defs_toggle), alt_defs);
    
    on_ofile_tgle (GTK_TOGGLE_BUTTON (defs_toggle), defs_wdg);
    gtk_widget_show_all (m_frame);
    
    /* send Listing to file ( > filename ) */

    entry_box = build_entry_frame("Formatted File Listing ", &m_frame);
    gtk_frame_set_label_align(GTK_FRAME(m_frame), 0.10, 0.50);
    gtk_grid_attach(GTK_GRID(grid), m_frame, 0, 1, 1, 2);
    
    gtk_widget_show_all(m_frame);
    vbx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    
    //gtk_widget_set_size_request(vbx,300,-1);
    list_radio_file = gtk_radio_button_new_with_label (NULL,
                                                       "Output to file/stdout");
    g_signal_connect (G_OBJECT (list_radio_file), "toggled",
                      G_CALLBACK (on_ofile_tgle), listing_wdg);

    /* set tooltip for list_radio_file label */
    
    gtk_widget_set_tooltip_text(list_radio_file,
                          "Write the formatted listing to file\nor stdout if filename is blank.\nThis is the regular, space-separated listing");
    listing_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    listing_wdg->o_entry = build_entry_entry (listing_box,
            listing_wdg->fname ? listing_wdg->fname : "output.List");
    listing_wdg->browse_button = build_browse_button (listing_box,
                                                 listing_wdg,
                                                 TRUE);
    
    list_radio_none = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file), "No output");
    gtk_widget_set_tooltip_text(list_radio_none,
              "\"Quiet\" mode.  No listing is  generated.");
    list_radio_gtk = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON(list_radio_file),
                        "Build for GUI");
    gtk_widget_set_tooltip_text(list_radio_gtk,
                          "Generate listing directly into GUI window for manipulation.\nNo file is written");
    
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
                sysfailed ("Internal error: invalid listing output!!!", FALSE);
        }
        gtk_toggle_button_set_active(tb, TRUE);
    }
    
    /* Now pack them all into the vbox */
    
    /*gtk_box_pack_start (GTK_BOX (vbx), gtk_separator_new (
                GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4);*/
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_gtk, FALSE, FALSE, 2);
    /*gtk_box_pack_start (GTK_BOX (vbx), gtk_separator_new(
                GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4); */
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_none, FALSE, FALSE, 2);
    /*gtk_box_pack_start (GTK_BOX (vbx), gtk_separator_new(
                GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4); */
    gtk_box_pack_start (GTK_BOX(vbx), list_radio_file, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX(vbx), listing_box, FALSE, FALSE, 2);
    /*gtk_box_pack_start(GTK_BOX (vbx), gtk_separator_new(
                GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4); */
    gtk_container_add(GTK_CONTAINER(entry_box), vbx);
    gtk_widget_show_all (m_frame);
    
    /*gtk_widget_show_all(table);*/

    /* **************************************************** *
     * Now create the "Appearance" page  - First with a     *
     * vbox to contain all the elements                     *
     * **************************************************** */

    aprbx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // Main vbox for appearance page */
    vbx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);   // for Page Setup widgets
    gtk_container_set_border_width(GTK_CONTAINER(vbx), 10);

    /* Begin page setup
     * First, create an hbox to set the page width/depth side by side */

    hbx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

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
    gtk_widget_set_tooltip_text(UpCase_toggle,
          "Labels, mnemonics, in source and listing are output in upper-case");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (UpCase_toggle),
                                  (gboolean) upcase);
    gtk_box_pack_start (GTK_BOX(vbx), UpCase_toggle, FALSE, FALSE, 2);

    /* Show zero offsets */

    Zero_toggle = gtk_check_button_new_with_label ("Show Zero Offsets");
    gtk_widget_set_tooltip_text(Zero_toggle,
            "Show \"0,R\" in operand rather than \",R\"");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Zero_toggle),
                                  (gboolean)showzeros);
    gtk_box_pack_start (GTK_BOX(vbx), Zero_toggle, FALSE, FALSE, 2);

    /*gtk_box_pack_start(GTK_BOX(vbx), gtk_hseparator_new(), FALSE, FALSE, 5);*/
    
    /* Frame to hold Appearance spinbuttons
     * Add Appearance vbx to frame, then frame to main aprbx */

    frame = gtk_frame_new("Appearance");
    lbl = gtk_label_new(0);
    gtk_frame_set_label_align (GTK_FRAME(frame), .10, .50);
    
    gtk_container_add(GTK_CONTAINER(frame), vbx);
    gtk_box_pack_start(GTK_BOX(aprbx), frame, FALSE, FALSE, 5);

    /* Types options */

    vbx = gtk_box_new(GTK_ORIENTATION_VERTICAL,5); /* Vbox for Types widgets */
    gtk_container_set_border_width (GTK_CONTAINER(vbx), 10);

    /* CPU type */
    CPU_toggle = gtk_check_button_new_with_label ("CPU = 6309");
    gtk_widget_set_tooltip_text(CPU_toggle,
          "Disassemble 6309 instructions.\nIn normal mode, 6309-only instructions are not accepted\nand rendered as data");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (CPU_toggle),
                                  (gboolean) cputype);
    gtk_box_pack_start (GTK_BOX(vbx), CPU_toggle, FALSE, FALSE, 2);

    /* Disassemble RS-DOS code */
    
    OS_toggle = gtk_check_button_new_with_label("RS-DOS binary");
    gtk_widget_set_tooltip_text(OS_toggle,
		          "The file is an RS-DOS binary instead of OS9");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (OS_toggle),
                                  (gboolean) isrsdos);
    gtk_box_pack_start (GTK_BOX(vbx), OS_toggle, FALSE, FALSE, 2);
    
    frame = gtk_frame_new("Types");
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

    switch (result = gtk_dialog_run(GTK_DIALOG(dialog)))
    {
    case GTK_RESPONSE_OK:
    case GTK_RESPONSE_ACCEPT:
        pgdpth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d_spin));
        pgwdth = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (w_spin));

        if (prog_wdg->fname != NULL)
        {
            g_free (prog_wdg->fname);
        }
        
        prog_wdg->fname = g_strdup(gtk_entry_get_text(GTK_ENTRY(bin_entry)));

        if ((prog_wdg->fname) && !(strlen(prog_wdg->fname)))
        {
            g_free(prog_wdg->fname);
            prog_wdg->fname = NULL;
        }

        menu_do_dis_sensitize(hbuf);

        if (cmd_wdg->fname != NULL)
        {
            g_free(cmd_wdg->fname);
        }

        cmd_wdg->fname = g_strdup(gtk_entry_get_text(GTK_ENTRY(cmd_entry)));

        if (!strlen(cmd_wdg->fname))
        {
            g_free(cmd_wdg->fname);
            cmd_wdg->fname = NULL;
        }

        cputype = (gint)
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(CPU_toggle));
        upcase = (gint)
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(UpCase_toggle));
        isrsdos = (gboolean)
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(OS_toggle));
        showzeros = (gboolean)
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Zero_toggle));

        if ((write_obj =
             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ofile_toggle))))
        {
            if (asmout_wdg->fname != NULL)
            {
                g_free (asmout_wdg->fname);
            }
            asmout_wdg->fname =
                g_strdup(gtk_entry_get_text(GTK_ENTRY(asmout_wdg->o_entry)));
        }

        if ((alt_defs =
             gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(defs_toggle))))
        {
            if (defs_wdg->fname != NULL)
            {
                g_free(defs_wdg->fname);
            }
            defs_wdg->fname =
                g_strdup(gtk_entry_get_text(GTK_ENTRY(defs_wdg->o_entry)));
        }

        /* Handle listing output */
        
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list_radio_file)))
        {
            write_list = LIST_FILE;

            if(listing_wdg->fname != NULL)
            {
                g_free(listing_wdg->fname);
            }
            listing_wdg->fname =
                g_strdup(gtk_entry_get_text(GTK_ENTRY(listing_wdg->o_entry)));
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

    gtk_widget_destroy(dialog);
}
