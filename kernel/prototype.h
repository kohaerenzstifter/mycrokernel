#ifndef KERNEL_PROTOTYPE_H

#define KERNEL_PROTOTYPE_H

#include "../include/types.h"
#include "types.h"

void kputsint(uint32_t what);
void kputhex(uint32_t what);
void kputunsint(uint32_t what);
void kputstring(char *what);
void kputchar(char what);
void pickNextProc();
uint32_t get_limit(uint32_t gdtIndex);
void free_gdt_idx(uint32_t gdtIndex);
void free_pid(uint32_t pid);
//void call_syscall_setFeatures_C(uint8_t features);
//TODO: should return error (parameter)
//tss_t *call_syscall_requestFeatures_C(uint8_t features);
//void call_syscall_send_C(tss_t *receiver, void *where, uint32_t length);
//void call_syscall_receive_C(tss_t *receiver, void *where, uint32_t length);

void disable_interrupts();
void enable_interrupts();
void halt();

void set_error(tss_t *context, tss_t *process);
uint32_t exchange_data(tss_t *curptr,tss_t *receiver);

#define unsetStateFlag(p,f) \
  p->state &= (~(f)); \
  if (p->state == 0) { \
    enqueue(p); \
  }

#define setStateFlag(p,f) \
  p->state |= f; \
  dequeue(p);
#endif
