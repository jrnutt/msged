/**
 *
 * date.h
 *
 * parse various string date formats into a unix style timestamp
 * header file
 *
 * jim nutt
 * 31 aug 1989

	PUBLIC DOMAIN
 *
**/

#include "pascal.h"

#ifndef DATE_H
#define DATE_H

time_t _pascal parsedate(char *ds);
char * _pascal atime(time_t now);
char * _pascal mtime(time_t now);
char * _pascal qtime(time_t now);

#endif
