/*!
 * @brief     Versal Debug Print System
 *
 *            Leveled debug output via dbg_printf(level, fmt, ...).
 *            Only prints if level <= current global debug level.
 *
 *            Levels:
 *              DBG_ALWAYS  (0) — Always printed (banners, critical info)
 *              DBG_ERROR   (1) — Error conditions
 *              DBG_WARNING (2) — Warnings
 *              DBG_INFO    (3) — Informational (step progress, status)
 *              DBG_DEBUG   (4) — Verbose debug (SPI tx/rx data, register values)
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#ifndef VERSAL_DEBUG_H
#define VERSAL_DEBUG_H

#include "xil_printf.h"

/*============= D E B U G   L E V E L S ====================================*/

#define DBG_ALWAYS      0U
#define DBG_ERROR       1U
#define DBG_WARNING     2U
#define DBG_INFO        3U
#define DBG_DEBUG       4U

/*============= D E F A U L T   L E V E L ==================================*/

#ifndef VERSAL_DBG_DEFAULT_LEVEL
#define VERSAL_DBG_DEFAULT_LEVEL    DBG_INFO
#endif

/*============= G L O B A L   L E V E L ====================================*/

extern uint32_t g_dbg_level;

/*============= M A C R O ==================================================*/

/*!
 * @brief  Debug print macro.
 *         Prints only if @p level <= g_dbg_level.
 *
 * Usage:
 *   dbg_printf(DBG_INFO, "Step %d done\r\n", step);
 *   dbg_printf(DBG_DEBUG, "[SPI] TX: %02X %02X %02X\r\n", tx[0], tx[1], tx[2]);
 *   dbg_printf(DBG_ERROR, "FATAL: hw_open failed err=%d\r\n", err);
 */
#define dbg_printf(level, fmt, ...) \
    do { \
        if ((level) <= g_dbg_level) { \
            xil_printf(fmt, ##__VA_ARGS__); \
        } \
    } while (0)

/*============= A P I ======================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief  Set global debug level at runtime.
 * @param[in] level  One of DBG_ALWAYS..DBG_DEBUG
 */
void dbg_set_level(uint32_t level);

/*!
 * @brief  Get current global debug level.
 * @return Current level (0-4)
 */
uint32_t dbg_get_level(void);

#ifdef __cplusplus
}
#endif

#endif /* VERSAL_DEBUG_H */
