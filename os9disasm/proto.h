/* cmdfile.c */
void do_cmd_file(void);
int cmderr(void);
int optincmd(char *lpos);
char *cmdsplit(char *dest, char *src);
void cmdamode(char *pt);
int DoMode(char *lpos);
void boundsline(char *mypos);
char *setoffset(char *p, struct ofsetree *oft);
void setupbounds(char *lpos);
void bndoverlap(void);
void bdinsert(struct databndaries *bb);
void getrange(char *pt, int *lo, int *hi, int bndtyp);
void tellme(char *pt);
char *movxnum(char *dst, char *src);
char *movdigit(char *dst, char *src);
int endofcmd(char *pp);
void nerrexit(char *l);
/* dprint.c */
void tabinit(void);
void PrintLine(char *pfmt, struct printbuf *pb);
void UpString(char *s);
void PrintFormatted(char *pfmt, struct printbuf *pb);
void StartPage(void);
void BlankLine(void);
void NonBoundsLbl(void);
void OS9Modline(void);
void WrtEmod(void);
void OS9DataPrint(void);
void ListData(struct nlist *me, int upadr);
void WrtEquates(int stdflg);
void TellLabels(struct nlist *me, int flg);
/* lbltree.c */
struct nlist *ListRoot(char symbol);
void dataget(struct databndaries *bp, char c, int k);
struct databndaries *bGoBegin(struct databndaries *pt);
struct databndaries *ClasHere(struct databndaries *bp, int adrs);
int LblCalc(char *dst, int adr, int amod);
void PrintLbl(char *dest, char clas, int adr, struct nlist *dl);
void movchr(char *dst, unsigned char ch);
struct nlist *addlbl(int loc, char C);
struct nlist *FindLbl(struct nlist *nl, int loc);
/* os9disasm.c */
void do_opt(char *c);
int main(int argc, char **argv);
/* progdis.c */
void progdis(void);
void NsrtBnds(struct databndaries *bp);
void MovBytes(struct databndaries *db);
void MovASC(int nb);
void AddDelims(char *dest, char *src);
struct lkuptbl *IsCmd(char *p);
int GetCmd(void);
int TxIdx(void);
int WrtOfst(int adr, char clas, char *dst1, char *dst2, char *RgN, int bytlen);
void DoPrt(struct nlist *nl);
void listlbls(void);
void os9hdr(void);
/* util_dis.c */
int o9_int(char *o9num);
char *skipblank(char *p);
int strpos(char *s, char c);
void PBFcp(char *dst, char *fmt, unsigned char *src, int sz);
void PBFcat(char *dst, char *fmt, unsigned char *src, int sz);
void *zalloc(size_t size);
