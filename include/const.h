#ifndef INCLUDE_CONST_H
#define INCLUDE_CONST_H

#define OK 0
#define ERR_MSG0 "OK"
#define UNKNOWNSYSCALL -1
#define ERR_MSG1 "unknown system call"
#define INVALIDBUFFER -2
#define ERR_MSG2 "invalid buffer"
#define CIRCULARSEND -3
#define ERR_MSG3 "attempt to send to yourself"
#define WOULDBLOCK -4
#define ERR_MSG4 "would block"
#define INVALIDFEATURE -5
#define ERR_MSG5 "no such feature"
#define INVALIDIRQ -6
#define ERR_MSG6 "no such irq"
#define IRQINUSE -7
#define ERR_MSG7 "irq already in use"
#define PROCESSDIED -8
#define ERR_MSG8 "peer died"
#define NOPERMS -9
#define ERR_MSG9 "no permissions for this operation"
#define UNAVAILABLE -10
#define ERR_MSG10 "requested resource unavailable"
#define FEATUREBUSY -11
#define ERR_MSG11 "feature busy"
#define BEYONDENDOFDEVICE -12
#define ERR_MSG12 "attempt to read beyond end of device"
#define NOTATA -13
#define ERR_MSG13 "not an ATA device"



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

#define FEATURE_TTY 1
#define FEATURE_CMD (FEATURE_TTY << 1)

#define NO_COLUMNS 80
#define NO_LINES 24

#define MAX_MSG_SIZE ((NO_LINES * NO_COLUMNS) + 1)

#endif