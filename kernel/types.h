#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#pragma pack(1)
struct _tss {
  unsigned short backlink;
//  unsigned short zeroes1;
  unsigned short interrupts;
  unsigned esp0;
  unsigned short ss0;
  unsigned short zeroes2;
  unsigned schedticks;
  unsigned char state;
  unsigned char misc;
  unsigned short zeroes3;
  unsigned ticksleft;
  unsigned short pid;
  unsigned short zeroes4;
  unsigned cr3_reg;
  unsigned eip_reg;
  unsigned eflag_reg;
  unsigned eax_reg;
  unsigned ecx_reg;
  unsigned edx_reg;
  unsigned ebx_reg;
  unsigned esp_reg;
  unsigned ebp_reg;
  unsigned esi_reg;
  unsigned edi_reg;
  unsigned short es_reg;
  unsigned short zeroes5;
  unsigned short cs_reg;
  unsigned short zeroes6;
  unsigned short ss_reg;
  unsigned short zeroes7;
  unsigned short ds_reg;
  unsigned short zeroes8;
  unsigned short fs_reg;
  unsigned short zeroes9;
  unsigned short gs_reg;
  unsigned short zeroes10;
  unsigned short ldt_sel;
  unsigned short zeroes11;
  //set lowest bit to TRUE to raise a debug
  //exception on a task switch
  unsigned char debug;
  unsigned char zeroes12;
  unsigned short iomap_base;
  struct _tss *next;
  unsigned short idx_tss;
  unsigned callMask;
  struct _tss *previous;
  struct _tss *firstSender;
  struct _tss *nextSender;
  struct _tss *receivingFrom;
  char procname[15];
};
#pragma pack()

typedef struct _schedqueue {
  tss_t *head;
  unsigned ticksleft;
  unsigned schedticks;
} schedqueue_t;

typedef struct {
  unsigned limit;
  void *base;
} DTdesc_t;


#pragma pack(1)
typedef struct _segdesc {
  unsigned short limit;
  unsigned short base1;
  unsigned char base2;
  unsigned char props1;
  unsigned char props2;
  unsigned char byte;
} segdesc_t;
#pragma pack()

#endif