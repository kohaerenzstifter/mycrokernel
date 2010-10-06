#ifndef KERNEL_PROTOTYPE_H

#define KERNEL_PROTOTYPE_H

#include "../include/types.h"
#include "types.h"

uint32_t get_free_gdt_idx(void);
void write_segment_desc(uint32_t index, uint32_t limit, uint32_t base,
  uint32_t present, uint32_t privilege, uint32_t type, uint32_t granularity);
void write_tss_desc(uint32_t index, uint32_t limit, uint32_t base,
  uint32_t present, uint32_t privilege, uint32_t busy, uint32_t granularity);

void kputsint(uint32_t what);
void kputhex(uint32_t what);
void kputunsint(uint32_t what);
void kputstring(char *what);
void kputchar(char what);
void pickNextProc();
uint32_t get_limit(uint32_t gdtIndex);
void free_gdt_idx(uint32_t gdtIndex);
void free_pid(uint32_t pid);
uint32_t inb(uint32_t port);
uint32_t inw(uint32_t port);
void outb(uint32_t port, uint32_t value);
//void call_syscall_setFeatures_C(uint8_t features);
//TODO: should return error (parameter)
//tss_t *call_syscall_requestFeatures_C(uint8_t features);
//void call_syscall_send_C(tss_t *receiver, void *where, uint32_t length);
//void call_syscall_receive_C(tss_t *receiver, void *where, uint32_t length);

void disable_interrupts();
void enable_interrupts();
void halt();

uint32_t exchange_data(tss_t *curptr,tss_t *receiver);

#define unsetStateFlag(p,f) \
  p->state &= (~(f)); \
  if (p->state == 0) { \
    enqueue(p); \
  }

#define setStateFlag(p,f) \
  p->state |= f; \
  dequeue(p);
  
#define set_err(proc, err) \
  proc->esi_reg = err; \

#endif
