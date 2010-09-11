#ifndef INCLUDE_ALWAYS_H
#define INCLUDE_ALWAYS_H

typedef void *va_list; 

#define va_start(l,t) \
  { \
    l = &t + 1; \
  }

#define va_arg(l, t) (*((t *) ((l = l + sizeof(t)) - sizeof(t)))) 
#define va_end(l) (l = (void *) 0) 
/* defined in C99, pretty useless, but make them happy ;) */ 
#define va_copy(d, s) (d = s)

#endif