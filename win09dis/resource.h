/* ************************************************************* $
 * Header file for resources for Win09Dis                        $
 * $Id::                                                         $
 * ************************************************************* */

#define IDR_MENU1       101
#define ID_LSTNGNEW     102
#define ID_CMDNEW       103
#define ID_LBLNEW       104
#define ID_LSTNGOPEN    105
#define ID_CMDOPEN      106
#define ID_LBLOPEN      107
#define ID_CMDSAVE      108
#define ID_LBLSAVE      109
#define ID_CMDSAVEAS    110
#define ID_LBLSAVEAS    111
#define ID_DASMPROG     112
#define ID_DASMTOFILE   113
#define ID_AMLISTEDIT   114
#define ID_DISOPTS      115
#define ID_OPTSLOAD     116
#define ID_OPTSSAVE     117
#define ID_HLPABOUT     118

#define IDD_OPTS        201
#define IDD_ABOUT       501
#define IDD_WINDOWQUIT  502
#define IDW_OPTSFILE    521 

#define IDT_FILES      1001
#define IDT_APPEAR     1002
#define IDC_OK         1003
#define IDC_CANCEL     1004
#define IDC_APPLY      1005
#define ID_SAVEALL     1021
#define ID_SELECTEM    1022

#define IDC_ABTTXT     2001

// Buttons, etc in Set Options dialog
// IDT_* = Text windows
// IDB_* = Buttons - these are what we need to refer to in callbacks

#define ID_NOTIMPLEMENTED 3001
#define IDT_PROG       3201
#define IDT_CMD	       3202
#define IDT_OUT        3203
#define IDT_DEFS       3204
#define IDT_FILE       3205
#define IDT_PGWDTH     3206
#define IDT_PGDPTH     3207

#define IDB_PROG       3211
#define IDB_CMD        3212
#define IDB_OUT	       3213
#define IDB_DEFS       3214
#define IDB_FILE       3215

#define IDR_GUI        3221
#define IDR_QUIET      3222
#define IDR_FILE       3223

#define IDUD_PGDPTH    3231
#define IDUD_PGWDTH    3232
#define IDCB_UPCASE    3233
#define IDCB_ZEROS     3234
#define IDCB_CPU       3235
#define IDCB_RSDOS     3236

// Listing popup
#define IDR_LSTPOPMNU  3301
#define ID_LSTP_SRCH   3302
#define ID_LSTP_OPEN   3303
#define ID_LBLREN      3304
#define ID_DEFBNDS     3305
#define ID_SETAMODE    3306

// Label popup
#define IDR_LBLPOPMNU  3311
#define ID_LBLP_SRCH   3312
#define ID_LBLP_OPEN   3313
#define ID_LBLP_SAVE   3314
#define ID_LBLP_SAVEAS 3315
#define ID_LBLP_INSRT  3316
#define ID_LBLP_DEL    3317
#define ID_LBLP_PROP   3318

// Dialogs for Listing and Label popup menus

#define IDD_RENAME_LBL 3401
#define IDRN_LBLNAME   3402
#define IDRN_LBLADDR   3403
#define IDRN_COMBO     3404

#define IDD_BNDSDEFINE 3411
#define IDDB_BT        3412
#define IDDB_AM        3413
#define IDDB_RNGE      3414
#define IDDB_OFST      3415

// Set Addressing Mode Dialog
#define IDD_SETAMODE   3421
#define IDSAM_CLASS    3422
#define IDSAM_AM       3423
#define IDSAM_RNGE     3424
#define IDSAM_OFST     3425

// Label Edit Dialog
#define IDD_LBLEDIT    3431
#define ID_LBLED_NAME  3432
#define ID_LBLED_ADDR  3433
#define ID_LBLED_CLASS 3434

// Listing/Labels Search Dialog
#define IDD_SRCHDLG    3441
#define IDD_LBL_SRCHDLG 3442
#define ID_SRCH_RBBOX  3443
#define ID_RB_ADDR     3444
#define ID_RB_LABEL    3445
#define ID_RB_OPERAND  3446
#define ID_CBEXACT     3447
#define ID_SRCH_TXT    3448
#define ID_SRCHBKWD    3449
#define ID_SRCHFWD     3450

// Amode Edit Dialog
#define IDD_AMODEEDIT  3461
#define ID_AM_ADD      3462
#define ID_AM_EDCLEAR  3463
#define ID_AM_EDAPPLY  3464
#define ID_AM_EDOPEN   3465
#define ID_AM_EDDEL    3466
#define ID_AM_EDCLOSE  3467
// Delete an amode dialog
#define IDD_AM_DEL     3471
#define ID_AM_DEL_COMBO 3472
