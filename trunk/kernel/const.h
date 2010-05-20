#ifndef KERNEL_CONST_H
#define KERNEL_CONST_H

#define FALSE 0
#define TRUE 1

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

#define MAX_NO_PROCS 100

#endif