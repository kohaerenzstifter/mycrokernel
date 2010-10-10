#ifndef KERNEL_EXTERNAL_H
#define KERNEL_EXTERNAL_H

#include "const.h"
#include "types.h"

extern unsigned hours;
extern unsigned char minutes;
extern unsigned char seconds;

extern uint32_t gdtBitmap[NO_AVL_DSCRPTS / sizeof(unsigned) / 8];
extern uint32_t pidBitmap[NO_AVL_PIDS / sizeof(unsigned) / 8];
extern uint32_t flag;
extern schedqueue_t schedqueue;

extern tss_t *curptr;
extern tss_t *nextptr;
extern tss_t idlestate;


extern tss_t proctab[];
extern tss_t *pproctab[];

extern DTdesc_t gdt;

extern tss_t syscallstate;

#endif