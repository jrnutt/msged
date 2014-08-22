/* PUBLIC DOMAIN */

/*
 * this structure defines a LINE 
 */
 
typedef struct _line {
	char *text;             /* pointer to actual line text */
	int len;                /* length of text */
	int alloced;            /* amount actually allocated */
	unsigned int block:1;	/* this is in a block */
	unsigned int hide:1;	/* this is a hidden line */
	unsigned int quote:1;   /* this is a quoted line */
	unsigned int hard:1;    /* this line has a hard return */
	int column;		/* if a block, starting column */
	struct _line *prev;     /* previous line in BUFFER */
	struct _line *next;     /* next line in BUFFER */
} LINE;
