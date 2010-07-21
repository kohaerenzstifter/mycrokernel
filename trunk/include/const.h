#ifndef INCLUDE_CONST_H
#define INCLUDE_CONST_H

char *error_strings[] = {
#define OK 0
  "OK",
#define UNKNOWNSYSCALL -1
  "unknown system call",
#define INVALIDBUFFER -2
  "invalid buffer",
#define CIRCULARSEND -3
  "attempt to send to yourself",
#define WOULDBLOCK -4
  "would block",
#define INVALIDFEATURE -5
  "no such feature",
#define INVALIDIRQ -6
  "no such irq",
#define IRQINUSE -7
  "irq already in use",
#define PROCESSDIED -8
  "peer died",
#define NOPERMS -9
  "no permissions for this operation",
#define UNAVAILABLE -10
  "requested resource unavailable"
#define FEATUREBUSY -11
  "feature busy"
};

#define err2String(x) error_strings[(-x)]


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

#define MAX_MSG_SIZE 300

#endif