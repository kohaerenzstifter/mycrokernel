#include "../include/types.h"
#include "../include/const.h"
#include "../usrlib/syscall.h"
#include "prototype.h"

int main()
{
  err_t error = OK;
  int i;
  char buffer[100];
  uint32_t bytes = 0;
  putstring("video: hallo"); putchar(LF);
  call_syscall_setFeature(1, &error);
  putstring("video: set feature"); putchar(LF);
  for(;;) {
    bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), &error);
    putstring("video: received ");putunsint(bytes); putstring(" bytes"); putchar(LF);
    buffer[bytes] = '\0';
    putstring(buffer);
    putchar(LF);
  }
}

/*int main()
{
  err_t error = OK;
  terror(call_syscall_request_irq(1, &error))
  terror(call_syscall_setFeature(FEATURE_SPEAK, &error))*/
//for(;;);


//finish:for(;;);
/*  if (error) {
    putstring("error: " error_to_string(error)); putchar(LF);
  }*/
  /*call_syscall_exit();
}*/