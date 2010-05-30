#ifndef INCLUDE_CONST_H
#define INCLUDE_CONST_H

#define OK 0
#define UNKNOWNSYSCALL -1
#define INVALIDBUFFER -2
#define CIRCULARSEND -3
#define WOULDBLOCK -4
#define INVALIDFEATURE -5
#define INVALIDIRQ -6
#define IRQINUSE -7
#define PROCESSDIED -8

#define INT0 1
#define INT1 (INT0 << 1)
#define INT2 (INT1 << 1)
#define INT3 (INT2 << 1)
#define INT4 (INT3 << 1)
#define INT5 (INT4 << 1)
#define INT6 (INT5 << 1)
#define INT7 (INT6 << 1)
#define INT8 (INT7 << 1)
#define INT9 (INT8 << 1)
#define INT10 (INT9 << 1)
#define INT11 (INT10 << 1)
#define INT12 (INT11 << 1)
#define INT13 (INT12 << 1)
#define INT14 (INT13 << 1)
#define INT15 (INT14 << 1)

#define LF 10

#define NULL 0

#define ANYPROC NULL

#define FEATURE_SPEAK 1

#endif