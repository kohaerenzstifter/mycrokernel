#include "../include/types.h"
#include "../usrlib/syscall.h"
#include "../usrlib/prototype.h"
#include "../include/const.h"
#include "../include/types.h"

#define PORT_DATA 0x0
#define PORT_ERROR 0x1
#define PORT_SECTORCOUNT 0x2
#define PORT_LBA_LOW 0x3
#define PORT_LBA_MID 0x4
#define PORT_LBA_HIGH 0x5
#define PORT_DRIVESELECT 0x6
#define PORT_HEADSELECT PORT_DRIVESELECT
#define PORT_COMMAND 0x7

#define PORT_REGULAR_STATUS PORT_COMMAND
#define PORT_ALTERNATE_STATUS 0x206

#define COMMAND_IDENTIFY 0xec
#define COMMAND_READSECTORS 0x20
#define COMMAND_WRITESECTORS 0x30

#define BIT_ERR 0x1

#define BIT_DRQ 0x8
#define BIT_SRV 0x10
#define BIT_DF 0x20
#define BIT_RDY 0x40
#define BIT_BUSY 0x80


#define PORTBASE_PRIMARY 0x1f0
#define PORTBASE_SECONDARY 0x170

#define is_master(c) ((c->bus_info & BUSINFO_MASTER) != 0)
#define is_primary(c) ((c->bus_info & BUSINFO_PRIMARY) != 0)

#define get_port_base(c) (is_primary(c) ? PORTBASE_PRIMARY : PORTBASE_SECONDARY)

#define IRQ_PRIMARY 14
#define IRQ_SECONDARY 15

#define irq_expected(c) (is_primary(c) ? IRQ_PRIMARY : IRQ_SECONDARY)

#define check_ata(c, e) \
  { \
    if (c->controller != ATA) { \
      terror(err_set(NOTATA)) \
    } \
  }

//TODO
#define check_lba_supported(c, e) \
  { \
  }

#define select_drive(c, e) \
  if (channel_selected != c) { \
    do_select_drive(c, e); \
  }

typedef enum { NONE, UNKNOWN, ATA, ATAPI, SATA} controller_t;

typedef struct _channel {
  controller_t controller;
  uint32_t lba_sectors;
  uint16_t bytes_per_sector;
  uint16_t bus_info;
} channel_t;

static uint16_t buffer[256];
static channel_t *channel_selected = NULL;

typedef struct _prdt_entry {
  uint32_t word1;
  uint32_t word2;
} prdt_entry_t;

static prdt_entry_t prdt_space[2];
static prdt_entry_t *prdt = NULL;
static uint32_t prdt_phys = 0;

static char dma_buffer_space[131072];
static char *dma_buffer = NULL;
static uint32_t dma_buffer_phys = (uint32_t) NULL;

#define NR_PORTS 2
#define CHANNELS_PER_PORT 2

#define PRIMARY 0
#define SECONDARY 1
#define MASTER 0
#define SLAVE 1

#define BUSINFO_MASTER 1
#define BUSINFO_PRIMARY (BUSINFO_MASTER << 1)

static channel_t channels[NR_PORTS][CHANNELS_PER_PORT] =
  {
    {
      {UNKNOWN, 0, 0, (BUSINFO_MASTER | BUSINFO_PRIMARY)},
      {UNKNOWN, 0, 0, (BUSINFO_PRIMARY)},
    },
    {
      {UNKNOWN, 0, 0, (BUSINFO_MASTER)},
      {UNKNOWN, 0, 0, (0)}
    }
  };


static void do_dma_buffer(err_t *error)
{
  uint32_t phys = (uint32_t) NULL;
  uint32_t next_boundary = (uint32_t) NULL;
  terror(phys = call_syscall_vir2phys(&dma_buffer_space[0], error));
  if ((phys % 65536) == 0) {
    dma_buffer = &dma_buffer_space[0];
    dma_buffer_phys = phys;
    goto finish;
  }
  next_boundary = ((phys / 65536) + 1) * 65536;
  dma_buffer = (char *) (((uint32_t) &dma_buffer_space[0]) + (next_boundary - phys));
  dma_buffer_phys = next_boundary;
finish:
  return;
}

static void do_prdt(err_t *error)
{
  uint32_t phys = (uint32_t) NULL;
  uint32_t next_boundary = (uint32_t) NULL;
  terror(phys = call_syscall_vir2phys(&prdt_space[0], error));
  if ((phys / 65536) == ((phys + sizeof(prdt_entry_t)) / 65536)) {
    prdt = &prdt_space[0];
    prdt_phys = phys;
    goto finish;    
  }
  next_boundary = ((phys / 65536) + 1) * 65536;
  prdt = (prdt_entry_t *) (((uint32_t) &prdt_space[0]) + (next_boundary - phys));
  prdt_phys = next_boundary;
finish:
  return;
}

static void claim_ports(err_t *error)
{
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_DATA, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_SECTORCOUNT, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_LOW, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_MID, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_LBA_HIGH, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_DRIVESELECT, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_COMMAND, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_ERROR, error))
  terror(call_syscall_claim_port(PORTBASE_PRIMARY | PORT_ALTERNATE_STATUS, error))

  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_DATA, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_SECTORCOUNT, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_LOW, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_MID, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_LBA_HIGH, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_DRIVESELECT, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_COMMAND, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_ERROR, error))
  terror(call_syscall_claim_port(PORTBASE_SECONDARY | PORT_ALTERNATE_STATUS, error))
finish:
  return;
}

static void do_select_drive(channel_t *channel, err_t *error)
{
  uint32_t value = 0;

  if (is_master(channel)) {
    value = 0xa0;
  } else {
    value = 0xb0;
  }
  
  terror(call_syscall_outb(get_port_base(channel) | PORT_DRIVESELECT, value, error))
  
  //TODO: delay???
  
  channel_selected = channel;

finish:
  return;
}

static void identify(channel_t *channel, err_t *error)
{
  controller_t suspect = UNKNOWN;
  uint32_t value = 0;
  uint32_t mid = 0;
  uint32_t high = 0;
  boolean_t jump = FALSE;

  void do_ATA(err_t *error) {
    int i = 0;
    outf(NULL, TRUE, "do_ATA()");
    for (i = 0; i < 256; i++) {
      terror(buffer[i] = call_syscall_inw(get_port_base(channel) | PORT_DATA, error))
    }
    channel->bytes_per_sector = buffer[5];
    channel->lba_sectors = buffer[61];
    channel->lba_sectors <<= 16;
    channel->lba_sectors |= buffer[60];
    
    /*outf(NULL, TRUE, "physikalische Zahl der Zylinder: %u", buffer[0]);
    outf(NULL, TRUE, "Zahl der Koepfe: %u", buffer[3]);
    outf(NULL, TRUE, "Zahl der unformatierten Bytes je physikalischer Spur: %u", buffer[4]);
    outf(NULL, TRUE, "Zahl der unformatierten Bytes je Sektor: %u", buffer[5]);
    outf(NULL, TRUE, "Zahl der physikalischen Sektoren je Spur: %u", buffer[6]);
    outf(NULL, TRUE, "Puffertyp: %u", buffer[20]);
    outf(NULL, TRUE, "Puffergroesse/512: %u", buffer[21]);
    outf(NULL, TRUE, "Zahl der ECC-Bytes, die bei Befehlen Schreib/Lies-Lang uebergeben werden: %u", buffer[22]);
    outf(NULL, TRUE, "Anzahl der Sektoren zwischen zwei Interrupts: %u", buffer[47]);
    outf(NULL, TRUE, "logische Zahl der Zylinder: %u", buffer[54]);
    outf(NULL, TRUE, "logische Zahl der Köpfe: %u", buffer[55]);
    outf(NULL, TRUE, "logische Zahl der Sektoren je Spur: %u", buffer[56]);
    uint32_t dword = buffer[58];
    dword <<= 16;
    dword |= buffer[57];
    outf(NULL, TRUE, "Bytes je logischem Sektor: %u", dword);
    outf(NULL, TRUE, "Anzahl Sektoren im LBA-Modus: %u", channel->lba_sectors);*/
finish:
    return;
  }
  
  void do_SATA(err_t *error) {
    outf(NULL, TRUE, "do_SATA()");
  }
  
  void do_ATAPI(err_t *error) {
    outf(NULL, TRUE, "do_ATAPI()");
  }
  
  terror(select_drive(channel, error));

  terror(call_syscall_outb(get_port_base(channel) | PORT_SECTORCOUNT, 0, error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_LOW, 0, error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_MID, 0, error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_HIGH, 0, error))

  terror(call_syscall_outb(get_port_base(channel) | PORT_COMMAND, COMMAND_IDENTIFY, error))
  
  terror(value = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))

  if (value == 0) {
    channel->controller = NONE;
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
    terror(value = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))
  }

  if (suspect == ATA) {
    if (value & BIT_DRQ) {
      terror(do_ATA(error))
      channel->controller = ATA;
      goto finish;
    }
  }

  if (suspect == ATAPI) {
    terror(do_ATAPI(error))
    channel->controller = ATAPI;
    goto finish;
  }
  
  if (suspect == SATA) {
    terror(do_SATA(error))
    channel->controller = SATA;
    goto finish;
  }
 
finish:
  if (channel->controller == NONE) {
    outf(NULL, TRUE, "NO DRIVE PRESENT");
  } else if (channel->controller == UNKNOWN) {
    outf(NULL, TRUE, "UNKNOWN CONTROLLER");
  } else if (!hasFailed(error)) {
    if (is_primary(channel)) {
      call_syscall_request_irq(14, error);
    } else {
      call_syscall_request_irq(15, error);
    }
  }
  return;
}

static void display_status(uint32_t status)
{
  outf(NULL, TRUE, "error: %u", (status & BIT_ERR) ? 1 : 0);
  outf(NULL, TRUE, "drq: %u", (status & BIT_DRQ) ? 1 : 0);
  outf(NULL, TRUE, "srv: %u", (status & BIT_SRV) ? 1 : 0);
  outf(NULL, TRUE, "df: %u", (status & BIT_SRV) ? 1 : 0);
  outf(NULL, TRUE, "rdy: %u", (status & BIT_RDY) ? 1 : 0);
  outf(NULL, TRUE, "busy: %u", (status & BIT_BUSY) ? 1 : 0);   
}

static void write_sector(uint32_t number, channel_t *channel, err_t *error)
{
  uint32_t i = 0;
  uint32_t bytes = 0;
  uint8_t value = 0;
  uint32_t status = 0;

  terror(check_ata(channel, error))
  terror(check_lba_supported(channel, error))

  if (number >= channel->lba_sectors) {
    terror(err_set(BEYONDENDOFDEVICE))
  }
  //TODO: check if drive is present (and maybe type)
  terror(select_drive(channel, error))

  if (is_master(channel)) {
    value = 0xe0;
  } else {
    value = 0xf0;
  }

  terror(call_syscall_outb(get_port_base(channel) | PORT_DRIVESELECT, (value | (number >> 24)), error))
  //can be left out
  //terror(call_syscall_outb(get_port_base(channel) | PORT_ERROR, 0, error))
  //read one sector
  terror(call_syscall_outb(get_port_base(channel) | PORT_SECTORCOUNT, 1, error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_LOW, (number & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_MID, ((number >> 8) & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_HIGH, ((number >> 16) & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_COMMAND, COMMAND_WRITESECTORS, error))

  terror(status = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))
  
  //TODO: check for error in status

  while (!(status & BIT_DRQ)) {
    terror(bytes = call_syscall_receive(ANYPROC, NULL, 0, error))
    if ((*error) != irq_expected(channel)) {
      continue;
    }
    terror(status = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))
    //TODO: check for error in status
  }

  for (i = 0; i < 256; i++) {
    terror(call_syscall_outw(get_port_base(channel), buffer[i], error))
  }
  //wait 400 ns
finish:
  return;
}

static void read_sector(uint32_t number, channel_t *channel, err_t *error)
{
  uint32_t i = 0;
  uint32_t bytes = 0;
  uint8_t value = 0;
  uint32_t status = 0;

  terror(check_ata(channel, error))
  terror(check_lba_supported(channel, error))

  if (number >= channel->lba_sectors) {
    terror(err_set(BEYONDENDOFDEVICE))
  }
  //TODO: check if drive is present (and maybe type)
  terror(select_drive(channel, error))

  if (is_master(channel)) {
    value = 0xe0;
  } else {
    value = 0xf0;
  }

  terror(call_syscall_outb(get_port_base(channel) | PORT_DRIVESELECT, (value | (number >> 24)), error))
  //can be left out
  //terror(call_syscall_outb(get_port_base(channel) | PORT_ERROR, 0, error))
  //read one sector
  terror(call_syscall_outb(get_port_base(channel) | PORT_SECTORCOUNT, 1, error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_LOW, (number & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_MID, ((number >> 8) & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_LBA_HIGH, ((number >> 16) & 0xff), error))
  terror(call_syscall_outb(get_port_base(channel) | PORT_COMMAND, COMMAND_READSECTORS, error))

  terror(status = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))
  
  //TODO: check for error in status

  while (!(status & BIT_DRQ)) {
    terror(bytes = call_syscall_receive(ANYPROC, NULL, 0, error))
    if ((*error) != irq_expected(channel)) {
      continue;
    }
    terror(status = call_syscall_inb(get_port_base(channel) | PORT_ALTERNATE_STATUS, error))
    //TODO: check for error in status
  }

  for (i = 0; i < 256; i++) {
    terror(buffer[i] = call_syscall_inw(get_port_base(channel) | PORT_DATA, error))
  }
  //wait 400 ns
finish:
  return;
}

/*static void show_off_drive_read(channel_t *channel, err_t *error)
{
  uint32_t value = 0;

  outf(NULL, TRUE, "now reading sector 0");
  terror(read_sector(0, &(channels[PRIMARY][MASTER]), error))
  outf(NULL, TRUE, "read sector 0:");
  value = buffer[0];
  value <<= 16;
  value |= buffer[1];
  outf(NULL, TRUE, "value 1st read is %u", value);
  value = value + 1;
  outf(NULL, TRUE, "changing value to %u", value);
  buffer[0] = ((value & (0xffff0000)) >> 16);
  buffer[1] = (value & (0xffff));
  terror(write_sector(0, &(channels[PRIMARY][MASTER]), error))
  value = 0;
  terror(read_sector(0, &(channels[PRIMARY][MASTER]), error))
  value = buffer[0];
  value <<= 16;
  value |= buffer[1];
  outf(NULL, TRUE, "value 2nd read is %u", value);
finish:
  return;
}*/

static void dma_read_sector(uint32_t number, channel_t *channel, err_t *error)
{
  //TODO
}

static void show_off_drive_read(channel_t *channel, err_t *error)
{
  terror(dma_read_sector(0,  &(channels[PRIMARY][MASTER]), error))
  //TODO
finish:
  return;
}

#define partition_get_bootable(entry) ((uint8_t) (*((char *) entry)))
#define partition_get_type(entry) ((uint8_t) (*(((char *) (entry)) + 4)))
#define partition_get_first_cylinder(entry) (((((uint16_t) (*(((char *) (entry)) + 2))) & 0xc0) << 2) | ((uint8_t) (*(((char *) (entry)) + 3))))
#define partition_get_first_head(entry) ((uint8_t) (*(((char *) (entry)) + 1)))
#define partition_get_first_sector(entry) (((uint8_t) (*(((char *) (entry)) + 2))) & 0x3f)
#define partition_get_last_cylinder(entry) (((((uint16_t) (*(((char *) (entry)) + 6))) & 0xc0) << 2) | ((uint8_t) (*(((char *) (entry)) + 7))))
#define partition_get_last_head(entry) ((uint8_t) (*(((char *) (entry)) + 5)))
#define partition_get_last_sector(entry) (((uint8_t) (*(((char *) (entry)) + 6))) & 0x3f)

void read_partition_table(channel_t *channel, err_t *error)
{
	char *entry_start = NULL;
	uint32_t i = 0;

	terror(read_sector(0, channel, error))

	for (i = 446; i < (446 + (4 * 16)); i += 16) {
		entry_start = (char *) &(buffer[(i / 2)]);
		if (partition_get_type(entry_start) == 0) {
			continue;
		}
		outf(NULL, TRUE, "partition: %d", ((i - 446) / 16));
		outf(NULL, TRUE, "bootable: %x", partition_get_bootable(entry_start));
		outf(NULL, TRUE, "type: %x", partition_get_type(entry_start));
		outf(NULL, TRUE, "first cylinder: %x", partition_get_first_cylinder(entry_start));
		outf(NULL, TRUE, "first head: %x", partition_get_first_head(entry_start));
		outf(NULL, TRUE, "first sector: %x", partition_get_first_sector(entry_start));
		outf(NULL, TRUE, "last cylinder: %x", partition_get_last_cylinder(entry_start));
		outf(NULL, TRUE, "last head: %x", partition_get_last_head(entry_start));
		outf(NULL, TRUE, "last sector: %x", partition_get_last_sector(entry_start));
	}

finish:
	return;
}

int main()
{
	uint32_t i = 0;
	uint32_t j = 0;
  err_t err = OK;
  err_t *error = &err;
//  terror(do_dma_buffer(error));
//  terror(do_prdt(error));
  terror(claim_ports(error))

  outf(NULL, TRUE, "IDENTIFY PRIMARY MASTER");
  terror(identify(&(channels[PRIMARY][MASTER]), error))
  outf(NULL, TRUE, "IDENTIFY PRIMARY SLAVE");
  terror(identify(&(channels[PRIMARY][SLAVE]), error))
  outf(NULL, TRUE, "IDENTIFY SECONDARY MASTER");
  terror(identify(&(channels[SECONDARY][MASTER]), error))
  outf(NULL, TRUE, "IDENTIFY SECONDARY SLAVE");
  terror(identify(&(channels[SECONDARY][SLAVE]), error))
  outf(NULL, TRUE, "now doing nothing any more");
	for (i = PRIMARY; i <= SECONDARY; i++) {
		for (j = MASTER; j <= SLAVE; j++) {
			if (channels[i][j].controller == ATA) {
				outf(NULL, TRUE, "reading partition table of channel[%d][%d]", i, j);
				terror(read_partition_table(&channels[i][j], err))
			}
		}
	}
  /*if (channels[PRIMARY][MASTER].controller == ATA) {
    terror(show_off_drive_read(&(channels[PRIMARY][MASTER]), error));
  } else {
    outf(NULL, TRUE, "will NOT read");
  }*/

  //for(;;);
finish:
  if (hasFailed(err)) {
      outf(NULL, TRUE, err2String(err));
  } else {
    outf(NULL, TRUE, "ALLES IN OBI");
  }
  call_syscall_exit();
  return 0;
}
