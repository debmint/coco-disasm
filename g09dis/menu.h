/* ******************************* *
 * menu.h - header file for menu.c *
 * ******************************* */

/* Id:$ */

/* ****************************
 * menu_items[]
 * ItemFactory list for menu creation
 *
 * NOTE:  This will probably be soon replaced
 * by Gui Toolkit
*/

static GtkItemFactoryEntry menu_items[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"},
    {"/File/_New", "<CTRL>N", get_filename, 100,
     "<StockItem>", GTK_STOCK_NEW},
    {"/File/_Open", "<CTRL>O", NULL, 0, "<Branch>", GTK_STOCK_OPEN},
    {"/File/Open/tear1", NULL, NULL, 0, "<Tearoff>"},
    {"/File/Open/_Listing", "<control>L", get_filename, 1,
     "<StockItem>", GTK_STOCK_OPEN},
    {"/File/Open/_Command file", "<CTRL>C", get_filename, 2,
     "<StockItem>", GTK_STOCK_OPEN},
    {"/File/Open/La_bel File", "<shift><control>B", get_filename, 3,
     "<StockItem>", GTK_STOCK_OPEN},
    {"/File/_Save", "<CTRL>S", get_filename, 100,
     "<StockItem>", GTK_STOCK_SAVE},
    {"/File/Save _As", NULL, NULL, 0, "<Item>"},
    {"/File/sep1", NULL, NULL, 0, "<Separator>"},
    {"/File/_Quit", "<CTRL>Q", gtk_main_quit, 0,
     "<StockItem>", GTK_STOCK_QUIT},

    {"/_View", NULL, NULL, 0, "<Branch>"},
    {"/View/tear", NULL, NULL, 0, "<Tearoff>"},

    {"/_Tools", NULL, NULL, 0, "<Branch>"},
    {"/Tools/tear", NULL, NULL, 0, "<Tearoff>"},
    {"/_Options", NULL, NULL, 0, "<Branch>"},
    {"/Options/tearo", NULL, NULL, 0, "<Tearoff>"},
    {"/Options/Disassembler Options", NULL, NULL, 0, "<Branch>"},
    {"/Options/Disassembler Options/teart", NULL, NULL, 0, "<Tearoff>"},
    {"/Options/Disassembler Options/Command File", NULL, NULL, 0,
     "<StockItem>", GTK_STOCK_EXECUTE},
    {"/Options/Disassembler Options/tsep1", NULL, NULL, 0, "<Separator>"},
    {"/Options/Disassembler Options/Page Size", NULL,
     pg_size_cb, 1, "<Item>"},
    {"/Options/Disassembler Options/CPU 6309", NULL,
     tool_toggle, 1, "<ToggleItem>"},
    {"/Options/Disassembler Options/UpperCase", NULL,
     tool_toggle, 2, "<ToggleItem>"},

    {"/Options/sep", NULL, NULL, 0, "<Separator>"},
    {"/Options/Rad1", NULL, print_selected, 1, "<RadioItem>"},
    {"/Options/Rad2", NULL, print_selected, 2, "/Options/Rad1"},
    {"/Options/Rad3", NULL, print_selected, 3, "/Options/Rad1"},
    {"/_Help", NULL, NULL, 0, "<LastBranch>"},
    {"/_Help/About", NULL, NULL, 0, "<Item>"},
};
