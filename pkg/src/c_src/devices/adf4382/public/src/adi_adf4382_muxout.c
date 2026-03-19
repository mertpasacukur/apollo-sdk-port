/*!
 * \brief     ADF4382 Muxout Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_muxout.h"
#include "adi_adf4382_regmap_inline_bfs.h"


int32_t adi_adf4382_muxout_voltage_set(adi_adf4382_device_t *adf4382, uint8_t cmos_ov)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(cmos_ov, 0, 1);

    int32_t err;

    err = adi_adf4382_bf___REG003d___CMOS_OV_set(adf4382, cmos_ov);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_muxout_outputctrl_set(adi_adf4382_device_t *adf4382, adi_adf4382_muxout_outputctrl_e muxout)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
	ADI_CMS_RANGE_CHECK(muxout, ADI_ADF4382_MUXOUT_OUTPUTCTRL_HIGH_Z, ADI_ADF4382_MUXOUT_OUTPUTCTRL_ANALOG_TEST);
    int32_t err;

	err = adi_adf4382_bf___REG002e___MUXOUT_set(adf4382, muxout);
	ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}