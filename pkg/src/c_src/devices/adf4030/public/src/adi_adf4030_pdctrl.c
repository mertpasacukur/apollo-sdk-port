/*!
 * \brief     Source file implementing APIs for ADF4030's Power Down control functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_PDCTRL
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_pdctrl.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_pdctrl_pd_set(adi_adf4030_device_t *adf4030, uint16_t target_block, uint8_t pd)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(pd, 0, 1);

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_ALL)) {
        err = adi_adf4030_bf___REG003c___PD_ALL_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_ADC)) {
        err = adi_adf4030_bf___REG003c___PD_ADC_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_LD)) {
        err = adi_adf4030_bf___REG0067___PD_LD_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_PLL)) {
        err = adi_adf4030_bf___REG003c___PD_PLL_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_NDIV)) {
        err = adi_adf4030_bf___REG0067___PD_NDIV_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_OSC)) {
        err = adi_adf4030_bf___REG0067___PD_OSC_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_PFDCP)) {
        err = adi_adf4030_bf___REG0067___PD_PFDCP_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_PKDET)) {
        err = adi_adf4030_bf___REG0067___PD_PKDET_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_block, ADI_ADF4030_PDCTRL_RDIV)) {
        err = adi_adf4030_bf___REG0067___PD_RDIV_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_pdctrl_bsync_pd_set(adi_adf4030_device_t *adf4030, uint16_t target_path, uint8_t pd)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(pd, 0, 1);

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_0)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_0_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_1)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_1_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_2)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_2_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_3)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_3_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_4)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_4_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_5)) {
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_5_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_6)) {
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_6_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_7)) {
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_7_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_8)) {
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_8_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    if (ADI_ADF4030_CHECK_MASK(target_path, ADI_ADF4030_BSYNC_PDCTRL_PATH_9)) {
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_9_set(adf4030, pd);
        ADI_ADF4030_CHECK_ERR_OK(err);
    }

    return API_CMS_ERROR_OK;
}


/*! @} */
