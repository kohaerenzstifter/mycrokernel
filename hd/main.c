#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../usrlib/prototype.h"
#include "../include/const.h"
#include "../include/types.h"

#define PORT_DATA 0x0

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

typedef enum { NONE, UNKNOWN, ATA, ATAPI, SATA} controller_t;

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

static controller_t identify(boolean_t primary, boolean_t master,
  uint16_t buffer[], err_t *error)
{
  controller_t result = UNKNOWN;
  controller_t suspect = UNKNOWN;
  uint32_t value = 0;
  uint32_t mid = 0;
  uint32_t high = 0;
  uint32_t port_base = 0;
  boolean_t jump = FALSE;

  void do_ATA(err_t *error) {
    int i = 0;
    outf(NULL, TRUE, "do_ATA()");
    for (i = 0; i < 256; i++) {
      terror(buffer[i] = call_syscall_inb(port_base | PORT_DATA, error))
      //buffer[i] = 0;
    }
finish:
    return;
  }
  
  void do_SATA(err_t *error) {
    outf(NULL, TRUE, "do_SATA()");
  }
  
  void do_ATAPI(err_t *error) {
    outf(NULL, TRUE, "do_ATAPI()");
  }
  
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
    result = NONE;
    goto finish;
  }

  for(;;) {
    if (!(value & BIT_BUSY)) {
      suspect = ATA;
      break;
    }
    
    if (jump) {
      goto carryOn;
    }
    
    terror(mid = call_syscall_inb(PORT_LBA_MID, error))
    terror(high = call_syscall_inb(PORT_LBA_HIGH, error))
    
    if ((mid == 0)&&(high == 0)) {
      jump = TRUE;
      goto carryOn;
    }

    if ((mid == 0x14)&&(high == 0xeb)) {
      suspect = ATAPI;
      break;
    }

    if ((mid == 0x3c)&&(high == 0xc3)) {
      suspect = SATA;
      break;
    }
    
    break;

carryOn:
    terror(value = call_syscall_inb(port_base | PORT_STATUS, error))
  }

  if (suspect == ATA) {
    if (value & BIT_DRQ) {
      terror(do_ATA(error))
      result = ATA;
      goto finish;
    }
  }

  if (suspect == ATAPI) {
    terror(do_ATAPI(error))
    result = ATAPI;
    goto finish;
  }
  
  if (suspect == SATA) {
    terror(do_SATA(error))
    result = SATA;
    goto finish;
  }
 
finish:
  if (result == NONE) {
    outf(NULL, TRUE, "NO DRIVE PRESENT");
  } else if (result == UNKNOWN) {
    outf(NULL, TRUE, "UNKNOWN CONTROLLER");
  }
  return result;
}

int main()
{
  err_t err = OK;
  err_t *error = &err;
  uint16_t buffer[256];

  terror(claim_ports(error))

  outf(NULL, TRUE, "IDENTIFY PRIMARY MASTER");
  terror(identify(TRUE, TRUE, buffer, error))
  outf(NULL, TRUE, "IDENTIFY PRIMARY SLAVE");
  terror(identify(TRUE, FALSE, buffer, error))
  outf(NULL, TRUE, "IDENTIFY SECONDARY MASTER");
  terror(identify(FALSE, TRUE, buffer, error))
  outf(NULL, TRUE, "IDENTIFY SECONDARY SLAVE");
  terror(identify(FALSE, FALSE, buffer, error))
finish:

  if (hasFailed(*error)) {
    outf(NULL, TRUE, err2String(err));
  } else {
    outf(NULL, TRUE, "ALLES IN OBI", err);
  }
  call_syscall_exit();
  return 0;
}
