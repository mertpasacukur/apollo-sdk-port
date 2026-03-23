#if !defined(VERSAL_PLATFORM)


/*!
 * @brief     Hardware Abstraction Layer header file.
 *
 * @copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

#ifndef __HAL_H__
#define __HAL_H__
/*============= I N C L U D E S ============*/

/*============= D E F I N E S ==============*/
#define PLATFORM_ID_ADS10     0x0001
#define PLATFORM_ID_MZCOMMS   0x0004
#define PLATFORM_ID_HTOL      0x0010

#define PLATFORM_ID_MASK      0xFFFF
#define PLATFORM_VERSION_MASK 0xFFFF0000UL

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Looks for uio_platform_id device and reads its value.
 *
 * @param[out]  platform_id   Value read from device
 *
 * @return  0 if SUCCESS.
 *          1 if no uio_platform_id device has been found - this is for backward
 *          compatibility with older BOOT.BIN that have no PLATFORM_ID
 */
int32_t get_platform_id(uint32_t *platform_id);

#ifdef __cplusplus
}
#endif
#endif /*__HAL_H__*/

#endif /* !defined(VERSAL_PLATFORM) */
