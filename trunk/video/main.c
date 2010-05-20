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
  putstring("video: setting feature");
  call_syscall_setFeature(1, &error);
  putstring("video: set feature");
  for(;;) {
    bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), &error);
    putstring("video: received ");putunsint(bytes); putstring(" bytes"); putchar(LF);
    buffer[bytes] = '\0';
    putstring(buffer);
    putchar(LF);
  }
}