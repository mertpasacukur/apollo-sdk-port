/*!
 * \brief     ADF4382 Power Down Control Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_pdctrl.h"
#include "adi_adf4382_regmap_inline_bfs.h"


int32_t adi_adf4382_pdctrl_pd_set(adi_adf4382_device_t *adf4382, adi_adf4382_pdctrl_target_e target, uint8_t pd)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_RANGE_CHECK(pd, 0, 1);

    int32_t err;

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_CLKOUT1))
    {
        err = adi_adf4382_bf___REG002b___PD_CLKOUT1_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_CLKOUT2))
    {
        err = adi_adf4382_bf___REG002b___PD_CLKOUT2_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_PFD_CP))
    {
        err = adi_adf4382_bf___REG002b___PD_PFDCP_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_LD))
    {
        err = adi_adf4382_bf___REG002b___PD_LD_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_VCO))
    {
        err = adi_adf4382_bf___REG002b___PD_VCO_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_NDIV))
    {
        err = adi_adf4382_bf___REG002b___PD_NDIV_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_RDIV))
    {
        err = adi_adf4382_bf___REG002b___PD_RDIV_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_ALL))
    {
        err = adi_adf4382_bf___REG002b___PD_ALL_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF4382_PDCTRL_TARGET_REF_DETECT))
    {
        err = adi_adf4382_bf___REG002a___PD_RDET_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

	if (ADI_CMS_MASK_MATCH(target, ADI_ADF5382_PDCTRL_TARGET_SYNC))
    {
        err = adi_adf4382_bf___REG002a___PD_SYNC_set(adf4382, pd);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}