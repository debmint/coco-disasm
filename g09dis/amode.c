/* **************************************** *
 * amode.c handles editing functions for    *
 *   editing addressing modes  a breakout   *
 *   from dasmedit.c                        *
 * **************************************** */

/* $Id$ */

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include "g09dis.h"
/*#define GOT_DTBL    */            /*  We loaded the data in dasmedit.c */
#include "../os9disasm/dtble.h"

extern GList *amode_list;

/* ******************************************** *
 * amode_init() - populate the addressing modes *
 *    list with the standard label classes      *
 * NOTE: a g_strdup() of the constant string    *
 *     is used so it can be deleted if desired  *
 * ******************************************** */

void
amode_init()
{
    gchar *amode_dflt[] = {"$ - Hexadecimal constant",
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
    gchar **cur =amode_dflt;

    while (*cur) {
        amode_list = g_list_append (amode_list, g_strdup(*cur));
        ++cur;
    }
}

static void
pack_label (GtkBox *box, gchar *text)
{
    GtkWidget *label;

    label = gtk_label_new (text);
    gtk_box_pack_start (box, label, FALSE, FALSE, 2);
}

static void
pack_hsep (GtkBox *box)
{
    GtkWidget *hsep;

    hsep = gtk_hseparator_new();
    gtk_box_pack_start (box, hsep, FALSE, FALSE, 2);
}

/* ************************************************************ *
 * callback for click on "add" button to add an addressing mode *
 * ************************************************************ */

static void
add_amode_man (GtkButton *button, GtkWidget *entry)
{
    GString *mstr;
    mstr = g_string_new (g_strdup(gtk_entry_get_text (GTK_ENTRY(entry))));
    gchar *message = NULL; 
    
    if (!amode_list)    /* never been initialized? */
    {
        amode_init();
    }

    if ( mstr->len == 0)
    {
        message = "A NULL String was Processed";
    }
    else {
        if (!g_ascii_isalpha(mstr->str[0]))
        {
            message = "The first character MUST be an ALPHA";
        }
    }

    if (message)
    {
        GtkWidget *mbox;
        
        mbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                       message);
        gtk_dialog_run(GTK_DIALOG(mbox));
        gtk_widget_destroy(mbox);
        g_string_free (mstr, TRUE);
        return;          /* do nothing in this case */
    }

    amode_list = g_list_append (amode_list, mstr->str);
    g_string_free (mstr, FALSE);
}

/* callback to clear the amode entry window */
static void
clear_entry( GtkButton *button, GtkWidget *entry)
{
    gtk_entry_set_text (GTK_ENTRY(entry), "");
}

/* ******************************* *
 * amode_add_from_string ()        *
 *   Add an amode to amode_list    *
 *     provided by a string        *
 *     (usually loaded from a file *
 * ******************************* */
void
amode_add_from_string (gchar *ptr)
{
    gchar *str;

    /* Work with a copy since ptr is temporary, plus we're going to modify
     * it */
    
    str = g_strdup (ptr);
    
    /* Get rid of any leading and trailing whitespaces */
    
    g_strstrip (str);

    /* In case amode_list has never been initialized */
    
    if (!amode_list)
    {
        amode_init();
    }

    /* Try to see if this entry has been already been inserted *
     * Note that this is very unreliable.  It's basically a    *
     * strcmp(), and a difference of a single character will   *
     * cause a match to fail                                   */
    
    if (!g_list_find (amode_list, str))
    {
        g_print("Adding : %s\n",str);/*DEBUG*/

        amode_list = g_list_append (amode_list, str);
    }
}

/* *************************************** *
 * amodes_from_file() - get list of amodes *
 *                      from a file        *
 * *************************************** */

static void
amodes_from_file (GtkButton *button, glbls *fdat)
{
    selectfile_open (fdat, "Label Descriptor", TRUE);

    if (fdat->filename_to_return)
    {
        FILE *infile;

        if ((infile = fopen(fdat->filename_to_return, "rb")))
        {
            char buf[500];

            while ( fgets (buf, sizeof(buf), infile)) {
                gchar *pt = buf;
                
                g_strstrip (buf);

                if ( *pt == '*' )
                {
                    ++pt;
                    if ( *pt == '\\')
                    {
                        ++pt;

                        amode_add_from_string (pt);
                    }
                }
            }
            
            fclose (infile);
        }
        
        /* last thing... */
        free_filename_to_return(&(fdat->filename_to_return));
    }
}

static void
delete_combo_changed (GtkComboBox *cbox, GtkWidget *call_win)
{
    gint index;

    if ( (index=gtk_combo_box_get_active (cbox)) != -1)
    {
        GtkWidget *dialog;
        gchar *element = g_list_nth_data (amode_list, index);

        dialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL |
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_QUESTION,
                                         GTK_BUTTONS_YES_NO,
                                         "Delete:\n     %s?", element);

        switch ( gtk_dialog_run (GTK_DIALOG(dialog)) ) {
            case GTK_RESPONSE_YES:
                g_list_remove (amode_list, element);
                g_free (element);
                break;
            default:
                break;
        }

        gtk_widget_destroy (dialog);

        /* Now, send a response to close the parent window.
         *
         * We had thought to keep that window open to recursively
         * delete entries, but it seemed that when an entry was
         * removed from the combobox, a "changed" signal must have
         * been emitted, causing this window to re-occur.
         *
         * Also, in the above case, if the deleted element was not
         * the last one, this recurring window asked for the same string
         * as before, and if "yes" was clicked, it caused the program
         * to hang.  Didn't ever figure out why..
         */
        
        gtk_dialog_response (GTK_DIALOG (call_win), GTK_RESPONSE_CLOSE);
    }
}

/* *************************** *
 * delete_amode_cb ()          *
 *  dialog to choose an amode  *
 *  entry to delete            *
 * *************************** */

static void
delete_amode_cb (GtkButton *button, gpointer zilch)
{
    GtkWidget *dialog,
              *combo;

    dialog = gtk_dialog_new_with_buttons (" Amode Entry Delete",
                                          GTK_WINDOW(window),
                                          GTK_DIALOG_MODAL |
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                          NULL);

    combo = build_label_selector(NULL, FALSE);
    g_signal_connect (combo, "changed", G_CALLBACK(delete_combo_changed),
                      dialog);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), combo,
                        FALSE, FALSE, 5);

    gtk_widget_show_all (dialog);

    /* Nothing returned, all processing done within dialog */
    
    gtk_dialog_run (GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* ********************************** *
 * amode_list_edit_cb() - add/delete  *
 * or otherwise modify the addressing *
 * mode list                          *
 * ********************************** */

void
amode_list_edit_cb(GtkAction * action, glbls *fdat)
{
    GtkWidget *dialog,
              *table,
              *button,
              *alignment,
              *man_entry;

    dialog = gtk_dialog_new_with_buttons ("Modify Addressing Mode List",
                                          GTK_WINDOW (window),
                                          /*GTK_DIALOG_MODAL|*/
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                          NULL);

    /* ********************* *
     * Manually add an entry *
     * ********************* */

    /* Set up containers */
    alignment = bounds_aligned_frame (GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                      "Add an addressing mode");
    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_set_size_request ( table, 280, -1);
    gtk_container_add (GTK_CONTAINER (alignment), table);

    /* Now insert tools */
    man_entry = gtk_entry_new ();
    gtk_table_attach_defaults (GTK_TABLE (table), man_entry, 0, 2, 0, 1);

    button = gtk_button_new_from_stock (GTK_STOCK_CLEAR);
    gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);
    g_signal_connect (button, "clicked", G_CALLBACK (clear_entry), man_entry);

    button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    gtk_table_attach_defaults (GTK_TABLE (table), button, 1, 2, 1, 2);
    g_signal_connect (button, "clicked", G_CALLBACK(add_amode_man), man_entry);
    
    /* ********************* *
     * Add entries from file *
     * ********************* */

    /* Set up containers */
    alignment = bounds_aligned_frame (GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                      "Add entries from file");
    button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
    g_signal_connect (button, "clicked", G_CALLBACK(amodes_from_file), &O9Dis);
    gtk_container_add (GTK_CONTAINER (alignment), button);

    /* *************** *
     * Delete an entry *
     * *************** */

    alignment = bounds_aligned_frame (GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                      "Delete an amode entry");
    button = gtk_button_new_with_label("Delete Entry");
    g_signal_connect (button, "clicked", G_CALLBACK(delete_amode_cb), NULL);
    gtk_container_add (GTK_CONTAINER (alignment), button);
    
    gtk_widget_show_all(dialog);

    /* simply run dialog - all events are handled from with callbacks,
     * so no processing is needed on return
     */
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

char
*get_addressing_mode(char *str, char *opcode, char *mnemonic, char *oprand)
{
    int opcode_val;
    struct lkuptbl *T;
    gint mymode;
    register int sz;

    if( !strncmp(opcode,"10",2) )
    {
        T = Pre10;
        sscanf( &opcode[2], "%02x", &opcode_val);
        sz = sizeof(Pre10)/sizeof(Pre10[0]);
    }
    else {
        if( !strncmp(opcode,"11",2) )
        {
            T = Pre11;
            sscanf( &opcode[2], "%02x", &opcode_val);
            sz = sizeof(Pre11)/sizeof(Pre11[0]);
        }
        else {
            T = Byte1;
            sscanf( opcode, "%02x", &opcode_val);
            sz = sizeof(Byte1)/sizeof(Byte1[0]);
        }
    }

    while( ((T->cod & 0xff) != opcode_val) ) {
        if( --sz == 0 )
        {
            g_print("\nEnd of table and no opcode matched!\n");
            return NULL;
        }
        ++T;
    }

    mymode = T->amode;

    if( strncmp( mnemonic, T->mnem, strlen(mnemonic) ) )
    {
        GtkWidget *dialog;
          
        dialog = gtk_message_dialog_new( GTK_WINDOW(window),
                                  GTK_DIALOG_MODAL |
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "The standard mnemonic %s $(%x)\ndoes not match %s in the file",T->mnem, T->cod, mnemonic);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

      /* ********************************************************** *
       * All indexed modes are chunked together as XIDX (x indexed) *
       * the actual register needs to be identified                 *
       * ********************************************************** */
      if( mymode == AM_XIDX )
      {
          gchar *reg;

          if ( !(reg = strchr(oprand, ',')))
          {
              g_print("No comma found in operand!!!\n");
              /* if no comma, we have a PROBLEM! */
              return NULL;
          }
          
          ++reg;

          /* take care of ",-R" and ",--R" */
          
          while ( *reg == '-' ) {
              ++reg;
          }

          switch (g_ascii_toupper(*reg) ) {
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
                  g_print("Lookup for Index Register FAILED ! \n");
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

/* *************************************** *
 * on_adr_mode_response () - Callback for  *
 *     when "OK" button is clicked in      *
 *     addressing mode setup               *
 * Passed:  dialog box containing widgets  *
 *          the response -> button clicked *
 *          the structure containing ptrs  *
 *              to the entry widgets       *
 * *************************************** */

static void
on_adr_mode_response( GtkDialog *dialog, gint resp,
                      struct adr_widgets *data)
{
    GString *line;

    switch (resp) {
        case GTK_RESPONSE_ACCEPT:
            line = g_string_new(data->cmd_mode);
            g_string_prepend (line, "> ");
            g_string_append_printf(line, " %1.1s ",
                                   gtk_entry_get_text(GTK_ENTRY (GTK_BIN(
                                               data->label_combo)->child)));
            /* offset stuff goes here */

            g_string_append(line,
                            gtk_entry_get_text(GTK_ENTRY(
                                               data->address_entry)));
            g_string_append(line, "\n");

            gtk_text_buffer_insert_at_cursor (
                    O9Dis.cmdfile.tbuf, line->str, -1);
                                    
            doc_set_modified( &O9Dis.cmdfile, TRUE);
            g_string_free(line, TRUE);
            break;
        default:
            break;
    }

    g_free (data);
    gtk_widget_destroy (GTK_WIDGET (dialog));
}
/* Try to guess a usable Addressing Mode from the mnemonic */

gchar *
guess_addr_mode(gchar *mnem)
{
    gchar *guess;
    
    if ( !g_ascii_strcasecmp (mnem, "equ") )
    {
        guess = "X";
    }
    else {
        if ( ! g_ascii_strcasecmp (mnem, "rmb"))
        {
            guess = "D";
        }
        else {
            guess = "L";
        }
    }
    return guess;
}

/* ******************************************* *
 * callback function to handle addressing mode *
 * ******************************************* */

void
adr_mode_cb(GtkAction * action, glbls *fdat)
{
    GtkWidget *dialog;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    struct adr_widgets *cb_data;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(
                                                 (fdat->list_file).tview));
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        GtkWidget *align;

        gchar *addr, *opcod, *pbyt, *mnem,
              *oprand;
        
        char adr_mode[4];

        if (!(cb_data = g_malloc(sizeof (struct adr_widgets))))
        {
            /* print error */
            g_print("adr_mode_cb() :   Error, cannot malloc memory!\n");
            return;
        }
        
        cb_data->cmd_mode = adr_mode;
        
        gtk_tree_model_get (model, &iter, LST_ADR, &addr,
                                          LST_OPCO, &opcod,
                                          LST_PBYT, &pbyt,
                                          LST_MNEM, &mnem,
                                          LST_OPER, &oprand, -1);

        /* *************************************** *
         * Get addressing mode for current command *
         * *************************************** */
        
        if ( !(cb_data->cmd_mode =
                    get_addressing_mode(adr_mode, opcod, mnem, oprand)) )
        {
            GtkWidget *errdialog;
            gchar *errmsg = NULL;

            errmsg = g_strconcat("No label class found for this instruction\n",
                                 "This is probably a non-classed line\n",
                                 "If a class SHOULD be present,\n"
                                 "please report a bug",
                                 NULL);

            errdialog = gtk_message_dialog_new (GTK_WINDOW(window),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_CLOSE,
                                                errmsg);
            g_free(errmsg);
            gtk_dialog_run(GTK_DIALOG(errdialog));
            gtk_widget_destroy(errdialog);
        }
        
        dialog = gtk_dialog_new_with_buttons("Set Addressing Mode",
                                             GTK_WINDOW(window),
                                             /*GTK_DIALOG_MODAL |*/
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_REJECT,
                                             GTK_STOCK_OK,
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
       
        g_signal_connect(dialog, "response",
                G_CALLBACK(on_adr_mode_response), cb_data);
        gtk_container_set_border_width (GTK_CONTAINER(dialog),15);
        
        pack_label (GTK_BOX(GTK_DIALOG(dialog)->vbox), cb_data->cmd_mode);
        
        /* Build selection for desired Addressing Mode */
        
        pack_hsep (GTK_BOX(GTK_DIALOG(dialog)->vbox));
        align = bounds_aligned_frame (GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                      "Addressing Mode");
        
        cb_data->label_combo = build_label_selector(NULL, TRUE);

        gtk_entry_set_text(GTK_ENTRY(GTK_BIN(cb_data->label_combo)->child),
                           guess_addr_mode(mnem));
        gtk_container_add(GTK_CONTAINER(align), cb_data->label_combo);

        /* *********************** *
         * Select address or range *
         * *********************** */
        
        align = bounds_aligned_frame (GTK_BOX(GTK_DIALOG(dialog)->vbox),
                                      "Select address or address range");
        
        cb_data->address_entry = gtk_entry_new();
        gtk_container_add(GTK_CONTAINER(align), cb_data->address_entry);
        gtk_entry_set_text (GTK_ENTRY(cb_data->address_entry), addr);
        
        /*gtk_widget_show_all (GTK_DIALOG(dialog)->vbox);
        gtk_dialog_run(GTK_DIALOG(dialog));*/
        gtk_widget_show_all (dialog);

        g_free(addr); g_free(opcod); g_free(pbyt);
        g_free(oprand); g_free(mnem);
        
        /*gtk_widget_destroy(dialog);*/
    }
}

