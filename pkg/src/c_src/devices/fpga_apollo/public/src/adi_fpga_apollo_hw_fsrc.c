/*!
 * \brief     Basic FPGA Apollo functions for handling FSRC invalid samples.
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_APOLLO__
 */

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_hw_fsrc.h"
#include "adi_fpga_apollo_private.h"
#include "adi_fpga_apollo_core.h"
#include <math.h>

int32_t adi_fpga_apollo_hw_fsrc_tx_sequencer_config (adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_hw_fsrc_count_t *count)
{                    
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(count);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_1, SEQ_GPIO_CHANGE_CNT_MASK, count->gpio_change_cnt);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_2, SEQ_FIRST_TRIG_CNT_MASK, count->first_trig_cnt);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_2, SEQ_SECOND_TRIG_CNT_MASK, count->second_trig_cnt);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_3, SEQ_TX_ACCUM_RST_CNT_MASK, count->fsrc_accum_reset_cnt);
    ADI_CMS_ERROR_RETURN(err);   

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_RX_DELAY_CNT_MASK, count->rx_delay_cnt);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_3, SEQ_EN_MASK, 1);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_hw_fsrc_rx_enable_set(adi_fpga_apollo_device_t *fpga, bool enable)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, DATA_ADC_EN, RX_FSRC_EN_MASK, enable ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_hw_fsrc_rx_enable_get(adi_fpga_apollo_device_t *fpga, bool *enable)
{
    int32_t err;
    uint32_t reg32;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(enable);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, DATA_ADC_EN, RX_FSRC_EN_MASK, &reg32);
    ADI_CMS_ERROR_RETURN(err);

    *enable = reg32 != 0;

    return err;
}

int32_t adi_fpga_apollo_hw_fsrc_tx_enable_set(adi_fpga_apollo_device_t *fpga, bool enable)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_TX_CONFIG, TX_FSRC_EN_MASK, enable ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_hw_fsrc_tx_enable_get(adi_fpga_apollo_device_t *fpga, bool *enable)
{
    int32_t err;
    uint32_t reg32;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(enable);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204B_TX_CONFIG, TX_FSRC_EN_MASK, &reg32);
    ADI_CMS_ERROR_RETURN(err);

    *enable = reg32 != 0;

    return err;
}



int32_t adi_fpga_apollo_hw_fsrc_ratio_set(adi_fpga_apollo_device_t *fpga, adi_apollo_jesd_link_select_e links, uint32_t n, uint32_t m)
{
    uint64_t fsrc_ratio = ((double)m) / ((double)n) * pow(2, 64);       // @TODO - remove double
    uint32_t hi_32;
    uint32_t lo_32;
    uint32_t link_count;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    
    lo_32 = fsrc_ratio & 0x00000000ffffffff;
    hi_32 = fsrc_ratio >> 32;

    printf("FPGA FSRC RATIO regs: 0x%08x 0x%08x\n", hi_32, lo_32);

    adi_fpga_apollo_core_jtx_link_cnt_get(fpga, &link_count);

    if (links & ADI_APOLLO_LINK_A0) {
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK0_FSRC_RATIO_1, lo_32);      // [31:0]
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK0_FSRC_RATIO_2, hi_32);      // [63:32]
    }

    if ( (links & ADI_APOLLO_LINK_A1) && (link_count > 2)) {
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK1_FSRC_RATIO_1, lo_32);      // [31:0]
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK1_FSRC_RATIO_2, hi_32);      // [63:32]
    }

    if (links & ADI_APOLLO_LINK_B0) {
        adi_fpga_apollo_core_reg_set(fpga, link_count > 2 ? JESD204B_TX_LINK2_FSRC_RATIO_1 : JESD204B_TX_LINK1_FSRC_RATIO_1, lo_32);      // [31:0]
        adi_fpga_apollo_core_reg_set(fpga, link_count > 2 ? JESD204B_TX_LINK2_FSRC_RATIO_2 : JESD204B_TX_LINK1_FSRC_RATIO_2, hi_32);      // [63:32]
    }

    if ((links & ADI_APOLLO_LINK_B1) && (link_count > 2)) {
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK3_FSRC_RATIO_1, lo_32);      // [31:0]
        adi_fpga_apollo_core_reg_set(fpga, JESD204B_TX_LINK3_FSRC_RATIO_2, hi_32);      // [63:32]
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_hw_fsrc_sr_enable_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint8_t enable)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t link_pd;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    if (terminal == ADI_FPGA_APOLLO_JRX) {
        err = adi_fpga_apollo_core_reg_get(fpga, JESD204B_RX_PWR_CTRL, &link_pd);
        ADI_CMS_ERROR_RETURN(err);
        if (enable) {
            err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_RX_CONFIG, RX_REPEAT_EN_MASK, (~link_pd & 0xF));
        } else {
            err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_RX_CONFIG, RX_REPEAT_EN_MASK, 0);
        }
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_fpga_apollo_core_reg_get(fpga, JESD204B_TX_PWR_CTRL, &link_pd);
        ADI_CMS_ERROR_RETURN(err);
        if (enable) {
            err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_TX_CONFIG, TX_REPEAT_EN_MASK, (~link_pd & 0xF));
        } else {
            err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_TX_CONFIG, TX_REPEAT_EN_MASK, 0);
        }
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}

int32_t adi_fpga_apollo_hw_fsrc_sr_enable_get(adi_fpga_apollo_device_t *fpga, uint16_t terminal, uint32_t *links_enable)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);
    ADI_CMS_NULL_PTR_CHECK(links_enable);

    if (terminal == ADI_FPGA_APOLLO_JRX) {
        err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204B_RX_CONFIG, RX_REPEAT_EN_MASK, links_enable);
        ADI_CMS_ERROR_RETURN(err);
    } else {
        err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204B_TX_CONFIG, TX_REPEAT_EN_MASK, links_enable);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}