#include "../include/types.h"
#include "../include/const.h"
#include "../include/always.h"

#include "syscall.h"


void memcpy(char *dst, char *src, uint32_t num)
{
	uint32_t bytes_pending = num;
	uint32_t *src_dword = NULL;
	uint32_t *dst_dword = NULL;
	uint16_t *src_word = NULL;
	uint16_t *dst_word = NULL;
	uint8_t *src_byte = NULL;
	uint8_t *dst_byte = NULL;

	src_dword = (uint32_t *) src;
	dst_dword = (uint32_t *) dst;

	while (bytes_pending > 3) {
		*dst_dword = *src_dword;
		src_dword++; dst_dword++;	
		bytes_pending -= 4;
	}
	src_word = (uint16_t *) src_dword;
	dst_word = (uint16_t *) dst_word;
	while (bytes_pending > 1) {
		*dst_word = *src_word;
		src_word++; dst_word++;
		bytes_pending -= 2;
	}
	src_byte = (uint8_t *) src_word;
	dst_byte = (uint8_t *) dst_word;
	*dst_byte = *src_byte;
}

uint32_t stringLength(char *me)
{
  char *cur = me;
  uint32_t result = 0;
  while (*cur != '\0') {
    cur++;
    result++;
  }
  return result;
}

void strncpy(char *dst, char *src, int num)
{
  int i;
  for (i = 0; i < num; i++) {
    dst[i] = src[i];
    if (src[i] == '\0') {
      break;
    }
  }
  dst[num - 1] = '\0';
}

static void hex2String(int32_t what, char *buf, uint32_t buflen)
{
  uint32_t i = 0;
  uint32_t idx = 0;
  uint8_t shift = 7;
  uint32_t mask = 0xf0000000;
  uint32_t val = 0;
  boolean_t first = FALSE;

  buf[0] = '0';
  buf[1] = 'x';

  for (i = 2; i < buflen; i++) {
    buf[i] = '\0';
  } 
  idx = 2;
  
  while (mask > 0) {
    if (((val = (mask & what)) == 0)&&(mask != 0xf)&&!first) {
      goto carryOn;
    }
    first = TRUE;
    val >>= (shift * 4);
    if ((val >= 0)&&(val <= 9)) {
      buf[idx] = '0' + val;
    } else {
      buf[idx] = 'A' + val - 10;
    }
    idx++;
carryOn:
    mask >>= 4;
    shift--;
  }
}

static void uint2String(uint32_t what, char *buf, uint32_t buflen)
{
  uint32_t val = 0;
  uint32_t divider = 1000000000;
  uint32_t idx = 0;
  uint32_t i = 0;
  boolean_t first = FALSE;
  
  for (i = 0; i < buflen; i++) {
    buf[i] = '\0';
  } 

  while (divider > 0) {
    if (((val = (what / divider)) == 0)&&(divider != 1)&&!first) {
      goto carryOn;
    }
    first = TRUE;
    buf[idx] = '0' + val;
    idx++;
carryOn:
    what -= (val * divider);
    divider /= 10;
  }
}

static void int2String(int32_t what, char *buf, uint32_t buflen)
{
  uint32_t idx = 0;
  uint32_t i = 0;

  for (i = 0; i < buflen; i++) {
    buf[i] = '\0';
  }

  if (what < 0) {
    buf[0] = '-';
    idx++;
    what = -what;
  }
  
  uint2String(what, &(buf[idx]), (buflen - 1));
}

void outf(err_t *error, boolean_t block, char *fmt, ...)
{
  char buf[15];
  int idx = 0;
  char *cur = fmt;
  char msg[MAX_MSG_SIZE];
  char next;
  int32_t sval = 0;
  uint32_t usval = 0;
  uint32_t len = 0;
  uint32_t i = 0;
  va_list vl;

  va_start(vl, fmt);
  
  for(;;) {
    if ((*cur != '%')||(((next = *(cur + 1)) != 'd')&&(next != 'u')&&(next != 'x'))) {
      msg[idx] = *cur;
      if (*cur == '\0') {
	terror(call_syscall_send_by_feature(FEATURE_TTY,
	  msg, idx + 1, block, error))
	goto finish;
      }
      idx++;
      cur++;
      if (idx >= (MAX_MSG_SIZE - 1)) {
	if (*cur == '\0') {
	  continue;
	}
	msg[idx] = '\0';
	terror(call_syscall_send_by_feature(FEATURE_TTY,
	  msg, idx + 1, TRUE, error))
	idx = 0;
	continue;
      }
      continue;
    }

    if (next == 'd') {
      sval = va_arg(vl, int32_t);
      int2String(sval, buf, sizeof(buf));
    } else if (next == 'u') {
      usval = va_arg(vl, uint32_t);
      uint2String(usval, buf, sizeof(buf));
    } else if (next == 'x') {
      sval = va_arg(vl, int32_t);
      hex2String(sval, buf, sizeof(buf));
    }
    len = stringLength(buf);

    if (len >= (sizeof(msg) - idx)) {
      msg[idx] = '\0';
      terror(call_syscall_send_by_feature(FEATURE_TTY,
	msg, idx + 1, TRUE, error))
      idx = 0;
    }

    for (i = 0; i < len; i++) {
      msg[idx] = buf[i];
      idx++;
    }

    cur += 2;
  }

finish:
  va_end(vl);
}

char *error_strings[] = {
  ERR_MSG0,
  ERR_MSG1,
  ERR_MSG2,
  ERR_MSG3,
  ERR_MSG4,
  ERR_MSG5,
  ERR_MSG6,
  ERR_MSG7,
  ERR_MSG8,
  ERR_MSG9,
  ERR_MSG10,
  ERR_MSG11,
  ERR_MSG12,
  ERR_MSG13
};

char *err2String(int errnum)
{
  char *result = NULL;
  if ((errnum > OK)||((-errnum) >=
    sizeof(error_strings)/sizeof(error_strings[0]))) {
    result = "impossible error";
    goto finish;
  }
  result = error_strings[(-errnum)];
finish:
  return result;
}

boolean_t streq(char *a, char *b)
{
  int i;
  boolean_t result = FALSE;
  for(i = 0;; i++) {
    if (a[i] != b[i]) {
      break;
    }
    if (a[i] == '\0') {
      result = TRUE;
      break;
    }
  }
  return result;
}
