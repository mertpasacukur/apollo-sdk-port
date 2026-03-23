/*!
 * @brief     Versal platform compatibility header.
 *
 *            Provides replacements for Linux kernel macros/functions
 *            used in the Apollo SDK but not available in Xilinx Standalone BSP.
 *
 *            Covers: div64_u64, div64_u64_rem, U64_MAX, etc.
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef __VERSAL_COMPAT_H__
#define __VERSAL_COMPAT_H__

#include <stdint.h>

/*
 * div64_u64 — 64-bit unsigned division.
 * In Linux kernel this is an optimized inline; for baremetal just use C division.
 */
#ifndef div64_u64
static inline uint64_t div64_u64(uint64_t dividend, uint64_t divisor)
{
    return dividend / divisor;
}
#endif

/*
 * div64_u64_rem — 64-bit unsigned division with remainder.
 */
#ifndef div64_u64_rem
static inline uint64_t div64_u64_rem(uint64_t dividend, uint64_t divisor, uint64_t *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}
#endif

/*
 * U64_MAX — may not be defined in some baremetal toolchains
 */
#ifndef U64_MAX
#define U64_MAX  ((uint64_t)~0ULL)
#endif

#ifndef UINT64_MAX
#define UINT64_MAX  U64_MAX
#endif

/*
 * ARRAY_SIZE — common kernel macro
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

/*
 * DIV_ROUND_UP — common kernel macro
 */
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

/*
 * DIV_ROUND_CLOSEST — common kernel macro
 */
#ifndef DIV_ROUND_CLOSEST
#define DIV_ROUND_CLOSEST(x, divisor) \
    (((x) + ((divisor) / 2)) / (divisor))
#endif

/*
 * container_of — common kernel macro
 */
#ifndef container_of
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
#endif

/*
 * min/max — common kernel macros
 */
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#endif /* __VERSAL_COMPAT_H__ */
