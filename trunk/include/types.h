#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned uint32_t;
typedef enum _boolean {FALSE = 0, TRUE} boolean_t;
struct _tss;
typedef struct _tss tss_t;
typedef uint32_t err_t;
#endif