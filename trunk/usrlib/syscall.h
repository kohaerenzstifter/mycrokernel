#ifndef USRLIB_SYSCALL_H
#define USRLIB_SYSCALL_H

#include "../include/types.h"

void call_syscall_set_feature(uint32_t feature, err_t *error);
void call_syscall_request_irq(uint32_t irq, err_t *error);
uint32_t call_syscall_receive(tss_t *receiver, void *where, uint32_t length, err_t *error);
uint32_t call_syscall_send_by_feature(uint32_t feature,void *where,uint32_t length, boolean_t block, err_t *error);
void call_syscall_exit();
uint32_t call_syscall_inb(uint32_t port, err_t *error);
uint32_t call_syscall_claim_port(uint32_t port, err_t *error);


#define hasFailed(x) (x < OK)

#define terror(x) \
  x; \
  if hasFailed((*error)) { \
    goto finish; \
  }

#endif