#ifndef _CTYPE_H
#define _CTYPE_H

/* Function prototypes for character classification */

int isalpha(int c);
int isdigit(int c);
int isspace(int c);
int isupper(int c);
int islower(int c);
int isalnum(int c);
int iscntrl(int c);
int ispunct(int c);
int isprint(int c);
int isgraph(int c);
int tolower(int c);
int toupper(int c);

#endif /* _CTYPE_H */