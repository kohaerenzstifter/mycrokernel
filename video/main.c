#include "../include/types.h"
#include "../include/const.h"
#include "../usrlib/syscall.h"
#include "prototype.h"


int main()
{
  err_t error = OK;
  uint32_t portval = 0;
  int i;
  char buffer[42];
  uint32_t bytes = 0;
  putchar('A'); for(;;);
  terror(call_syscall_claim_port(0x60, &error))
  terror(call_syscall_request_irq(1, &error))
  terror(call_syscall_setFeature(FEATURE_SPEAK, &error))
  for(;;) {
    terror(bytes = call_syscall_receive(ANYPROC, buffer, sizeof(buffer), &error))
    if (error > 0) {
      if (error & 2) {
	terror(portval = call_syscall_inb(0x60, &error))
	//putchar('A');
	//putstring("scancode: "); puthex(portval);
      }
      error = OK;
    } else {
      //putunsint(bytes); putstring(" bytes: "); putchar(LF);
      bytes = (bytes < sizeof(buffer)) ? bytes : (sizeof(buffer) - 1);
      buffer[bytes] = '\0';
      putstring("received: "); putstring(buffer);
    }
    putchar(LF);
  }
finish:
  if (hasFailed(error)) {
    putstring("error: "); putstring(err2String(error)); putchar(LF);
  }
  putstring("will exit"); putchar(LF);
  call_syscall_exit();
  return 0;
}