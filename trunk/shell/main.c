#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../include/const.h"


int main()
{
  err_t error = OK;
  for(;;);
finish:
  call_syscall_exit();
  return 0;
}
