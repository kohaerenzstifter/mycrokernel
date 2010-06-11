#ifndef KERNEL_CONST_H
#define KERNEL_CONST_H


#define LF 10
#define NO_SCHEDQUEUES 10


#define FRONT 1
#define BACK 0

#define NO_AVL_DSCRPTS 8192
#define INVALID_GDT_IDX (NO_AVL_DSCRPTS * sizeof(segdesc_t))

#define NO_AVL_PIDS 0xffff
#define INVALID_PID NO_AVL_PIDS

//state flags
#define EMPTY 1
#define RECEIVING (EMPTY << 1)
#define SENDING (RECEIVING << 1)

//misc flags
#define ENQUEUED 1

#define TESTFEATURE 1

//#define MAX_NO_PROCS 100

#define MAX_IRQ 15

#define E_NONEWPROCESS

#define MAX_NO_PROCS 100
#define NO_BASE 0xffffffff
#define CODE 0
#define DATA 1
#define STACK 2
#define PAGE_GRANULARITY 1
#define BYTE_GRANULARITY 0
#define KERNEL_BASE 0x14800
#define NUM_PORTS 65536

#endif