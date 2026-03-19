
/*!
 * \brief    Private FPGA Apollo functions implementation
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_fpga_apollo_private.h"

/*****************************************************************************/
int32_t adi_fpga_apollo_private_write32_bitfield(adi_fpga_apollo_device_t *fpga,
					uint32_t reg, uint32_t mask, uint32_t val)
{
	int32_t err = API_CMS_ERROR_OK;
	uint32_t reg_val = 0, reg_offset = 0;
	uint32_t tmask = mask;

	ADI_CMS_INVALID_PARAM_CHECK(mask == 0);

	/* Determine bf offset in register */
	while ((tmask & 1) == 0) {
		tmask = tmask >> 1;
		reg_offset++;
	}

	err = fpga->hal_info.reg_read(reg, &reg_val);
	ADI_CMS_ERROR_RETURN(err);
	
	reg_val = (reg_val & ~mask) | ((val & tmask) << reg_offset);

	err = fpga->hal_info.reg_write(reg, reg_val);
	ADI_CMS_ERROR_RETURN(err);

	return err;
}

/*****************************************************************************/
int32_t adi_fpga_apollo_private_read32_bitfield(adi_fpga_apollo_device_t *fpga,
                                                uint32_t reg, uint32_t mask, uint32_t *val)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t reg_offset = 0;
    uint32_t tmask = mask;

    ADI_CMS_INVALID_PARAM_CHECK(mask == 0);

    while ((tmask & 1) == 0)
    {
        tmask >>= 1;
        reg_offset++;
    }

    err = fpga->hal_info.reg_read(reg, val);
    ADI_CMS_ERROR_RETURN(err);

    *val = ((*val) & mask) >> reg_offset;

    return err;
}

/*****************************************************************************/
int32_t adi_fpga_apollo_private_jesd_rx_phy_prbs_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    /* Don't set skip init */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, DATA_ADC_EN,
				SKIP_RX_LINK_INIT_MASK, 0x00);
    ADI_CMS_ERROR_RETURN(err);

    /* Start capture */
    err = adi_fpga_apollo_private_capture_start(fpga, false);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/*****************************************************************************/

int32_t adi_fpga_apollo_private_capture_start(adi_fpga_apollo_device_t *fpga,
					bool skip_link_init)
{
	int32_t err = API_CMS_ERROR_OK;

	/* Optional skip link init */
	err = adi_fpga_apollo_private_write32_bitfield(fpga, DATA_ADC_EN,
			SKIP_RX_LINK_INIT_MASK, skip_link_init ? 1 : 0);
	ADI_CMS_ERROR_RETURN(err);

	/* Start capture */
	err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_CAPTURE,
		CAPTURE_START_MASK, 1);
	ADI_CMS_ERROR_RETURN(err);

	return err;
}

/*****************************************************************************/

int32_t adi_fpga_apollo_private_jesd_rx_phy_prbs_pat_set(adi_fpga_apollo_device_t *fpga,
							adi_apollo_fpga_prbs_e prbs)
{
	int32_t err = API_CMS_ERROR_OK;

	err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_RX_PRBS_CTRL,
						GT_RX_PRBS_SEL_MASK, prbs);
	ADI_CMS_ERROR_RETURN(err);

	return err;
}