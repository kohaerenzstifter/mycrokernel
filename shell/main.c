#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../include/const.h"


static void main_loop(err_t *error)
{
  char buffer[300];
  uint32_t bytes = 0;

  for(;;) {
    //terror(bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), error))
  }
finish:
  return;
}

int main()
{
  err_t error = OK;
  terror(call_syscall_set_feature(FEATURE_CMD, &error))
  terror(main_loop(error))
finish:
  call_syscall_exit();
  return 0;
}
