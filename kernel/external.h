#ifndef KERNEL_EXTERNAL_H
#define KERNEL_EXTERNAL_H

#include "const.h"
#include "types.h"

extern unsigned hours;
extern unsigned char minutes;
extern unsigned char seconds;

extern unsigned gdtBitmap[NO_AVL_DSCRPTS / sizeof(unsigned) / 8];
extern unsigned pidBitmap[NO_AVL_PIDS / sizeof(unsigned) / 8];

extern schedqueue_t schedqueue;

extern tss_t *curptr;
extern tss_t *nextptr;
extern tss_t idlestate;


extern tss_t proctab[MAX_NO_PROCS];

extern DTdesc_t gdt;

extern tss_t syscallstate;

extern err_t err;

#endif