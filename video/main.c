#include "../include/types.h"
#include "../include/const.h"
#include "../usrlib/syscall.h"
#include "prototype.h"

static uint32_t shiftCount = 0;
static char cmdBuffer[300];
static char *bufferPtr = &cmdBuffer[0];

typedef void (*keyfunc_t)(uint32_t no_args, void *address);

typedef struct _keymap {
  keyfunc_t func;
  uint32_t no_args;
  void *args;
} keymap_t;

void read_hex(uint32_t no_args, void *address)
{
  puthex((uint32_t) address);
}

void read_unsint(uint32_t no_args, void *address)
{
  putunsint((uint32_t) address);
}

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

void enter_pressed(uint32_t no_args, void *address)
{
  err_t error;
  putcharacter(LF);
/*putstring(__FILE__":"); putunsint(__LINE__); putcharacter(LF);
  call_syscall_send_by_feature(FEATURE_CMD, cmdBuffer, strlen(cmdBuffer), FALSE, &error);
putstring(__FILE__":"); putunsint(__LINE__); putcharacter(LF);*/
  bufferPtr = &cmdBuffer[0];
}

void buffer_character(char me)
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

void read_char(uint32_t no_args, void *address)
{
  putcharacter((uint32_t) address);
  buffer_character((char) address);
}

void shift_released(uint32_t no_args, void *address)
{
  shiftCount--;
}

void shift_pressed(uint32_t no_args, void *address)
{
  shiftCount++;
}

void backspace_pressed(uint32_t no_args, void *address)
{
  //TODO
}

static keymap_t km[183][2] =
{
  {{&read_hex, 1, (void *) 0}, {NULL, 0, NULL}},
  {{&read_hex, 1, (void *) 11}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 2}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 3}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 4}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 5}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 6}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 7}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 8}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 9}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 10}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 11}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 12}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 13}, {NULL, 0, NULL}},
  {{&backspace_pressed, 0, NULL}, {&backspace_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) 15}, {NULL, 0, NULL}},
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
  {{&read_unsint, 1, (void *) 16}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 17}, {NULL, 0, NULL}},
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
  {{&read_unsint, 1, (void *) 28}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 29}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 30}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) 32}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) 'z'}, {&read_char, 1, (void *) 'Z'}},
  {{&read_char, 1, (void *) 'x'}, {&read_char, 1, (void *) 'X'}},
  {{&read_char, 1, (void *) 'c'}, {&read_char, 1, (void *) 'C'}},
  {{&read_char, 1, (void *) 'v'}, {&read_char, 1, (void *) 'V'}},
  {{&read_char, 1, (void *) 'b'}, {&read_char, 1, (void *) 'B'}},
  {{&read_char, 1, (void *) 'n'}, {&read_char, 1, (void *) 'N'}},
  {{&read_char, 1, (void *) 'm'}, {&read_char, 1, (void *) 'M'}},
  {{&read_unsint, 1, (void *) 44}, {&read_unsint, 1, (void *) 44}},
  {{&read_unsint, 1, (void *) 44}, {&read_unsint, 1, (void *) 44}},
  {{&read_unsint, 1, (void *) 42}, {NULL, 0, NULL}},
  {{&shift_pressed, 0, NULL}, {&shift_pressed, 0, NULL}},
  {{&read_unsint, 1, (void *) 44}, {NULL, 0, NULL}},
  {{&read_unsint, 1, (void *) 45}, {NULL, 0, NULL}},
  {{&read_char, 1, (void *) ' '}, {&read_char, 1, (void *) ' '}},
  {{NULL, 1, (void *) 40}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 41}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 42}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 45}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 46}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 47}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 48}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 49}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 50}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 51}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 52}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 53}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 54}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 55}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 56}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 57}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 58}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 59}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 60}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 61}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 62}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 63}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 64}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 65}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 66}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 67}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 68}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 69}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 70}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 71}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 72}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 73}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 74}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 75}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 76}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 77}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 78}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 79}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 80}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 81}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 82}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 83}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 84}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 85}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 86}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 87}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 88}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 89}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 40}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 41}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 42}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 45}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 46}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 47}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 48}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 49}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 50}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 51}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 52}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 53}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 54}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 55}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 56}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 57}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 58}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 59}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 60}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 61}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 62}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 63}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 64}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 65}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 66}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 67}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 68}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 69}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 70}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 71}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 72}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 73}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 74}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 75}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 76}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 77}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 78}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 79}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 80}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 81}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 82}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 83}, {NULL, 0, NULL}},
  {{&shift_released, 0, NULL}, {&shift_released, 0, NULL}},
  {{NULL, 1, (void *) 85}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 86}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 87}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 88}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 23}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 22}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 21}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 20}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 19}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 18}, {NULL, 0, NULL}},
  {{NULL, 1, (void *) 17}, {NULL, 0, NULL}},
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
  bytes = (bytes < sizeof(buffer)) ? bytes : (sizeof(buffer) - 1);
  buffer[bytes] = '\0';
  putstring(buffer);
}

static void main_loop(err_t *error)
{
  uint32_t scancode = 0;
  char buffer[100];
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
