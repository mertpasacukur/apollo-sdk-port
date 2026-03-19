/*!
 * \brief     FPGA Apollo Clk functions
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_APOLLO__
 */

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_clk.h"
#include "adi_fpga_jesd_registers.h"
#include "adi_fpga_apollo_private.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_common_types.h"
#include "adi_fpga_apollo_clk_types.h"
#include "adi_utils.h"

/*============= C O D E ====================*/
int32_t adi_fpga_apollo_clk_line_rate_div_set(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint16_t terminal, uint32_t div)
{

    int32_t err = API_CMS_ERROR_OK;
    int32_t i;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (link_sel & (ADI_FPGA_APOLLO_LINK_0 << i)) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                err = adi_fpga_apollo_private_write32_bitfield(fpga, LINE_RATE_RX_CTRL, (LINK_0_RX_DIV_MASK << (i*8)), div);
                ADI_CMS_ERROR_RETURN(err);
            } else {
                err = adi_fpga_apollo_private_write32_bitfield(fpga, LINE_RATE_TX_CTRL, (LINK_0_TX_DIV_MASK << (i*8)), div);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    return err;
}

int32_t adi_fpga_apollo_clk_line_rate_div_get(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint16_t terminal, uint32_t *div)
{

    int32_t err = API_CMS_ERROR_OK;
    int32_t i;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (link_sel & (ADI_FPGA_APOLLO_LINK_0 << i)) {
            if (terminal == ADI_FPGA_APOLLO_JRX) {
                err = adi_fpga_apollo_private_read32_bitfield(fpga, LINE_RATE_RX_CTRL, (LINK_0_RX_DIV_MASK << (i*8)), div);
                ADI_CMS_ERROR_RETURN(err);
            } else {
                err = adi_fpga_apollo_private_read32_bitfield(fpga, LINE_RATE_TX_CTRL, (LINK_0_TX_DIV_MASK << (i*8)), div);
                ADI_CMS_ERROR_RETURN(err);
            }
        }
    }

    return err;
}

/**
 * ----------------------------------------------------------------------------
 * Link div values and the corresponding (line rate / refclk) ratio 
 * 
 * Link Div     JESD204B    JESD204C  
 * ---------    ---------   ---------
 *     0            40          66
 *     1            20          33
 *     2            10          16.5
 *     3            5           8.25
 * ----------------------------------------------------------------------------
 * JESD204B Supported Line Rates
 * 
 * Ratio             :  40.0  |  20.0  |  10.0  |  5.0  |  
 * Line Rate Min(GHz):   4.0  |   2.0  |   1.0  |   .5  |
 * Line Rate Max(GHz):  20.0  |  10.0  |   5.0  |  2.5  |
 * ----------------------------------------------------------------------------
 * JESD204C Supported Line rates
 * 
 * Ratio             :  66.0    |   33.0    |   16.5    |   8.25    | 
 * Line Rate Min(GHz):   8.56   |    4.455  |    2.0    |   1.0     | 
 * Line Rate Max(GHz):  32.75   |   16.375  |    8.1875 |   4.09375 |
 * ----------------------------------------------------------------------------
 * 
 * Determining which div setting to use:
 * 
 * - To use integers scale minimum values up by common multiplier
 * 204B -> use multiplier = 2 : [8, 4, 2, 1]
 * 204C -> use multiplier = 1000 : [8560, 4455, 2000, 1000]
 * 
 * - check: bit rate(KHz) >= minimum line rate(GHz) * (1e6 / multiplier val)
 * - if true set Link Div to corresponding value 
 * - else check next minimum line rate
 * 
 * TODO: Add lane rate adapt support for dual link configs.
*/
int32_t adi_fpga_apollo_clk_line_rate_div_calc(adi_fpga_apollo_device_t* fpga, uint16_t side_idx, uint16_t terminal, uint32_t bitrate,
                                                uint32_t ver)
{

    uint32_t div_idx;
    uint32_t min_vals[2][4] = {{8, 4, 2, 1},
                               {8560, 4455, 2000, 1000}};
    uint32_t multipliers[2] = {2, 1000};
    adi_fpga_clk_div_t *link;
 
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_RANGE_CHECK(terminal, ADI_FPGA_APOLLO_JRX, ADI_FPGA_APOLLO_JTX);

    link = &fpga->state_info.clk_info.clk_div[terminal + (side_idx * 2)];
    link->line_rate = bitrate;
    link->side = side_idx;
    link->terminal = terminal;
    link->ver = ver;

    /* check if bitrate is valid */
    if (ver && fpga->state_info.clk_info.is_prbs) {
        ADI_CMS_RANGE_CHECK(bitrate, 500000, 32750000); /* JESD204C PRBS min: .5GHz max: 32.75GHz */
    } else if (ver) {
        ADI_CMS_RANGE_CHECK(bitrate, 1000000, 32750000); /* JESD204C min: 1GHz max: 32.75GHz */
    } else {
        ADI_CMS_RANGE_CHECK(bitrate, 500000, 20000000); /* JESD204B (PRBS) min: .5GHz max: 20GHz */
    }

    for (div_idx = 0; div_idx < 4; div_idx++) {
        if (bitrate >= min_vals[ver][div_idx] * 1e6 / multipliers[ver]) {
            link->link_div = div_idx;
            break;
        }
    } 

    /* Save max lane and corresponding div rate to verify all dividers are set relatively */
    if (fpga->state_info.clk_info.max_link_rate_khz < bitrate) {
        fpga->state_info.clk_info.max_link_rate_khz = bitrate;
        fpga->state_info.clk_info.min_link_div = link->link_div;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_clk_line_rate_div_pgm(adi_fpga_apollo_device_t* fpga)
{
    int32_t err;
    uint32_t link_count, div_sel, rate_ratio;
    uint16_t link_sel, link_idx; 
    adi_fpga_clk_div_t link_info;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        link_info = fpga->state_info.clk_info.clk_div[link_idx]; 

        /* get link count */
        if (link_info.terminal == ADI_FPGA_APOLLO_JRX) {
            err = adi_fpga_apollo_core_jrx_link_cnt_get(fpga, &link_count);
            ADI_CMS_ERROR_RETURN(err);
        } else {
            err = adi_fpga_apollo_core_jtx_link_cnt_get(fpga, &link_count);
            ADI_CMS_ERROR_RETURN(err);
        }

        /* if 4 link FPGA image in use then link 0 and 1 should be set the same (likewise for link 2 and 3) */
        if (link_count == 4) {
            link_sel = (ADI_FPGA_APOLLO_LINK_0 << (link_info.side * 2)) | (ADI_FPGA_APOLLO_LINK_1 << (link_info.side * 2));
        } else if (link_count == 2) {
            link_sel = (ADI_FPGA_APOLLO_LINK_0 << link_info.side);
        } else {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_ERROR);
        }

        /* Check if div ratios are set relative to max lane rate */
        rate_ratio = fpga->state_info.clk_info.max_link_rate_khz / link_info.line_rate;
        ADI_CMS_SINGLE_SELECT_CHECK(rate_ratio);
        ADI_CMS_RANGE_CHECK(rate_ratio, 1, 8);
        
        /* Convert rate ratio to divider select (0-3) */
        if (rate_ratio == 1) {
            div_sel = ADI_FPGA_APOLLO_DIV_1;
        } else if (rate_ratio == 2) {
            div_sel = ADI_FPGA_APOLLO_DIV_2;
        } else if (rate_ratio == 4) {
            div_sel = ADI_FPGA_APOLLO_DIV_4;
        } else if (rate_ratio == 8) {
            div_sel = ADI_FPGA_APOLLO_DIV_8;
        } else {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }
        
        /* Adjust link divider relative to minimum divider */
        link_info.link_div = fpga->state_info.clk_info.min_link_div + div_sel;

        err = adi_fpga_apollo_clk_line_rate_div_set(fpga, link_sel, link_info.terminal, link_info.link_div);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}