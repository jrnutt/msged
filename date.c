/**
 *
 * date.c
 *
 * parse various string date formats into a unix style timestamp
 *
 * jim nutt
 * 31 aug 1989
 * released into the PUBLIC DOMAIN 30 Jul 1990 by jim nutt
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "msged.h"
#include "date.h"

#define valid_date(timestamp) \
	   !((timestamp->tm_wday > 6)  || (timestamp->tm_wday < 0) || \
	 (timestamp->tm_mon > 11)  || (timestamp->tm_mon < 0)  || \
	 (timestamp->tm_mday > 31) || (timestamp->tm_mday < 0) || \
	 (timestamp->tm_year > 99) || (timestamp->tm_year < 0) || \
	 (timestamp->tm_hour > 23) || (timestamp->tm_hour < 0) || \
	 (timestamp->tm_min > 59)  || (timestamp->tm_min < 0)  || \
	 (timestamp->tm_sec > 59)  || (timestamp->tm_sec < 0))

/**OS2**/	/* Removed redundant #define of strcmpl */

char * pascal attrib_line(MSG *m, char *format);

static char *month[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *day[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

time_t pascal parsedate(char *ds)

{
	char work[80];
	char *s;
	int t;
	struct tm timestamp;

	memset(&timestamp,0, sizeof timestamp);
	strcpy(work,ds);

	if (strchr(ds,'-') != NULL) {   /* quickbbs style date */
		s = strtok(work,"-");
		timestamp.tm_mon = atoi(s) - 1;
		s = strtok(NULL,"-");
		timestamp.tm_mday = atoi(s);
		s = strtok(NULL," ");
		timestamp.tm_year = atoi(s);
		s = strtok(NULL,":");
		while (isspace(*s)) s++;
		timestamp.tm_hour = atoi(s);
		s = strtok(NULL," ");
		timestamp.tm_min = atoi(s);
	}
	else {				/* fido style date */
		s = strtok(work, " ");
		if ((t = atoi(s)) == 0) { /* a usenet date */
			s = strtok(NULL," ");
			t = atoi(s);
		}
		timestamp.tm_mday = t;
		s = strtok(NULL, " ");
		for (t = 0; t < 12; t++)
			if (strcmpl(s, month[t]) == 0) break;
		if (t==12) t=1; 										/*WRA*/
		timestamp.tm_mon = t;
		s = strtok(NULL, " ");
		timestamp.tm_year = atoi(s);
		s = strtok(NULL,":");
		while (isspace(*s)) s++;
		timestamp.tm_hour = atoi(s);
		s = strtok(NULL,": \0");
		timestamp.tm_min = atoi(s);
		s = strtok(NULL," ");
		if (s != NULL)
			timestamp.tm_sec = atoi(s);
	}
	return mktime(&timestamp);
}

char * pascal atime(time_t now)

{
	static char atime_buffer[40];
	struct tm *timestamp;

	timestamp = localtime(&now);

	if (timestamp == NULL)
		return("invalid date");

	sprintf(atime_buffer, "%s %s %02d 19%02d  %02d:%02d:%02d",
		day[timestamp->tm_wday], month[timestamp->tm_mon],
		timestamp->tm_mday, timestamp->tm_year,
		timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec);
	return(atime_buffer);
}

char * pascal mtime(time_t now)

{
	static char mtime_buffer[21];
	struct tm *timestamp;

	timestamp = localtime(&now);

	if (timestamp == NULL)
		return("invalid date");

	sprintf(mtime_buffer, "%02d %s %02d  %02d:%02d:%02d",
		timestamp->tm_mday, month[timestamp->tm_mon],
		timestamp->tm_year, timestamp->tm_hour,
		timestamp->tm_min, timestamp->tm_sec);
	return(mtime_buffer);
}

char * pascal qtime(time_t now)

{
	static char qtime_buffer[20];
	struct tm *timestamp;

	timestamp = localtime(&now);

	if (timestamp == NULL)
		return("invalid date");

	sprintf(qtime_buffer, "%s %02d %02d:%02d",
		month[timestamp->tm_mon], timestamp->tm_mday,
		timestamp->tm_hour, timestamp->tm_min);
	return(qtime_buffer);
}

char * pascal attrib_line(MSG *m, char *format)

{
	char work[128];
	char *t = work;
	struct tm *timestamp;

	if (format == NULL)
		return(NULL);

	memset(work,0,sizeof work);
	timestamp = localtime(&(m->timestamp));
	if (timestamp == NULL)
		return(NULL);
	while (*format) {
		if (*format == '%') {
			switch (tolower(*++format)) {
				case '%' :
					*t = *format;
				default  :
					break;
				case 't' :
					strcpy(t,m->isto);
					break;
				case 'f' :
					strcpy(t,m->isfrom);
					break;
				case 'a' :
					strcpy(t,show_address(m->from));
					break;
				case 'w' :
					strcpy(t,day[timestamp->tm_wday]);
					break;
				case 'd' :
					sprintf(t,"%02d",timestamp->tm_mday);
					break;
				case 'y' :
					sprintf(t,"%02d",timestamp->tm_year);
					break;
				case 'm' :
					strcpy(t,month[timestamp->tm_mon]);
					break;
				case 'h' :
					sprintf(t,"%02d:%02d",timestamp->tm_hour,timestamp->tm_min);
					break;
			}
			t = work + strlen(work);
			format++;
		}
		else if (*format == '\\') {
			if (*++format == 'n')
				*t++ = '\n';
			format++;
		}
		else
			*t++ = *format++;
	}
	return strdup(work);
}
