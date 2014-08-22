/* PUBLIC DOMAIN */

#define __handle
#define handle_malloc(n)	malloc(n)
#define handle_calloc(n)	calloc((n),1)
#define handle_realloc(h,n)	realloc((h),(n))
#define handle_free(h)		free(h)
#define handle_strdup(h)	strdup(h)

#undef handle_ishandle
#define handle_ishandle(h)	0
