#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     Versal Debug Print System — Implementation
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#include <stdint.h>
#include "versal_debug.h"

/*============= G L O B A L   D A T A ======================================*/

uint32_t g_dbg_level = VERSAL_DBG_DEFAULT_LEVEL;

/*============= A P I ======================================================*/

void dbg_set_level(uint32_t level)
{
    if (level > DBG_DEBUG) {
        level = DBG_DEBUG;
    }
    g_dbg_level = level;
}

uint32_t dbg_get_level(void)
{
    return g_dbg_level;
}

#endif /* !defined(VERSAL_PLATFORM) */
