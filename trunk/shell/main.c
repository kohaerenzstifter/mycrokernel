#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../include/const.h"


static void main_loop(err_t *error)
{
  char buffer[MAX_MSG_SIZE];
  uint32_t bytes = 0;

  for(;;) {
    terror(bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), error))
    terror(call_syscall_send_by_feature(FEATURE_TTY, buffer, bytes, FALSE, error))
  }
finish:
  return;
}

int main()
{
  err_t error = OK;
  terror(call_syscall_set_feature(FEATURE_CMD, &error))
  terror(main_loop(&error))
finish:
  call_syscall_exit();
  return 0;
}
