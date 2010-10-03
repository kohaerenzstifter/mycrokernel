#ifndef USRLIB_PROTOTYPE_H
#define USRLIB_PROTOTYPE_H

void outf(err_t *error, boolean_t block, char *fmt, ...) __attribute__((format (printf,3, 4)));
uint32_t strlen(char *me);
char *err2String(err_t errnum);
void strncpy(char *dst, char *src, int num);
boolean_t streq(char *a, char *b);

#endif