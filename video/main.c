#include "../include/types.h"
#include "../include/const.h"
#include "../usrlib/syscall.h"
#include "prototype.h"
#include "external.h"

static uint32_t shiftCount = 0;
static char cmdBuffer[MAX_MSG_SIZE] = "";
static char *bufferPtr = &cmdBuffer[0];

typedef void (*keyfunc_t)(uint32_t no_args, void *address);

typedef struct _keymap {
  keyfunc_t func;
  uint32_t no_args;
  void *args;
} keymap_t;

static void puthex(uint32_t what)
{
  uint32_t val = 0;
  uint8_t shift = 7;
  uint32_t mask = 0xf0000000;

  while (mask > 0) {
    if (((val = (mask & what)) == 0)&&(mask != 0xf)) {
      goto carryOn;
    }
    val >>= (shift * 4);
    if ((val >= 0)&&(val <= 9)) {
      putcharacter('0' + val);
    } else {
      putcharacter('A' + val - 10);
    }
carryOn:
    mask >>= 4;
    shift--;
  }
}
    
static void putstring(char *what)
{
  while (*what != '\0') {
    putcharacter(*what);
    what++;
  }
}

static void putunsint(uint32_t what)
{
  uint32_t val = 0;
  uint32_t divider = 1000000000;
  while (divider > 0) {
    if (((val = (what / divider)) == 0)&&(divider != 1)) {
      goto carryOn;
    }
    puthex(val);
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

static void read_hex(uint32_t no_args, void *address)
{
  puthex((uint32_t) address);
}

static void read_unsint(uint32_t no_args, void *address)
{
  putunsint((uint32_t) address);
}

static void enter_pressed(uint32_t no_args, void *address)
{
  putcharacter(LF);
  
  if (bufferPtr != NULL) {
    *bufferPtr = '\0';
  }
  
  call_syscall_send_by_feature(FEATURE_CMD, cmdBuffer, strlen(cmdBuffer) + 1, FALSE, NULL);
  bufferPtr = &cmdBuffer[0];
  *bufferPtr = '\0';
}

static void buffer_character(char me)
{
  if (bufferPtr == NULL) {
    goto finish;
  }
  *bufferPtr = me;
  if (bufferPtr == &cmdBuffer[sizeof(cmdBuffer) - 2]) {
    *(bufferPtr + 1) = '\0';
    bufferPtr = NULL;
    goto finish;
  }
  bufferPtr++;
finish:
  return;
}

static void read_char(uint32_t no_args, void *address)
{
  putcharacter((uint32_t) address);
  buffer_character((char) address);
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
  //TODO
}

static keymap_t km[183][2] =
{
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
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
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&backspace_pressed, 0, NULL}, {&backspace_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
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
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&enter_pressed, 0, (void *) NULL}, {&enter_pressed, 0, NULL}},
  {{NULL, 1, (void *) NULL}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'a'}, {&read_char, 1, (void *) 'A'}},
  {{&read_char, 1, (void *) 's'}, {&read_char, 1, (void *) 'S'}},
  {{&read_char, 1, (void *) 'd'}, {&read_char, 1, (void *) 'D'}},
  {{&read_char, 1, (void *) 'f'}, {&read_char, 1, (void *) 'F'}},
  {{&read_char, 1, (void *) 'g'}, {&read_char, 1, (void *) 'G'}},
  {{&read_char, 1, (void *) 'h'}, {&read_char, 1, (void *) 'H'}},
  {{&read_char, 1, (void *) 'j'}, {&read_char, 1, (void *) 'J'}},
  {{&read_char, 1, (void *) 'k'}, {&read_char, 1, (void *) 'K'}},
  {{&read_char, 1, (void *) 'l'}, {&read_char, 1, (void *) 'L'}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'z'}, {&read_char, 1, (void *) 'Z'}},
  {{&read_char, 1, (void *) 'x'}, {&read_char, 1, (void *) 'X'}},
  {{&read_char, 1, (void *) 'c'}, {&read_char, 1, (void *) 'C'}},
  {{&read_char, 1, (void *) 'v'}, {&read_char, 1, (void *) 'V'}},
  {{&read_char, 1, (void *) 'b'}, {&read_char, 1, (void *) 'B'}},
  {{&read_char, 1, (void *) 'n'}, {&read_char, 1, (void *) 'N'}},
  {{&read_char, 1, (void *) 'm'}, {&read_char, 1, (void *) 'M'}},
  {{&read_unsint, 1, (void *) __LINE__}, {&read_unsint, 1, (void *) 44}},
  {{&read_unsint, 1, (void *) __LINE__}, {&read_unsint, 1, (void *) 44}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) ' '}, {&read_char, 1, (void *) ' '}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_released, 0, NULL}, {&shift_released, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) __LINE__}, {NULL, 0, NULL}},
  {{&shift_released, 0, NULL}, {&shift_released, 0, NULL}}
};

static void process_scancode(uint32_t scancode)
{
  keyfunc_t func = NULL;
  uint8_t shifted = (shiftCount == 0) ? 0 : 1;

  if (scancode >= sizeof(km)/sizeof(km[0])) {
    goto finish;
  }
  func = km[scancode][shifted].func;
  if (func == NULL) {
    goto finish;
  }
  func(km[scancode][shifted].no_args, km[scancode][shifted].args);
finish:
  return;
}

static void process_message(char *buffer, uint32_t bytes)
{
  buffer[bytes] = '\0';
  putstring(buffer); putcharacter(LF);
}

static void main_loop(err_t *error)
{
  uint32_t scancode = 0;
  char buffer[MAX_MSG_SIZE];
  uint32_t bytes = 0;

  for(;;) {
    terror(bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), error))
    if (*error > 0) {
      if (*error & 2) {
	terror(scancode = call_syscall_inb(0x60, error))
	process_scancode(scancode);
      }
      *error = OK;
    } else {
      bytes = (bytes < sizeof(buffer)) ? bytes : (sizeof(buffer) - 1);
      buffer[sizeof(buffer) - 1] = '\0';
      process_message(buffer, bytes);
    }
  }
finish:
  return;
}

int main()
{
  err_t error = OK; 
  cls();

  terror(call_syscall_claim_port(0x60, &error))
  terror(call_syscall_claim_port(0x3D4, &error))
  terror(call_syscall_claim_port(0x3D5, &error))

  move_cursor();

  terror(call_syscall_request_irq(1, &error))
  terror(call_syscall_set_feature(FEATURE_TTY, &error))
  terror(main_loop(&error))
finish:
  if (hasFailed(error)) {
    putstring("error: "); putstring(err2String(error)); putcharacter(LF);
  }
  putstring("will exit"); putcharacter(LF);
  call_syscall_exit();
  return 0;
}
