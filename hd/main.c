#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../usrlib/prototype.h"
#include "../include/const.h"
#include "../include/types.h"

#define PORT_SECTORCOUNT 0x2
#define PORT_LBA_LOW 0x3
#define PORT_LBA_MID 0x4
#define PORT_LBA_HIGH 0x5
#define PORT_DRIVESELECT 0x6
#define PORT_HEADSELECT PORT_DRIVESELECT
#define PORT_COMMAND 0x7

#define PORT_STATUS PORT_COMMAND

#define COMMAND_IDENTIFY 0xec

#define BIT_ERR 0x1

#define BIT_DRQ 0x8

#define BIT_BUSY 0x80


#define PORTBASE_PRIMARY 0x1f0
#define PORTBASE_SECONDARY 0x170

static void claim_ports(err_t *error)
{
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_SECTORCOUNT, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_LOW, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_MID, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_HIGH, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_DRIVESELECT, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_COMMAND, error))
  
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_SECTORCOUNT, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_LOW, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_MID, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_HIGH, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_DRIVESELECT, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_COMMAND, error))  
finish:
  return;
}

//false, if driver does not exist...
static boolean_t identify(boolean_t primary, boolean_t master, err_t *error)
{
  boolean_t result = FALSE;
  uint32_t value = 0;
  uint32_t port_base = 0;

  if (master) {
    value = 0xa0;
  } else {
    value = 0xb0;
  }
  if (primary) {
    port_base = PORTBASE_PRIMARY;
  } else {
    port_base = PORTBASE_SECONDARY;
  }
  terror(call_syscall_outb(port_base | 0x6, value, error))

  terror(call_syscall_outb(port_base | PORT_SECTORCOUNT, 0, error))
  terror(call_syscall_outb(port_base | PORT_LBA_LOW, 0, error))
  terror(call_syscall_outb(port_base | PORT_LBA_MID, 0, error))
  terror(call_syscall_outb(port_base | PORT_LBA_HIGH, 0, error))

  terror(call_syscall_outb(port_base | PORT_COMMAND, COMMAND_IDENTIFY, error))
  
  terror(value = call_syscall_inb(port_base | PORT_STATUS, error))
  if (value == 0) {
    //drive does not exist
    outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
    goto finish;
  }

  for(;;) {
    if (!(value & BIT_BUSY)) {
      outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
      break;
    }
    terror(value = call_syscall_inb(PORT_LBA_MID, error))
    if (value != 0) {
      //not ATA!
      outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
      break;
    }
    terror(value = call_syscall_inb(PORT_LBA_HIGH, error))
    if (value != 0) {
      //not ATA!
      outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
      break;
    }
    terror(value = call_syscall_inb(port_base | PORT_STATUS, error))
    if (value & BIT_ERR) {
      //error!
      outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
      break;
    }
    if (value & BIT_DRQ) {
      //data ready
      outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
      break;
    }
  }

  //TODO
  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);

finish:
  return result;
}

int main()
{
  err_t err = OK;
  err_t *error = &err;
  
  terror(claim_ports(error))

  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
  terror(identify(TRUE, TRUE, error))
  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
  terror(identify(TRUE, FALSE, error))
  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
  terror(identify(FALSE, TRUE, error))
  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
  terror(identify(FALSE, FALSE, error))
  outf(NULL, TRUE, __FILE__ ":%d", __LINE__);
finish:

  if (hasFailed(*error)) {
    outf(NULL, TRUE, err2String(err));
  } else {
    outf(NULL, TRUE, "ALLES IN OBI", err);
  }
  call_syscall_exit();
  return 0;
}
