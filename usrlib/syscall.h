#ifndef USRLIB_SYSCALL_H
#define USRLIB_SYSCALL_H
#include "../include/types.h"
void call_syscall_setFeature(uint32_t feature, err_t *error);
//TODO: should return error (parameter)
uint32_t call_syscall_receive(tss_t *receiver, void *where, uint32_t length, err_t *error);
//tss_t *call_syscall_requestFeature(uint32_t feature);
//void call_syscall_send(tss_t *receiver, void *where, uint32_t length);
uint32_t call_syscall_send_by_feature(uint32_t feature,void *where,uint32_t length, boolean_t block, err_t *error);
//int call_syscall_show_tss();
#endif