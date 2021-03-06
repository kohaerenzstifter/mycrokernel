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

#define TODOERROR \
	{ \
		outf(NULL, TRUE, "%s:%d: UNDEFINED ERROR", __FILE__, __LINE__); \
		goto finish; \
	}

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

typedef struct _inode {
	uint16_t type_and_perms;
	uint16_t user_id;
	uint32_t size;
	uint32_t last_access;
	uint32_t last_change;
	uint32_t last_modify_time;
	uint32_t deletion_time;
	uint16_t group_id;
	uint16_t nr_hard_links;
	uint32_t nr_disk_sectors;
	uint32_t flags;
	uint32_t os_specific1;
	uint32_t direct0;
	uint32_t direct1;
	uint32_t direct2;
	uint32_t direct3;
	uint32_t direct4;
	uint32_t direct5;
	uint32_t direct6;
	uint32_t direct7;
	uint32_t direct8;
	uint32_t direct9;
	uint32_t direct10;
	uint32_t direct11;
	uint32_t singly_indirect;
	uint32_t doubly_indirect;
	uint32_t triply_indirect;
	uint32_t generation_nr;
	uint32_t special1;
	uint32_t special2;
	uint32_t fragment_block_addr;
	char os_specific2[12];
} inode_t;

typedef struct _block_group {
	uint32_t addr_inode_usage_bitmap;
	uint32_t addr_block_usage_bitmap;
	uint32_t block_address_inode_table;
} block_group_t;

//until I can think of a better value...
#define MAX_NR_BLOCK_GROUPS 5

typedef struct _partition {
	boolean_t used;
	boolean_t bootable;
	uint32_t first_sector;
	uint32_t last_sector;
	uint32_t type;
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t nr_block_groups;
	uint32_t block_size;
	uint32_t nr_inodes;
	uint32_t nr_blocks;
	uint32_t nr_blocks_per_group;
	uint32_t nr_inodes_per_group;
	uint32_t inode_size;
	block_group_t block_groups[MAX_NR_BLOCK_GROUPS];
} partition_t;

typedef struct _channel {
  controller_t controller;
  uint32_t lba_sectors;
  uint16_t bytes_per_sector;
  uint16_t bus_info;
	uint16_t nr_heads;
	uint16_t nr_sectors_per_track;
	partition_t partitions[4];
} channel_t;

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


static void show_inode(channel_t *channel, uint8_t partition_nr,
  inode_t *inode, err_t *error);

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

static void identify(int port, int ch, err_t *error)
{
  controller_t suspect = UNKNOWN;
  uint32_t value = 0;
  uint32_t mid = 0;
  uint32_t high = 0;
  boolean_t jump = FALSE;
	uint16_t buffer[256];
  channel_t *channel = &(channels[port][ch]);

  void do_ATA(err_t *error) {
    int i = 0;
    for (i = 0; i < 256; i++) {
      terror(buffer[i] = call_syscall_inw(get_port_base(channel) | PORT_DATA, error))
    }
    channel->bytes_per_sector = buffer[5];
    channel->lba_sectors = buffer[61];
    channel->lba_sectors <<= 16;
    channel->lba_sectors |= buffer[60];
	channel->nr_heads = buffer[3];
	channel->nr_sectors_per_track = buffer[6];

    outf(NULL, TRUE, "Number of physical cylinders: %u", buffer[0]);
    outf(NULL, TRUE, "Number of heads: %u", buffer[3]);
    //outf(NULL, TRUE, "Zahl der unformatierten Bytes je physikalischer Spur: %u", buffer[4]);
    //outf(NULL, TRUE, "Zahl der unformatierten Bytes je Sektor: %u", buffer[5]);
    //outf(NULL, TRUE, "Zahl der physikalischen Sektoren je Spur: %u", buffer[6]);
    //outf(NULL, TRUE, "Puffertyp: %u", buffer[20]);
    //outf(NULL, TRUE, "Puffergroesse/512: %u", buffer[21]);
    //outf(NULL, TRUE, "Zahl der ECC-Bytes, die bei Befehlen Schreib/Lies-Lang uebergeben werden: %u", buffer[22]);
    //outf(NULL, TRUE, "Anzahl der Sektoren zwischen zwei Interrupts: %u", buffer[47]);
    outf(NULL, TRUE, "Number of logical cylinders: %u", buffer[54]);
    outf(NULL, TRUE, "Number of logical heads: %u", buffer[55]);
    outf(NULL, TRUE, "Number of logical sectors per track: %u", buffer[56]);
    uint32_t dword = buffer[58];
    dword <<= 16;
    dword |= buffer[57];
    outf(NULL, TRUE, "Bytes per logical sector: %u", dword);
    outf(NULL, TRUE, "Number of sectors in LBA mode: %u", channel->lba_sectors);
	outf(NULL, TRUE, "");
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
	  outf(NULL, TRUE, "Detected ATA drive on port %x channel %x:");
	  outf(NULL, TRUE, "");
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
    //outf(NULL, TRUE, "NO DRIVE PRESENT");
  } else if (channel->controller == UNKNOWN) {
    //outf(NULL, TRUE, "UNKNOWN CONTROLLER");
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

static void write_sector(uint32_t number, channel_t *channel, void *b, err_t *error)
{
  uint32_t i = 0;
  uint32_t bytes = 0;
  uint8_t value = 0;
  uint32_t status = 0;
	uint16_t *buffer = (uint16_t *) b;

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

static void read_sector(uint32_t number, channel_t *channel,
  char *b, err_t *error)
{
  uint32_t i = 0;
  uint32_t bytes = 0;
  uint8_t value = 0;
  uint32_t status = 0;
	uint16_t *buffer = (uint16_t *) b;

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

#define get_addr_block_usage_bitmap(block_descriptor) \
  (*((uint32_t *) (&block_descriptor[0])))
#define get_addr_inode_usage_bitmap(block_descriptor) \
  (*((uint32_t *) (&block_descriptor[4])))
#define get_block_address_inode_table(block_descriptor) \
  (*((uint32_t *) (&block_descriptor[8])))
#define get_nr_unallocated_blocks(block_descriptor) \
  (*((uint16_t *) (&block_descriptor[12])))
#define get_nr_unallocated_inodes(block_descriptor) \
  (*((uint16_t *) (&block_descriptor[14])))
#define get_nr_directories(block_descriptor) \
  (*((uint16_t *) (&block_descriptor[16])))

static void read_group_descriptor_table(channel_t *channel,
  uint8_t partition_nr, err_t *error)
{
	int i = 0;
	char block_descriptor_table[1024];
	char *block_descriptor = NULL;
	char *curbuf = block_descriptor_table;
	uint32_t sector = channel->partitions[partition_nr].first_sector;
	sector += (channel->partitions[partition_nr].block_size == 1024) ? 4 :
	  (channel->partitions[partition_nr].block_size / 512);

	terror(read_sector(sector, channel, curbuf, error))
	sector++; curbuf += 512;
	terror(read_sector(sector, channel, curbuf, error))
 
	block_descriptor = block_descriptor_table;
	for (i = 0; i < channel->partitions[partition_nr].nr_block_groups; i++) {
		channel->partitions[partition_nr].block_groups[i].addr_block_usage_bitmap =
			get_addr_block_usage_bitmap(block_descriptor);
		channel->partitions[partition_nr].block_groups[i].addr_inode_usage_bitmap =
			get_addr_inode_usage_bitmap(block_descriptor);
		channel->partitions[partition_nr].block_groups[i].block_address_inode_table =
		  get_block_address_inode_table(block_descriptor);
		/*outf(NULL, TRUE, "number unallocated blocks in group: %d",
		  get_nr_unallocated_blocks(block_descriptor));
		outf(NULL, TRUE, "number unallocated inodes in group: %d",
		  get_nr_unallocated_inodes(block_descriptor));
		outf(NULL, TRUE, "number directories in group: %d",
		  get_nr_directories(block_descriptor));*/
		block_descriptor += 32;
	}
finish:
	return;
}

static void read_inode(channel_t *channel, uint8_t partition_nr,
  uint32_t inode_nr, inode_t *inode, err_t *error)
{
	uint32_t block_group_nr = 0;
	uint32_t block_nr = 0;
	uint32_t inode_index = 0;
	partition_t *partition = NULL;
	block_group_t *block_group = NULL;
	uint32_t inode_size = 0;
	uint32_t block_size = 0;
	uint32_t inode_offset = 0;
	uint32_t block_address_inode_table = 0;
	uint32_t sector = 0;
	uint32_t offset = 0;
	char buffer[512];
	inode_t *in = NULL;

	if ((inode_nr < 1)||(inode_nr >
	  channel->partitions[partition_nr].nr_inodes)) {
		TODOERROR
	}

	partition = &(channel->partitions[partition_nr]);

	block_group_nr = (inode_nr - 1) /
	  channel->partitions[partition_nr].nr_inodes_per_group;
	inode_index = (inode_nr - 1) %
	  channel->partitions[partition_nr].nr_inodes_per_group;
	block_size = partition->block_size;
	inode_size = partition->inode_size;
	block_nr = (inode_index * inode_size) / block_size; 

	block_group = &(partition->block_groups[block_group_nr]);
	block_address_inode_table = block_group->block_address_inode_table;

	inode_offset = partition->first_sector * 512;
	inode_offset += (block_group_nr * partition->nr_blocks_per_group
	  * partition->block_size);
	inode_offset += (block_address_inode_table * partition->block_size);
	inode_offset += (inode_index * inode_size);

	sector = inode_offset / 512;
	offset = inode_offset % 512;

	terror(read_sector(sector, channel, buffer, error));
	in = ((inode_t *) (&(buffer[offset])));

	memcpy(inode, in, sizeof(inode_t));

finish:
	return;
}

static void do_direct_directory(channel_t *channel, uint8_t partition_nr,
  char *block, err_t *error)
{
	uint16_t size = 0;
	uint16_t *size_addr = NULL;
	char *cur = NULL;
	inode_t inode;
	uint32_t inode_nr = 0;

	//TODO: termination condition?
	for (cur = block; cur < &(block[1024]);) {
		size_addr = &(cur[4]); 
		if (size_addr > &(block[1023])) {
			goto finish;
		}

		outf(NULL, TRUE, "Found entry:");
		outf(NULL, TRUE, &(cur[8]));

		inode_nr = *((uint32_t *) cur);

		/*terror(read_inode(channel, partition_nr, inode_nr, &inode, error))
		terror(show_inode(channel, partition_nr, &inode, error))*/

		size = *size_addr;
		cur += size;
	}
finish:
	return;
}

static void do_indirect_directory(channel_t *channel, uint8_t partition_nr,
  uint32_t block_nr, uint8_t indirection_level, err_t *error)
{
	char block[1024];
	char *cur = NULL;
	uint32_t block_size = channel->partitions[partition_nr].block_size;
	uint32_t byte_offset = block_nr * block_size;
	uint32_t sector_nr = byte_offset / 512;
	sector_nr += channel->partitions[partition_nr].first_sector;
	uint32_t *indirect = NULL;

	if (block_size != 1024) {
		TODOERROR
	}

	cur = block;
	terror(read_sector(sector_nr, channel, cur, error))
	cur += 512;
	terror(read_sector(sector_nr, channel, cur, error))
	if (indirection_level < 1) {
		terror(do_direct_directory(channel, partition_nr, block, error))
	} else {
		for (indirect = &(block[0]); indirect < &(block[1024]); indirect++) {
			if ((*indirect) == 0) {
				goto finish;
			}
			terror(do_indirect_directory(channel, partition_nr, (*indirect),
			  (indirection_level - 1), error))
		}
	}
finish:
	return;
}

static void read_directory(channel_t *channel, uint8_t partition_nr,
  inode_t *inode, err_t *error)
{
	uint32_t *direct = NULL;
	for (direct = &(inode->direct0); direct <= &(inode->direct11);
	  direct++) {
		if ((*direct) == 0) {
			goto finish;
		}
		terror(do_indirect_directory(channel, partition_nr, (*direct), 0, error))
	}
	if (inode->singly_indirect == 0) {
		goto finish;
	}
	terror(do_indirect_directory(channel, partition_nr, inode->singly_indirect,
	  1, error))
	if (inode->doubly_indirect == 0) {
		goto finish;
	}
	terror(do_indirect_directory(channel, partition_nr, inode->doubly_indirect,
	  2, error))
	if (inode->triply_indirect == 0) {
		goto finish;
	}
	terror(do_indirect_directory(channel, partition_nr, inode->triply_indirect,
	  3, error))
finish:
	return;
}

#define DIRECTORY 0x4000

static void show_inode(channel_t *channel, uint8_t partition_nr,
  inode_t *inode, err_t *error)
{
	if (inode->type_and_perms & DIRECTORY) {
		outf(NULL, TRUE, "Reading directory");
		terror(read_directory(channel, partition_nr, inode, error))
	} else {
		outf(NULL, TRUE, "type of inode is %x", inode->type_and_perms);
	}
finish:
	return;
}

static void read_root_inode(channel_t *channel, uint8_t partition_nr,
  err_t *error)
{
	inode_t inode;
	outf(NULL, TRUE, "Reading root inode ...");
	terror(read_inode(channel, partition_nr, 2, &inode, error))
	terror(show_inode(channel, partition_nr, &inode, error))
finish:
	return;
}

#define get_block_size(superblock) (1024 << (*((uint32_t *) (&superblock[24]))))
#define get_nr_inodes(superblock) (*((uint32_t *) (&superblock[0])))
#define get_nr_blocks(superblock) (*((uint32_t *) (&superblock[4])))
#define get_nr_blocks_per_group(superblock) (*((uint32_t *) (&superblock[32])))
#define get_nr_inodes_per_group(superblock)  (*((uint32_t *) (&superblock[40])))
#define get_major_version(superblock) (*((uint32_t *) (&superblock[76])))
#define get_minor_version(superblock) (*((uint16_t *) (&superblock[62])))
#define get_inode_size(superblock) (*((uint16_t *) (&superblock[88])))

static void read_superblock(channel_t *channel, uint8_t partition_nr,
  err_t *error)
{
	char superblock[1024];
	char *curbuf = superblock;
	uint32_t sector = channel->partitions[partition_nr].first_sector + 2;

	terror(read_sector(sector, channel, curbuf, error))
	sector++; curbuf += 512;
	terror(read_sector(sector, channel, curbuf, error))
	outf(NULL, TRUE, "Suberblock signature: %x", superblock[56]);
	channel->partitions[partition_nr].block_size =
	  get_block_size(superblock);
	channel->partitions[partition_nr].nr_inodes =
	  get_nr_inodes(superblock);
	channel->partitions[partition_nr].nr_blocks =
	  get_nr_blocks(superblock);
	channel->partitions[partition_nr].nr_blocks_per_group =
	  get_nr_blocks_per_group(superblock);
	channel->partitions[partition_nr].nr_inodes_per_group =
	  get_nr_inodes_per_group(superblock);
	channel->partitions[partition_nr].major_version =
	  get_major_version(superblock);
	channel->partitions[partition_nr].inode_size =
	  (channel->partitions[partition_nr].major_version < 1) ?
		128 : get_inode_size(superblock);
	channel->partitions[partition_nr].minor_version =
	  get_minor_version(superblock);
	channel->partitions[partition_nr].nr_block_groups =
		(get_nr_inodes(superblock) / get_nr_inodes_per_group(superblock)) <
	  (get_nr_blocks(superblock) / get_nr_blocks_per_group(superblock)) ?
	  (get_nr_inodes(superblock) / get_nr_inodes_per_group(superblock)) :
	  (get_nr_blocks(superblock) / get_nr_blocks_per_group(superblock));
	if (channel->partitions[partition_nr].nr_block_groups > MAX_NR_BLOCK_GROUPS)
	  {
		TODOERROR;
	}
	terror(read_group_descriptor_table(channel, partition_nr, error))
	terror(read_root_inode(channel, partition_nr, error))

finish:
	return;
}
	
#define partition_get_bootable(entry) ((uint8_t) (*((char *) entry)))
#define partition_get_type(entry) ((uint8_t) (*(((char *) (entry)) + 4)))
#define partition_get_first_cylinder(entry) (((((uint16_t) \
  (*(((char *) (entry)) + 2))) & 0xc0) << 2) | ((uint8_t) \
  (*(((char *) (entry)) + 3))))
#define partition_get_first_head(entry) ((uint8_t) (*(((char *) \
  (entry)) + 1)))
#define partition_get_first_sector(entry) (((uint8_t) (*(((char *) \
  (entry)) + 2))) & 0x3f)
#define partition_get_last_cylinder(entry) (((((uint16_t) (*(((char *) \
  (entry)) + 6))) & 0xc0) << 2) | ((uint8_t) (*(((char *) (entry)) + 7))))
#define partition_get_last_head(entry) ((uint8_t) (*(((char *) (entry)) + 5)))
#define partition_get_last_sector(entry) (((uint8_t) (*(((char *) \
  (entry)) + 6))) & 0x3f)

/*#define chs2lba(ch, c, h, s) ((c * (ch)->nr_heads *
  (ch)->nr_sectors_per_track) + (h * (ch)->nr_sectors_per_track) + s - 1)*/
#define chs2lba(ch, c, h, s) (c * (ch)->nr_heads + h) * (ch)->nr_sectors_per_track + s - 1

void read_partition_table(channel_t *channel, err_t *error)
{
	uint8_t partition_nr = 0;
	char *entry_start = NULL;
	uint32_t i = 0;
	uint16_t cylinder = 0;
	uint16_t head = 0;
	uint16_t sector = 0;
	uint16_t buffer[256];

	terror(read_sector(0, channel, buffer, error))

	for (i = 446; i < (446 + (4 * 16)); i += 16) {
		entry_start = (char *) &(buffer[(i / 2)]);

		partition_nr = ((i - 446) / 16);

		if (partition_get_type(entry_start) == 0) {
			channel->partitions[partition_nr].used = FALSE;
			continue;
		}
		outf(NULL, TRUE, "Partition %u in use ...", partition_nr);
		channel->partitions[partition_nr].bootable =
		  partition_get_bootable(entry_start);
		channel->partitions[partition_nr].type =
			partition_get_type(entry_start);
		cylinder = (partition_get_first_cylinder(entry_start));
		head = (partition_get_first_head(entry_start));
		sector = (partition_get_first_sector(entry_start));
		channel->partitions[partition_nr].first_sector =
		  chs2lba(channel, cylinder, head, sector);
		cylinder = partition_get_last_cylinder(entry_start);
		head = partition_get_last_head(entry_start);
		sector = partition_get_last_sector(entry_start);
		channel->partitions[partition_nr].last_sector =
			chs2lba(channel, cylinder, head, sector);

		if (partition_get_type(entry_start) == 0x83) {
			terror(read_superblock(channel, partition_nr, error));
		}

		channel->partitions[partition_nr].used = TRUE;
	}
	outf(NULL, TRUE, "");
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

  terror(identify(PRIMARY, MASTER, error))
  terror(identify(PRIMARY, SLAVE, error))
  terror(identify(SECONDARY, MASTER, error))
  terror(identify(SECONDARY, SLAVE, error))

	for (i = PRIMARY; i <= SECONDARY; i++) {
		for (j = MASTER; j <= SLAVE; j++) {
			if (channels[i][j].controller == ATA) {
				outf(NULL, TRUE, "Reading partition table of channel[%d][%d] ...", i, j);
				terror(read_partition_table(&channels[i][j], error))
			}
		}
	}

finish:
  if (hasFailed(err)) {
      outf(NULL, TRUE, err2String(err));
  } else {
    outf(NULL, TRUE, "IDE driver exiting without errors!");
  }
  call_syscall_exit();
  return 0;
}
