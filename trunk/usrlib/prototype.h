#ifndef USRLIB_SYSCALL_H
#define USRLIB_SYSCALL_H

void outf(err_t *error, boolean_t block, char *fmt, ...) __attribute__((format (printf,3, 4)));
uint32_t strlen(char *me);
char *err2String(int errnum);
void strncpy(char *dst, char *dst, int num);
boolean_t streq(char *a, char *b);

#endif