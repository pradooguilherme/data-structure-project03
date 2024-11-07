#define MAXKEYS 4
#define MINKEYS MAXKEYS/2
#define NIL (-1)
#define NOKEY '@'
#define NO 0
#define YES 1
typedef struct {
short keycount; // number of keys in page
char key[MAXKEYS]; // the actual keys
short child[MAXKEYS+1]; // ptrs to rrns of descendants
} BTPAGE;
#define PAGESIZE sizeof(btpage)
extern short root; // rrn of root page
extern int btfd; // file descriptor of btree file
extern int infd; // file descriptor of input file
/* prototypes */
btclose ();
btopen ();
btread (short rrn, BTPAGE *page_ptr);
btwrite (short rrn, BTPAGE *page_ptr);
create_root (char key, short left, short right);
short create_tree();
short getpage ();
short getroot ();
insert (short rrn, char key, short *promo_r_child, char *promo_key);
ins_in_page (char key,short r_child, BTPAGE *p_page);
pageinit (BTPAGE *p_page);
putroot(short root);
search_node (char key, BTPAGE *p_page, short *pos);
split(char key, short r_child, BTPAGE *p_oldpage, char *promo_key, short *promo_r_child, BTPAGE *p_newpage);
