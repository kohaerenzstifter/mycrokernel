#include "../include/types.h"
#include "../include/const.h"
#include "../usrlib/syscall.h"
#include "prototype.h"
#include "external.h"

static uint32_t shiftCount = 0;

#define BUFFER_SIZE (((NO_LINES * NO_COLUMNS) < MAX_MSG_SIZE) ? ((NO_LINES * NO_COLUMNS) - 1) : -1)
#define do_putcharacter(c) \
  if (!tty) { \
    putcharacter(c); \
  } else { \
    tty_putcharacter(c); \
  }

static boolean_t echo = TRUE;
static boolean_t screen_echoed[BUFFER_SIZE] = {FALSE};
static char snd_buffer[BUFFER_SIZE + 1] = {'\0'};
static char rcv_buffer[MAX_MSG_SIZE] = {'\0'};
static boolean_t input_dirty = FALSE;
static uint32_t screen_idx = 0;

typedef void (*keyfunc_t)(uint32_t no_args, void *address);

typedef struct _keymap {
  keyfunc_t func;
  uint32_t no_args;
  void *args;
} keymap_t;

static void puthex(uint32_t what, boolean_t tty)
{
  uint32_t val = 0;
  uint8_t shift = 7;
  uint32_t mask = 0xf0000000;
  boolean_t first = FALSE;

  while (mask > 0) {
    if (((val = (mask & what)) == 0)&&(mask != 0xf)&&!first) {
      goto carryOn;
    }
    first = TRUE;
    val >>= (shift * 4);
    if ((val >= 0)&&(val <= 9)) {
      do_putcharacter('0' + val);
    } else {
      do_putcharacter('A' + val - 10);
    }
carryOn:
    mask >>= 4;
    shift--;
  }
}
    
static void putstring(char *what, boolean_t tty)
{
  while (*what != '\0') {
    do_putcharacter(*what);
    what++;
  }
}

static void putunsint(uint32_t what, boolean_t tty)
{
  uint32_t val = 0;
  uint32_t divider = 1000000000;
  boolean_t first = FALSE;

  while (divider > 0) {
    if (((val = (what / divider)) == 0)&&(divider != 1)&&!first) {
      goto carryOn;
    }
    first = TRUE;
    puthex(val, tty);
carryOn:
    what -= (val * divider);
    divider /= 10;
  }
}

static void cls()
{
  uint32_t i = 0;

  cursor = 0;
  for (i = 0; i < NO_LINES; i++) {
    putcharacter(LF);
  }
  cursor = 0;
}

static void read_unsint(uint32_t no_args, void *address)
{
  putunsint((uint32_t) address, FALSE);
}

static void flush()
{
  screen_idx = 0;
  snd_buffer[0] = '\0';
}

static void enter_pressed(uint32_t no_args, void *address)
{
  putcharacter(LF);
  call_syscall_send_by_feature(FEATURE_CMD, snd_buffer,
    strlen(snd_buffer) + 1, FALSE, NULL);
  flush();
}

static void escape_pressed(uint32_t no_args, void *address)
{
  echo = !echo;
  tty_putcharacter(LF);
  if (echo) {
    putstring("echo now on", TRUE);
  } else {
    putstring("echo now off", TRUE);
  }
}

static void noop(uint32_t no_args, void *address)
{
}

static void printline(uint32_t no_args, void *address)
{
  tty_putcharacter(LF);
  putstring("scancode in line ", TRUE);
  putunsint((uint32_t) address, TRUE);
  putstring(" unhandled", TRUE);
  return;
}

static void read_char(uint32_t no_args, void *address)
{
  if (screen_idx < BUFFER_SIZE) {
    if (echo) {
      putcharacter((uint32_t) address);
      screen_echoed[screen_idx] = TRUE;
    } else {
      screen_echoed[screen_idx] = FALSE;
    }

    snd_buffer[screen_idx] = (char) address;
    screen_idx++;
    snd_buffer[screen_idx] = '\0';
  }
}

static void shift_released(uint32_t no_args, void *address)
{
  shiftCount--;
}

static void shift_pressed(uint32_t no_args, void *address)
{
  shiftCount++;
}

static void backspace_pressed(uint32_t no_args, void *address)
{
  if (screen_idx <= 0) {
    goto finish;
  }

  screen_idx--;
  snd_buffer[screen_idx] = '\0';

  if (screen_echoed[screen_idx]) {
    backspace();
    goto finish;
  }
  
  while ((screen_idx > 0)&&(!screen_echoed[screen_idx - 1])) {
    screen_idx--;
    snd_buffer[screen_idx] = '\0';
  }

finish:
  return;
}

static void process_message(char *buffer, uint32_t bytes)
{
  if (snd_buffer[0] != '\0') {
    input_dirty = TRUE;
    putcharacter(LF);
  }
  buffer[bytes - 1] = '\0';
  putstring(buffer, FALSE);
  putcharacter(LF);
}

static void alt_pressed(uint32_t no_args, void *address)
{
  char *message = "I am a sudden message";
  tty_putcharacter(LF);
  putstring("faking sudden message", TRUE);
  process_message(message, strlen(message) + 1);
}

static keymap_t km[183][2] =
{
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&escape_pressed, 0, NULL}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 1}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 2}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 3}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 4}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 5}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 6}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 7}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 8}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 9}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 0}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&backspace_pressed, 0, NULL}, {&backspace_pressed, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'q'}, {&read_char, 1, (void *) 'Q'}},
  {{&read_char, 1, (void *) 'w'}, {&read_char, 1, (void *) 'W'}},
  {{&read_char, 1, (void *) 'e'}, {&read_char, 1, (void *) 'E'}},
  {{&read_char, 1, (void *) 'r'}, {&read_char, 1, (void *) 'R'}},
  {{&read_char, 1, (void *) 't'}, {&read_char, 1, (void *) 'T'}},
  {{&read_char, 1, (void *) 'y'}, {&read_char, 1, (void *) 'Y'}},
  {{&read_char, 1, (void *) 'u'}, {&read_char, 1, (void *) 'U'}},
  {{&read_char, 1, (void *) 'i'}, {&read_char, 1, (void *) 'I'}},
  {{&read_char, 1, (void *) 'o'}, {&read_char, 1, (void *) 'O'}},
  {{&read_char, 1, (void *) 'p'}, {&read_char, 1, (void *) 'P'}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&enter_pressed, 0, (void *) NULL}, {&enter_pressed, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'a'}, {&read_char, 1, (void *) 'A'}},
  {{&read_char, 1, (void *) 's'}, {&read_char, 1, (void *) 'S'}},
  {{&read_char, 1, (void *) 'd'}, {&read_char, 1, (void *) 'D'}},
  {{&read_char, 1, (void *) 'f'}, {&read_char, 1, (void *) 'F'}},
  {{&read_char, 1, (void *) 'g'}, {&read_char, 1, (void *) 'G'}},
  {{&read_char, 1, (void *) 'h'}, {&read_char, 1, (void *) 'H'}},
  {{&read_char, 1, (void *) 'j'}, {&read_char, 1, (void *) 'J'}},
  {{&read_char, 1, (void *) 'k'}, {&read_char, 1, (void *) 'K'}},
  {{&read_char, 1, (void *) 'l'}, {&read_char, 1, (void *) 'L'}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'z'}, {&read_char, 1, (void *) 'Z'}},
  {{&read_char, 1, (void *) 'x'}, {&read_char, 1, (void *) 'X'}},
  {{&read_char, 1, (void *) 'c'}, {&read_char, 1, (void *) 'C'}},
  {{&read_char, 1, (void *) 'v'}, {&read_char, 1, (void *) 'V'}},
  {{&read_char, 1, (void *) 'b'}, {&read_char, 1, (void *) 'B'}},
  {{&read_char, 1, (void *) 'n'}, {&read_char, 1, (void *) 'N'}},
  {{&read_char, 1, (void *) 'm'}, {&read_char, 1, (void *) 'M'}},
  {{&printline, 1, (void *) __LINE__}, {&printline, 1, (void *) 44}},
  {{&printline, 1, (void *) __LINE__}, {&printline, 1, (void *) 44}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&alt_pressed, 1, (void *) NULL}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) ' '}, {&read_char, 1, (void *) ' '}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&noop, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_released, 0, NULL}, {&shift_released, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&printline, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_released, 0, NULL}, {&shift_released, 0, NULL}}
};

static void restore_input()
{
  int i;
  for (i = 0; snd_buffer[i] != '\0'; i++) {
    if (screen_echoed[i]) {
      putcharacter(snd_buffer[i]);
    }
  }
  input_dirty = FALSE;
}

static void process_scancode(uint32_t scancode)
{
  keyfunc_t func = NULL;
  uint8_t shifted = (shiftCount == 0) ? 0 : 1;
  
  if (input_dirty) {
    restore_input();
    goto finish;
  }

  if (scancode >= sizeof(km)/sizeof(km[0])) {
    tty_putcharacter(LF);
    putstring("scancode ", TRUE);
    putunsint(scancode, TRUE);
    putstring(" out of range!", TRUE);
    goto finish;
  }
  func = km[scancode][shifted].func;
  if (func == NULL) {
    tty_putcharacter(LF);
    putstring("function pointer for scancode ", TRUE);
    putunsint(scancode, TRUE);
    putstring(" is NULL!", TRUE);
    goto finish;
  }
  func(km[scancode][shifted].no_args, km[scancode][shifted].args);
finish:
  return;
}

static void main_loop(err_t *error)
{
  uint32_t scancode = 0;
  uint32_t bytes = 0;

  for(;;) {
    terror(bytes = call_syscall_receive(ANYPROC, rcv_buffer,
      (screen_idx == 0) ? sizeof(rcv_buffer) : 0, error))
    if (*error > 0) {
      if (*error & 2) {
	terror(scancode = call_syscall_inb(0x60, error))
	process_scancode(scancode);
      }
      *error = OK;
    } else {
      bytes = (bytes < sizeof(rcv_buffer)) ? bytes : (sizeof(rcv_buffer) - 1);
      process_message(rcv_buffer, bytes);
    }
  }
finish:
  return;
}

int main()
{
  err_t err = OK;
  err_t *error = &err;

  cls();
  tty_putcharacter(LF);

  terror(call_syscall_claim_port(0x60, error))
  terror(call_syscall_claim_port(0x3D4, error))
  terror(call_syscall_claim_port(0x3D5, error))

  move_cursor();

  terror(call_syscall_request_irq(1, error))
  terror(call_syscall_set_feature(FEATURE_TTY, error))
  terror(main_loop(error))
finish:
  if (hasFailed(error)) {
    putstring("ferror: ", FALSE); putstring(err2String(*error), FALSE); putcharacter(LF);
  }
  putstring("will exit", FALSE); putcharacter(LF);
  call_syscall_exit();
  return 0;
}
