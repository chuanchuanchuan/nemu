#ifndef __COMMON_H__
#define __COMMON_H__

#define USE_RAMDISK

//#define USE_CACHE

/* You will define this macro in PA4 */
//#define HAS_DEVICE

//#define LOG_FILE

#include "debug.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>

#ifndef __cplusplus
typedef enum {
	false = 0,
	true = 1
} bool;
#endif

typedef uint32_t hwaddr_t;
typedef uint32_t lnaddr_t;
typedef uint32_t swaddr_t;

typedef uint16_t ioaddr_t;

#pragma pack (1)
typedef union {
	uint32_t _4;
	uint32_t _3	: 24;
	uint16_t _2;
	uint8_t _1;
} unalign;
#pragma pack ()
#define unalign_rw(addr, len)	(((unalign *)(addr))->_##len)

#endif
