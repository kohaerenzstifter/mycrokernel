#include "../include/types.h"

uint32_t strlen(char *me)
{
  char *cur = me;
  uint32_t result = 0;
  while (*cur != '\0') {
    cur++;
    result++;
  }
  return result;
}