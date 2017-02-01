/* cmdfile.c */
void do_cmd_file(void);
int apndcmnt(char *lpos);
char *cmntsetup(char *cpos, char *clas, int *adrs);
char *cmdsplit(char *dest, char *src);
void getrange(char *pt, int *lo, int *hi, int usize, int allowopen);
void tellme(char *pt);
/* dprint.c */
void adjpscmd(void);
void tabinit(void);
void PrintLine(char *pfmt, struct printbuf *pb, char class, int cmdlow, int cmdhi);
void RsOrg(void);
void RsEnd(void);
void ROFPsect(struct rof_hdr *rptr);
void OS9Modline(void);
void coco_wrt_end(void);
void WrtEmod(void);
void WrtEnds(void);
void ROFDataPrint(void);
void OS9DataPrint(void);
void ListData(struct nlist *me, int upadr, char class);
void WrtEquates(int stdflg);
/* lbltree.c */
struct nlist *ListRoot(char symbol);
struct databndaries *ClasHere(struct databndaries *bp, int adrs);
int LblCalc(char *dst, int adr, int amod);
void PrintLbl(char *dest, char clas, int adr, struct nlist *dl);
struct nlist *addlbl(int loc, char C);
struct nlist *FindLbl(struct nlist *nl, int loc);
/* os9disasm.c */
void do_opt(char *c);
int main(int argc, char **argv);
/* progdis.c */
void MovASC(int nb, char class);
void NsrtBnds(struct databndaries *bp);
void DoPrt(struct nlist *nl);
void rsdoshdr(void);
void os9hdr(void);
void progdis(void);
/* rof.c */
void rofhdr(void);
char rof_class(int typ);
void rof_addlbl(int adrs, struct rof_extrn *ref);
struct rof_extrn *find_extrn(struct rof_extrn *xtrn, int adrs);
struct rof_extrn *rof_lblref(int *value);
int rof_datasize(char class);
void ListInitROF(struct nlist *nl, int mycount, int notdp, char class);
void rof_ascii(char *cmdline);
/* util_dis.c */
int o9_fgetword(FILE *fp);
int o9_int(char *o9num);
char *skipblank(char *p);
int strpos(char *s, char c);
void printbounds(struct databndaries *bnd);
void listbounds(void);
void nerrexit(char *l);
void listlbls(void);
