/*!
 * \brief     Basic FPGA Apollo functions
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
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_fpga_jesd_registers.h"
#include "adi_fpga_apollo_private.h"
#include "adi_utils.h"

/*============= D E F I N E S ==============*/

/*============= D A T A ====================*/

#ifdef ADS10
    /* Apollo phy to FPGA phy mapping */
    static uint8_t jrx_lane_map[] = {3, 22, 23, 13, 2, 21, 1, 20, 14, 15, 0, 12, 7, 17, 9, 8, 6, 10, 5, 11, 16, 19, 4, 18}; /* Apollo Tx to FPGA Rx */
    static uint8_t jtx_lane_map[] = {3, 13, 2, 14, 0, 12, 1, 15, 21, 22, 20, 23, 7, 8, 6, 11, 4, 16, 5, 9, 17, 18, 10, 19}; /* Apollo Rx to FPGA Tx */
#endif /* ADS10 */

#ifdef VCU128
    /* VCU128 phy to FPGA phy mapping*/
    static uint8_t jrx_lane_map[] = {19, 14, 15, 5, 18, 13, 17, 12, 6, 7, 16, 4, 23, 9, 1, 0, 22, 2, 21, 3, 8, 11, 20, 10}; /* Apollo Tx to FPGA Rx */
    static uint8_t jtx_lane_map[] = {19, 5, 18, 6, 16, 4, 17, 7, 13, 14, 12, 15, 23, 0, 22, 3, 20, 8, 21, 1, 9, 10, 2, 11}; /* Apollo Rx to FPGA Tx */
#endif /* VCU128 */

/*============= C O D E ====================*/
static uint32_t jrx_fpga_to_device_phy(uint32_t fpga_prbs_lock);

int32_t adi_fpga_apollo_core_version_get(adi_fpga_apollo_device_t *fpga,
                                        adi_fpga_apollo_version_t *fpgaVersion)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(fpgaVersion);

    fpgaVersion->major = 0;
    fpgaVersion->minor = 1;
    fpgaVersion->patch = 0;

    return err;
}

int32_t adi_fpga_apollo_core_wait_us(adi_fpga_apollo_device_t* fpga, uint32_t wait_us)
{
    int32_t err = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(!(wait_us > 0));

    err = fpga->hal_info.delay_us(fpga, wait_us);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_image_ver_get(adi_fpga_apollo_device_t* fpga, uint32_t *image_ver)
{
    int32_t err;
    uint32_t vid;
    uint32_t vid2;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(image_ver);

    *image_ver = 0;

    err = fpga->hal_info.reg_read(VERSION_ID, &vid); // version id
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_read(VERSION_ID2, &vid2); // version id2
    ADI_CMS_ERROR_RETURN(err);

    *image_ver = ((vid & 0x0000FFFF) << 16) | (vid2 & 0x0000FFFF);

    return err;
}

int32_t adi_fpga_apollo_core_reg_get(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg_offset, uint32_t *out_data)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(out_data);

    err = fpga->hal_info.reg_read(reg_offset, out_data);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_reg_set(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg_offset, uint32_t data)
{
    int32_t err = API_CMS_ERROR_OK;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = fpga->hal_info.reg_write(reg_offset, data);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_bf_set(adi_fpga_apollo_device_t *fpga,
					                uint32_t reg, uint32_t bf_mask, uint32_t val)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, reg, bf_mask, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_bf_get(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg, uint32_t bf_mask, uint32_t *val)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, reg, bf_mask, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_jtx_link_status_get(adi_fpga_apollo_device_t *fpga, uint8_t *link_up)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t fpga_reg32;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(link_up);

    /* ADS10 FPGA link status */
    err = adi_fpga_apollo_core_reg_get(fpga, 0x0208, &fpga_reg32);
    ADI_CMS_ERROR_RETURN(err);
    if (fpga->state_info.jrx[0].jesd_jesdv == 1) {
        *link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;
    } else {
        *link_up = 1; //(fpga_reg32 & 0x20) == 0x20 ? 1 : 0;
    }
    return err;
}

int32_t adi_fpga_apollo_core_jesd_rx_config(adi_fpga_apollo_device_t *fpga,
                                            adi_fpga_jesd_param_t jrx_param[],
                                            uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;
    uint32_t jrx_l_scr = 0, jrx_np = 0, jrx_m = 0, jrx_f = 0;
    uint32_t jrx_k = 0, jrx_e = 0, jrx_link_pd = 0;
    uint32_t link_count = 0;
    uint32_t src_stride = 1;        /* Used for bit field reg offsets based on # links */

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(jrx_param);
    ADI_CMS_INVALID_PARAM_CHECK(length > 4)

    /* Used to check if struct has been populated */
    for (i = 0; i < MAX_JESD_LINKS; i++) {
        jrx_param[i].valid = 1;
    }

    /*
     * Determine the number of links the image supports:
     *
     * Note: Bit fields are collapsed depending on # links. jrx_link_pd is shown below for reference:
     *
     *  2 links: image supports a single link per side
     *
     *      [3 2 1 0] jrx_link_pd
     *       x x | |---- LINK A
     *           |------ LINK B
     *
     *  4 links: image supports dual link per side
     *
     *      [3 2 1 0] jrx_link_pd
     *       | | | |---- LINK A0
     *       | | |------ LINK A1
     *       | |-------- LINK B0
     *       |-----------LINK B1
     *
    */
    err = adi_fpga_apollo_core_jrx_link_cnt_get(fpga, &link_count);
    ADI_CMS_ERROR_RETURN(err);

    // FSRC images don't support dual link, therefore bit fields are compressed
    src_stride = (link_count == 2) ? 2 : 1;

    for (i = 0; i < link_count; i++) {
        jrx_l_scr |= (((jrx_param[i * src_stride].jesd_l - 1) & 0x1F) |
            ((jrx_param[i * src_stride].jesd_scr & 0x01) << 7)) << (8 * i);
        jrx_np    |= (((jrx_param[i*src_stride].jesd_np - 1) & 0x1F) << 3) << (8 * i);
        jrx_m     |= (jrx_param[i * src_stride].jesd_m - 1) << (8 * i);
        jrx_f     |= (jrx_param[i * src_stride].jesd_f - 1) << (8 * i);
        jrx_k     |= (jrx_param[i * src_stride].jesd_k - 1) << (8 * i);
        jrx_link_pd |= (jrx_param[i * src_stride].jesd_link_pd & 0x01) << i;
        jrx_e     |= (jrx_param[i * src_stride].jesd_e - 1) << (8 * i);
    }
    err = fpga->hal_info.reg_write(JESD204B_RX_CONFIG, jrx_param[0].jesd_subclass);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_PWR_CTRL, jrx_link_pd);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_L_SCR, jrx_l_scr);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_NP, jrx_np);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_M, jrx_m);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_F, jrx_f);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_RX_K, jrx_k);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204C_RX_E, jrx_e);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(DATA_PROTOCOL_SEL, (jrx_param[0].jesd_jesdv & 0x03));
    ADI_CMS_ERROR_RETURN(err);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_RX_CONFIG,
        JESD204B_RX_SUBCLASS_MASK, jrx_param[0].jesd_subclass);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t link = 0; link < length; link++){
        fpga->state_info.jrx[link] = jrx_param[link];
    }

    /*
     * Determine capture framing parameters.
     * These parameters are stored on the FPGA device context and are referenced during capture read back.
     */
    err = adi_fpga_apollo_capture_frame_populate(fpga);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_jesd_tx_config(adi_fpga_apollo_device_t *fpga,
                                            adi_fpga_jesd_param_t jtx_param[],
                                            uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;
    uint32_t jtx_l_scr = 0, jtx_np = 0, jtx_m = 0, jtx_f = 0;
    uint32_t jtx_k = 0, jtx_e = 0, jtx_link_pd = 0, jtx_link_param = 0;
    uint32_t link_count = 0;
    uint32_t src_stride = 1;        /* Used for bit field reg offsets based on # links */

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(jtx_param);
    ADI_CMS_INVALID_PARAM_CHECK(length > 4)

    /* Used to check if struct has been populated */
    for (i = 0; i < MAX_JESD_LINKS; i++) {
        jtx_param[i].valid = 1;
    }

    /*
     * Determine the number of links the image supports:
     *
     * Note: Bit fields are collapsed depending on # links. jtx_link_pd is shown below for reference:
     *
     *  2 links: image supports a single link per side
     *
     *      [3 2 1 0] jtx_link_pd
     *       x x | |---- LINK A
     *           |------ LINK B
     *
     *  4 links: image supports dual link per side
     *
     *      [3 2 1 0] jtx_link_pd
     *       | | | |---- LINK A0
     *       | | |------ LINK A1
     *       | |-------- LINK B0
     *       |-----------LINK B1
     *
    */
    err = adi_fpga_apollo_core_jtx_link_cnt_get(fpga, &link_count); // link_count val is 1-based
    ADI_CMS_ERROR_RETURN(err);

    // FSRC images don't support dual link, therefore bit fields are compressed
    src_stride = (link_count == 2) ? 2 : 1;

    for (i = 0; i < link_count; i++) {
        jtx_l_scr |= (((jtx_param[i * src_stride].jesd_l - 1) & 0x1F) |
            ((jtx_param[i * src_stride].jesd_scr & 0x01) << 7)) << (8 * i);
        jtx_np    |= (((jtx_param[i * src_stride].jesd_np - 1) & 0x1F) << 3) << (8 * i);
        jtx_m     |= (jtx_param[i * src_stride].jesd_m - 1) << (8 * i);
        jtx_f     |= (jtx_param[i * src_stride].jesd_f - 1) << (8 * i);
        jtx_k     |= (jtx_param[i * src_stride].jesd_k - 1) << (8 * i);
        jtx_link_pd |= (jtx_param[i * src_stride].jesd_link_pd & 0x01) << i;
        jtx_e |= (jtx_param[i * src_stride].jesd_e - 1) << (8 * i);
        jtx_link_param = (((jtx_param[i * src_stride].jesd_s - 1) & 0x1F) << 19) |
                         (((jtx_param[i * src_stride].jesd_n - 1) & 0x1F) << 9) |
                         (((jtx_param[i * src_stride].jesd_ns - 1) & 0xFF) << 24);
        err = fpga->hal_info.reg_write(JESD204B_TX_LINK0_CTRL + i, jtx_link_param);
        ADI_CMS_ERROR_RETURN(err);
    }
    err = fpga->hal_info.reg_write(JESD204B_TX_PWR_CTRL, jtx_link_pd);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_TX_L_SCR, jtx_l_scr);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_TX_NP, jtx_np);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_TX_M, jtx_m);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_TX_F, jtx_f);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204B_TX_K, jtx_k);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(JESD204C_TX_E, jtx_e);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(DATA_PROTOCOL_SEL, (jtx_param[0].jesd_jesdv & 0x03));
    ADI_CMS_ERROR_RETURN(err);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204B_TX_CONFIG,
        JESD204B_TX_SUBCLASS_MASK, jtx_param[0].jesd_subclass);
    ADI_CMS_ERROR_RETURN(err);

    for (uint8_t link = 0; link < length; link++){
        fpga->state_info.jtx[link] = jtx_param[link];
    }
    return err;
}

int32_t adi_fpga_apollo_core_capture_status(adi_fpga_apollo_device_t *fpga, uint8_t *status)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t temp;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(status);

    /* Read capture status */
    err = fpga->hal_info.reg_read(CTRL_CAPTURE, &temp);
    ADI_CMS_ERROR_RETURN(err);
    *status = (uint8_t) temp;
    return err;
}

int32_t adi_fpga_apollo_core_capture_start_x(adi_fpga_apollo_device_t* fpga)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);

    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_EN_MASK, 0);    // Enable the ext FPGA trigger
    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_MASK, 0);       // This toggle (0->1) starts a trigger sequence
    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_EN_MASK, 1);    // Enable the ext FPGA trigger
    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_DEBUG_MASK, 0);          // Disable optional cap trig to GPIO

    adi_fpga_apollo_core_capture_start(fpga);           // Arm the FPGA capture. Capture executes when SYSREF sequencer starts (in func adi_fpga_apollo_core_capture_start_x2()
    fpga->hal_info.delay_us(fpga, 5*1000);

    return 0;
}


int32_t adi_fpga_apollo_core_capture_start_x2(adi_fpga_apollo_device_t* fpga)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);

    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_MASK, 0);    // This toggle starts a trigger sequence
    adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_MASK, 1);
    return 0;
}

int32_t adi_fpga_apollo_core_capture_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Start capture */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_CAPTURE,
                    CAPTURE_START_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_capture_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Stop capture */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_CAPTURE,
                    CAPTURE_STOP_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_capture_size_set(adi_fpga_apollo_device_t *fpga, uint64_t cap_size)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(cap_size > FPGA_MAX_CAPTURE_SIZE || cap_size == 0);

    /* Set capture size. Note: cap_size is the number of 64K blocks, set size in bytes / (64*1024) */
    err = fpga->hal_info.reg_write(GT_RX_CAPTURE_LEN, cap_size & 0xffffffffu);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(GT_RX_CAPTURE_LEN_H, (cap_size >> 32) & 0xffffffffu);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_capture_complete_wait(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t val = 0;
    uint8_t tries = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    do
    {
        err = adi_fpga_apollo_private_read32_bitfield(fpga, STATUS_MISC, FIFO_READ_READY_MASK, &val);
        ADI_CMS_ERROR_RETURN(err);
        if (val == 1) {
            return API_CMS_ERROR_OK;
        }
        fpga->hal_info.delay_us(fpga, 50000);
    } while (++tries < 200);

    ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OPERATION_TIMEOUT);
    return err;
}

int32_t adi_fpga_apollo_core_fifo_ready_set(adi_fpga_apollo_device_t* fpga, uint32_t val)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, STATUS_MISC,
        FIFO_READ_READY_MASK, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


int32_t adi_fpga_apollo_core_fifo_ready_get(adi_fpga_apollo_device_t* fpga, uint32_t *val)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, STATUS_MISC,
        FIFO_READ_READY_MASK, val);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


int32_t adi_fpga_apollo_core_not_running_wait(adi_fpga_apollo_device_t *fpga) {
    int32_t err = API_CMS_ERROR_OK;
    uint32_t val = 1;
    uint8_t tries = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    do {
        err = adi_fpga_apollo_private_read32_bitfield(fpga, 0x948, 0x00000001, &val);
        ADI_CMS_ERROR_RETURN(err);
        if (val == 0) {
            return API_CMS_ERROR_OK;
        }
        fpga->hal_info.delay_us(fpga, 500000);
    } while (++tries < 20);

    ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OPERATION_TIMEOUT);
    return err;
}

int32_t adi_fpga_apollo_core_pattern_read_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Start pattern read */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_LOAD_CTRL,
                    GT_TX_PTN_READ_START_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_pattern_read_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Stop pattern read */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_LOAD_CTRL,
                    GT_TX_PTN_READ_STOP_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_memory_section_select (adi_fpga_apollo_device_t *fpga,
                                                    uint8_t section_start, uint8_t section_end)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* section start config */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_LOAD_CTRL,
                    GT_TX_PTN_LOAD_SECTION_START_MASK, section_start);
    ADI_CMS_ERROR_RETURN(err);

    /* section end config */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_LOAD_CTRL,
                    GT_TX_PTN_LOAD_SECTION_END_MASK, section_end);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_transmit_link_config(adi_fpga_apollo_device_t *fpga,
                                            uint32_t mem_addr, uint32_t len, uint8_t dual_link)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_SKIP_DATA_MASK, 0);
    ADI_CMS_ERROR_RETURN(err);

    /* Set address to all the links even though link is not enabled */
    for(int i=0; i<4;i++) {
        err = fpga->hal_info.reg_write(0x598 + i, mem_addr);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Set length of all the links even though link is not enabled */
    for(int i=0; i<4; i++) {
        err = fpga->hal_info.reg_write(0x588 + i, len);
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Set transmit data repeatedly */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_CTRL,
                    GT_TX_DATA_MODE_MASK, 0);
    ADI_CMS_ERROR_RETURN(err);
    /* All memory for link group if single link */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, GT_TX_PATTERN_CTRL,
                    TX_SINGLE_LINK_FULL_MEM_MASK, (dual_link == 0) ? 3 : 0);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_write_memory(adi_fpga_apollo_device_t *fpga,
                                            uint32_t mem_addr, uint8_t *tx_data,
                                            uint32_t tx_data_bytes)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(tx_data);

    err = fpga->hal_info.mem_write(mem_addr, tx_data_bytes, tx_data);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_memory_write(adi_fpga_apollo_device_t *fpga,
                                          uint32_t mem_addr, uint8_t *tx_data,
                                          uint32_t tx_data_bytes)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(tx_data);

    err = fpga->hal_info.mem_write(mem_addr, tx_data_bytes, tx_data);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_memory_read(adi_fpga_apollo_device_t *fpga,
                                         uint32_t mem_addr, uint8_t data[],
                                         uint32_t data_len)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(data);

    err = fpga->hal_info.mem_read(mem_addr, data_len, data);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_transmit_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Start pattern transmit */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_START_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_transmit_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Stop pattern transmit */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_STOP_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_ptn_play_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t jtx_link_pd;
    uint32_t play_links;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = fpga->hal_info.reg_read(JESD204B_TX_PWR_CTRL, &jtx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    play_links = ((~jtx_link_pd & 0xF) << 8) | 0x01;

    /* Apply to all links */
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_PLAY_CTRL, play_links);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_ptn_play_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t jtx_link_pd;
    uint32_t play_links;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = fpga->hal_info.reg_read(JESD204B_TX_PWR_CTRL, &jtx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    play_links = ((~jtx_link_pd & 0xF) << 8) | 0x04;

    /* Apply to all links */
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_PLAY_CTRL, play_links);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_tx_fsrc_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Stop pattern transmit */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_FSRC_STOP, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_tx_fsrc_change_rate(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Stop pattern transmit */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_FSRC_CHANGE_RATE, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_tx_fsrc_seq_start(adi_fpga_apollo_device_t* fpga)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Start the SYSREF sequencer */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_3, SEQ_START_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_pattern_len_set(adi_fpga_apollo_device_t *fpga, uint64_t len)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Set pattern length */
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_LEN, (uint32_t)len & 0xffffffffu);
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_LEN_H, (uint32_t)(len >> 32) & 0xffffffffu);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_pattern_addr_set(adi_fpga_apollo_device_t *fpga, uint32_t addr)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* repeated pattern */
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_CTRL, 0x00);
    ADI_CMS_ERROR_RETURN(err);
    /* Set pattern length */
    err = fpga->hal_info.reg_write(GT_TX_PATTERN_ADDR, addr);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_init(adi_fpga_apollo_device_t *fpga,
                        adi_apollo_fpga_prbs_e prbs)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Select the FPGA PRBS pattern */
    err = adi_fpga_apollo_private_jesd_rx_phy_prbs_pat_set(fpga, prbs);
    ADI_CMS_ERROR_RETURN(err);

    /* Enable lanes on FPGA */
    err = adi_fpga_apollo_private_jesd_rx_phy_prbs_lanes_set(fpga);
    ADI_CMS_ERROR_RETURN(err);

    /* Enable Links A-link0, B-link0. Disable links A-link1, B-link1 */
    if (fpga->state_info.jrx_link_count == 4) {
        err = adi_fpga_apollo_private_write32_bitfield(fpga,
            JESD204B_RX_PWR_CTRL,
            JESD204B_RX_LINK_PD_MASK,
            0x0A);
        ADI_CMS_ERROR_RETURN(err);
    } else if (fpga->state_info.jrx_link_count == 2) {
        err = adi_fpga_apollo_private_write32_bitfield(fpga,
            JESD204B_RX_PWR_CTRL,
            JESD204B_RX_LINK_PD_MASK,
            0x0C);
        ADI_CMS_ERROR_RETURN(err);
    } else {
		printf("Number of links: %d not supported\n", fpga->state_info.jrx_link_count);
	}

    /* Start PRBS capture (needed after setting prbs sel) */
    err = adi_fpga_apollo_private_jesd_rx_phy_prbs_start(fpga);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_print_errors(adi_fpga_apollo_device_t *fpga,
                adi_apollo_fpga_prbs_e prbs, bool clear_errors, uint16_t *active_lanes, uint16_t *err_lanes)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t cycle_cnt_low, cycle_cnt_hi;
    uint32_t error_cnt, gt_rx_prbs_locked, apollo_prbs_locked;
    uint16_t num_lanes_active = 0, num_lanes_with_errors = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_core_jesd_rx_phy_prbs_request_update(fpga, clear_errors);
    ADI_CMS_ERROR_RETURN(err);

    adi_fpga_apollo_core_reg_get(fpga, GT_RX_PRBS_LOCKED, &gt_rx_prbs_locked);
    apollo_prbs_locked = jrx_fpga_to_device_phy(gt_rx_prbs_locked);
    printf("PRBS LOCKED: 0x%08x(fpga)    0x%08x(apollo)\n",
            gt_rx_prbs_locked, apollo_prbs_locked);

    /* Get cycle and error counts */
    for (uint32_t i = 0; i < 24; i++) {
        err = fpga->hal_info.reg_read(GT_RX_PRBS_CYCLE_CNT1 + jrx_lane_map[i],
            &cycle_cnt_low);
        ADI_CMS_ERROR_RETURN(err);
        err = fpga->hal_info.reg_read(GT_RX_PRBS_CYCLE_CNT2 + jrx_lane_map[i],
            &cycle_cnt_hi);
        ADI_CMS_ERROR_RETURN(err);

        if (cycle_cnt_low != 0 || cycle_cnt_hi != 0) {
            num_lanes_active++;
        }

        err = fpga->hal_info.reg_read(GT_RX_PRBS_ERR_CNT + jrx_lane_map[i],
            &error_cnt);
        ADI_CMS_ERROR_RETURN(err);

        if (error_cnt != 0) {
            num_lanes_with_errors++;
        }

        /* Results in terms of Apollo phy lanes */
        printf("PRBS CYCLES[%2d]: 0x%08x%08x    ERRORS[%2d]: 0x%08x %s    LOCKED[%2d]: %d\n",
            i,
            cycle_cnt_hi,
            cycle_cnt_low,
            i,
            error_cnt,
            (error_cnt == 0) ? " " : "*",
            i,
            (apollo_prbs_locked >> i) & 0x01);
    }
    printf("PRBS #Lanes w/ Activity: %d    #Lanes w/ Errors: %d\n\n",
        num_lanes_active, num_lanes_with_errors);

    *active_lanes = num_lanes_active;
    *err_lanes = num_lanes_with_errors;

    return err;
    }



int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_request_update(adi_fpga_apollo_device_t *fpga,
                                bool clear_errors)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Force prbs counter reset */
    err = adi_fpga_apollo_private_write32_bitfield(fpga,
        GT_RX_PRBS_CTRL,
        GT_RX_PRBS_CNT_RESET_MASK,
        clear_errors ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);

	fpga->hal_info.delay_us(fpga, 1000000); /* Delay to accumulate cycles */

    /* Request cycle and error counter update */
    err = adi_fpga_apollo_private_write32_bitfield(fpga,
        GT_RX_PRBS_CTRL,
        GT_RX_PRBS_CNT_REQ_MASK,
        1);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_start(adi_fpga_apollo_device_t *fpga,
                         bool invert)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    if (invert) {
        fpga->hal_info.reg_write(TX_POLARITY, POLARITY_INVERTED);
    } else {
        fpga->hal_info.reg_write(TX_POLARITY, POLARITY_NOT_INVERTED);
    }

    /* Set FPGA JTx L param (12 lanes on link0 and link2) */
    err = adi_fpga_apollo_private_write32_bitfield(fpga,
        JESD204B_TX_L_SCR,
        JESD204B_LINK0_TX_L_PARAM_MASK,
        (NUM_JTX_LANES / 2 - 1));
    ADI_CMS_ERROR_RETURN(err);

    if (fpga->state_info.jtx_link_count == 4) {
        err = adi_fpga_apollo_private_write32_bitfield(fpga,
            JESD204B_TX_L_SCR,
            JESD204B_LINK2_TX_L_PARAM_MASK,
            (NUM_JTX_LANES / 2 - 1));
        ADI_CMS_ERROR_RETURN(err);

        /* Power up Link0 and Link2 */
        err = fpga->hal_info.reg_write(JESD204B_TX_PWR_CTRL, 0x0A);
        ADI_CMS_ERROR_RETURN(err);
    } else if (fpga->state_info.jtx_link_count == 2) {
        err = adi_fpga_apollo_private_write32_bitfield(fpga,
            JESD204B_TX_L_SCR,
            JESD204B_LINK1_TX_L_PARAM_MASK,
            (NUM_JTX_LANES / 2 - 1));
        ADI_CMS_ERROR_RETURN(err);

        /* Power up Link0 and Link1 */
        err = fpga->hal_info.reg_write(JESD204B_TX_PWR_CTRL, 0x0C);
        ADI_CMS_ERROR_RETURN(err);
	} else {
		printf("Number of links: %d not supported\n", fpga->state_info.jrx_link_count);
	}
    err = adi_fpga_apollo_private_write32_bitfield(fpga,
        CTRL_TRANSMIT,
        TRANSMIT_START_MASK,
        1);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_pattern_set(adi_fpga_apollo_device_t *fpga,
                        adi_apollo_fpga_prbs_e prbs[],
                        uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t lane = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(length > NUM_JTX_LANES);

    /* The PRBS pattern for each lane */
    for (lane = 0; lane < length; lane++) {
        err = adi_fpga_apollo_private_write32_bitfield(fpga,
            GT_TX_PRBS_CTRL + jtx_lane_map[lane],
            GT_TX_PRBS_SEL_MASK,
            prbs[lane]);
        ADI_CMS_ERROR_RETURN(err);
    }

    return err;
}


int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_drive_config(adi_fpga_apollo_device_t *fpga,
                    uint8_t diff_amp[], uint8_t pre_cur[],
                    uint8_t post_cur[], uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t lane = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(length > NUM_JTX_LANES);

    for (lane = 0; lane < length; lane++) {
        err = fpga->hal_info.reg_write((GT_TX_DRV_CTRL + jtx_lane_map[lane]),
            ((diff_amp[lane] << GT_TX_DIFF_CTRL_REG_OFST) & GT_TX_DIFF_CTRL_MASK) |
            ((pre_cur[lane]  << GT_TX_PRE_CURSOR_REG_OFST) & GT_TX_PRE_CURSOR_MASK) |
            ((post_cur[lane] << GT_TX_POST_CURSOR_REG_OFST) & GT_TX_POST_CURSOR_MASK));

        ADI_CMS_ERROR_RETURN(err);
    }
    return err;
}

int32_t adi_fpga_apollo_core_bidir_start(adi_fpga_apollo_device_t *fpga)
{
    int32_t err;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = adi_fpga_apollo_core_reg_set(fpga, BIDIR_CTRL, BIDIR_START_MASK);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_bidir_stop(adi_fpga_apollo_device_t *fpga)
{
    int32_t err;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = adi_fpga_apollo_core_reg_set(fpga, BIDIR_CTRL, BIDIR_STOP_MASK);
    if (err != API_CMS_ERROR_OK)
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_skip_rx_link_init_set(adi_fpga_apollo_device_t *fpga, uint8_t skip_link_init)
{
    int32_t err;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, DATA_ADC_EN, SKIP_RX_LINK_INIT_MASK, skip_link_init ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}


int32_t adi_fpga_apollo_core_rx_keep_link_mask_set(adi_fpga_apollo_device_t* fpga, uint8_t mask)
{
    int32_t err;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, DATA_ADC_EN, RX_KEEP_LINK_MASK, mask);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_tx_keep_link_mask_set(adi_fpga_apollo_device_t* fpga, uint8_t mask)
{
    int32_t err;
    ADI_CMS_NULL_PTR_CHECK(fpga);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT, TX_KEEP_LINK_MASK, mask);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_jtx_link_cnt_get(adi_fpga_apollo_device_t *fpga, uint32_t* link_count)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(link_count);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204B_TX_LINK_COUNT, JESD204B_TX_LINK_COUNT_MASK, link_count);   // get link_cnt-1

    *link_count += 1;

    return err;
}

int32_t adi_fpga_apollo_core_jrx_link_cnt_get(adi_fpga_apollo_device_t *fpga, uint32_t* link_count)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(link_count);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204B_RX_LINK_COUNT, JESD204B_RX_LINK_COUNT_MASK, link_count);   // get link_cnt-1

    *link_count += 1;

    return err;
}


int32_t adi_fpga_apollo_core_supports_hw_tl_get(adi_fpga_apollo_device_t *fpga, bool* supports_hw_tl)
{
    int32_t err;
    uint32_t vid;
    uint32_t vid2;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(supports_hw_tl);

    err = fpga->hal_info.reg_read(VERSION_ID, &vid); // version id
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_read(VERSION_ID2, &vid2); // version id2
    ADI_CMS_ERROR_RETURN(err);

    // Note: Until a bitfield is provided to indicate hw_tl support status, use well-known absolute fpga date codes.
    *supports_hw_tl = ( ( (vid == 0x2208) && (vid2 == 0x3000))
        || ((vid == 0x2205) && (vid2 == 0x2700))
        || ((vid == 0x2210) && (vid2 == 0x0300))
        || ((vid == 0x2210) && (vid2 == 0x0500))
        || ((vid == 0x2309) && (vid2 == 0x2100))
        );

    return err;
}

int32_t adi_fpga_apollo_core_supports_hw_fsrc_get(adi_fpga_apollo_device_t *fpga, bool* supports_hw_fsrc)
{
    int32_t err;
    uint32_t vid;
    uint32_t vid2;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(supports_hw_fsrc);

    err = fpga->hal_info.reg_read(VERSION_ID, &vid); // version id
    ADI_CMS_ERROR_RETURN(err);
    err = fpga->hal_info.reg_read(VERSION_ID2, &vid2); // version id2
    ADI_CMS_ERROR_RETURN(err);

    // Note: Until a bitfield is provided to indicate hw fsrc support status, use well-known absolute fpga date codes.
    *supports_hw_fsrc = (((vid == 0x2208) && (vid2 == 0x3000))
        || ((vid == 0x2210) && (vid2 == 0x0300))
        || ((vid == 0x2210) && (vid2 == 0x0500))
        || ((vid == 0x2309) && (vid2 == 0x2100))
        );

    return err;
}

int32_t adi_fpga_apollo_core_feature_flags_get(adi_fpga_apollo_device_t *fpga, adi_fpga_feature_flag_t *feature_flags)
{
    int32_t err;
    uint32_t vid;
    uint32_t feature_flag_reg = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(feature_flags);

    /* Check for legacy (on or before May 2022) FPGA images (i.e. doesn't have feature flag register) */
    err = fpga->hal_info.reg_read(VERSION_ID, &vid); // version id
    ADI_CMS_ERROR_RETURN(err);

    if (vid <= 0x2205) {
        /* Legacy FPGAs */
        feature_flags->raw_data = 0;
        feature_flags->spi_clk_adj = 0;
        feature_flags->dl_demo = 1;
        feature_flags->rx_hw_tl = 1;        /* FPGA handles JESD TPL (ADC) */
        feature_flags->tx_hw_tl = 0;        /* FPGA doesn't handle JESD TPL (DAC)*/
        feature_flags->rx_hw_fsrc = 0;      /* FPGA doesn't support HW FSRC (ADC) */
        feature_flags->tx_hw_fsrc = 0;      /* FPGA doesn't support HW FSRC (DAC) */
        feature_flags->rx_capture_delay = 0;
    } else {
        /* FPGA provides feature flags */
        err = fpga->hal_info.reg_read(FEATURE_FLAGS, &feature_flag_reg); // version id
        ADI_CMS_ERROR_RETURN(err);
        feature_flags->raw_data = ((feature_flag_reg >> 7) & 0x01);
        feature_flags->spi_clk_adj = ((feature_flag_reg >> 10) & 0x01);
        feature_flags->dl_demo = ((feature_flag_reg >> 11) & 0x01);
        feature_flags->rx_hw_tl = ((feature_flag_reg >> 12) & 0x01);
        feature_flags->tx_hw_tl = ((feature_flag_reg >> 13) & 0x01);
        feature_flags->rx_hw_fsrc = ((feature_flag_reg >> 14) & 0x01);
        feature_flags->tx_hw_fsrc = ((feature_flag_reg >> 15) & 0x01);
        feature_flags->rx_capture_delay = ((feature_flag_reg >> 16) & 0x01);

    }

    return err;
}


int32_t adi_fpga_apollo_core_sys_indicator_set(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sys_indicator_e indicator)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(!(indicator == ADI_FPGA_APOLLO_RX_SYS_READY_INDICATOR || indicator == ADI_FPGA_APOLLO_TX_PAT_START_INDICATOR));

    err = adi_fpga_apollo_private_write32_bitfield(fpga, MISC_0, SMA_J3_SEL_MASK, indicator);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_sys_indicator_get(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sys_indicator_e* indicator)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(indicator);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, MISC_0, SMA_J3_SEL_MASK, (uint32_t *) indicator);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_sysref_src_set(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sysref_source_e sysref_src)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(!((sysref_src == ADI_FPGA_APOLLO_SYSREF_SRC_AD9528) || (sysref_src == ADI_FPGA_APOLLO_SYSREF_SRC_FMC) || (sysref_src == ADI_FPGA_APOLLO_SYSREF_SRC_EXT)));

    err = adi_fpga_apollo_private_write32_bitfield(fpga, CLK_SRC_SEL, SYSREF_SEL_MASK, sysref_src);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_sysref_src_get(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sysref_source_e *sysref_src)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(sysref_src);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, CLK_SRC_SEL, SYSREF_SEL_MASK, (uint32_t*)sysref_src);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_transmit_skip_data_en(adi_fpga_apollo_device_t *fpga, bool skip)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Set the transmit skip data flag. If set then zeros are transmitted instead of pattern data. */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, CTRL_TRANSMIT,
                    TRANSMIT_SKIP_DATA_MASK, skip ? 1 : 0);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}


int32_t adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(adi_fpga_apollo_device_t* fpga, uint32_t ext_trig_en)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_EN_MASK, ext_trig_en);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_sysref_seq_ext_trig_enable_get(adi_fpga_apollo_device_t* fpga, uint32_t* ext_trig_en)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(ext_trig_en);

    err = adi_fpga_apollo_private_read32_bitfield(fpga, SEQ_CTRL_4, SEQ_EXT_TRIG_EN_MASK, (uint32_t *) ext_trig_en);
    ADI_CMS_ERROR_RETURN(err);
    return err;
}

int32_t adi_fpga_apollo_core_platform_identify(adi_fpga_apollo_device_t* fpga)
{

    int32_t err = API_CMS_ERROR_OK;
    char product_name[FPGA_MAX_NAME_LENGTH];
    char board_name[FPGA_MAX_NAME_LENGTH];
    char design_ext1[FPGA_MAX_NAME_LENGTH];
    char design_ext2[FPGA_MAX_NAME_LENGTH];

    ADI_CMS_NULL_PTR_CHECK(fpga);

    for (int i = 0; i < 4; i++) {
        err = adi_fpga_apollo_core_reg_get(fpga, PRODUCT_NAME + i, (uint32_t *) &product_name[4*i]);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_reg_get(fpga, BOARD_NAME + i, (uint32_t *) &board_name[4*i]);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_reg_get(fpga, DESIGN_EXT1 + i, (uint32_t *) &design_ext1[4*i]);
        ADI_CMS_ERROR_RETURN(err);
        err = adi_fpga_apollo_core_reg_get(fpga, DESIGN_EXT2 + i, (uint32_t *) &design_ext2[4*i]);
        ADI_CMS_ERROR_RETURN(err);
    }

    if (!strcmp(board_name, "vcu128")) {
        fpga->state_info.design_id = ADI_FPGA_APOLLO_DESIGN_VCU128;
    } else if (!strcmp(design_ext1, "raw")) {
        fpga->state_info.design_id = ADI_FPGA_APOLLO_DESIGN_RAW_EVAL;
    } else if (!strcmp(design_ext1, "customer") && !(strcmp(design_ext2, "dual_multi"))) {
        fpga->state_info.design_id = ADI_FPGA_APOLLO_DESIGN_MM_REF;
    } else if (!strcmp(design_ext1, "mcs")) {
        fpga->state_info.design_id = ADI_FPGA_APOLLO_DESIGN_MCS;
    } else {
        fpga->state_info.design_id = ADI_FPGA_APOLLO_DESIGN_STD_EVAL;
    }

    return err;
}

int32_t adi_fpga_apollo_core_jrx_link_error_count_get(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint32_t* sh_err_count, uint32_t* emb_err_count)
{

    int32_t err = API_CMS_ERROR_OK;
    uint32_t jrx_link_pd;
    uint32_t sh_err = 0, emb_err = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(sh_err_count);
    ADI_CMS_NULL_PTR_CHECK(emb_err_count);

    ADI_CMS_CHECK((fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_STD_EVAL), API_CMS_ERROR_NOT_SUPPORTED);

    err = fpga->hal_info.reg_read(JESD204B_RX_PWR_CTRL, &jrx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    for (int i = 0; i < MAX_JESD_LINKS; i++) {
        if ((link_sel & ~jrx_link_pd) & (ADI_FPGA_APOLLO_LINK_0 << i)) {
            err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204C_RX_LINK_ERROR_CNT + i, JESD204C_RX_SH_ERR_CNT, &sh_err);
            ADI_CMS_ERROR_RETURN(err);
            *sh_err_count += sh_err;

            err = adi_fpga_apollo_private_read32_bitfield(fpga, JESD204C_RX_LINK_ERROR_CNT + i, JESD204C_RX_EMB_ERR_CNT, &emb_err);
            ADI_CMS_ERROR_RETURN(err);
            *emb_err_count += emb_err;
        }
    }

    return err;
}

int32_t adi_fpga_apollo_core_jrx_link_error_cnt_reset(adi_fpga_apollo_device_t* fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t jrx_link_pd;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    ADI_CMS_CHECK((fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_STD_EVAL), API_CMS_ERROR_NOT_SUPPORTED);

    err = fpga->hal_info.reg_read(JESD204B_RX_PWR_CTRL, &jrx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204C_RX_LINK_ERROR_CNT_RESET, JESD204C_RX_SH_ERR_CNT_RESET, 0xf);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, JESD204C_RX_LINK_ERROR_CNT_RESET, JESD204C_RX_EMB_ERR_CNT_RESET, 0xf);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_max_mem_size_check(adi_fpga_apollo_device_t *fpga, uint64_t size_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    ADI_CMS_NULL_PTR_CHECK(fpga);

    if (fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_MM_REF || fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_VCU128) {
        if (size_bytes > FPGA_MULTI_MODE_MEM_SIZE) {
            printf("WARNING: Number of bytes exceeds FPGA memory size\n");
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }
    } else {
        if (size_bytes > FPGA_STD_MEM_SIZE) {
            printf("WARNING: Number of bytes exceeds FPGA memory size\n");
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_INVALID_PARAM);
        }
    }

    return err;
}

int32_t adi_fpga_apollo_core_spi2_sdo_alt_enable_set(adi_fpga_apollo_device_t* fpga, uint8_t enable)
{

    int32_t err = API_CMS_ERROR_OK;
    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = adi_fpga_apollo_private_write32_bitfield(fpga, SPI_MASTER_CTRL, FMCB_GPIO_SDO_EN, (uint32_t) enable);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_core_bidir_init(adi_fpga_apollo_device_t* fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t jtx_link_pd, jrx_link_pd;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    err = fpga->hal_info.reg_read(JESD204B_TX_PWR_CTRL, &jtx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.reg_read(JESD204B_RX_PWR_CTRL, &jrx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_rx_keep_link_mask_set(fpga, 0x0);   /* Do stop or re-start the FPGA jrx link before init */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_tx_keep_link_mask_set(fpga, 0x0);   /* Do stop or re-start the FPGA jtx link before init */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_skip_rx_link_init_set(fpga, 0);     /* Clear skip-init to allow Tx init in bdir_start */
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_capture_size_set(fpga, 1);          /* Capture size is 1 x 64K bytes */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_bidir_start(fpga);                  /* Init the tx and rx links - sends 3 triggers */
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.delay_us(fpga, 100000);                    /* Allow FPGA state to advance */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_not_running_wait(fpga);              /* Wait for rx not running */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_skip_rx_link_init_set(fpga, 0x1);     /* Enable skip-init to prevent Tx link from reinitializing during capture_start. */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_tx_keep_link_mask_set(fpga, (~jtx_link_pd & 0xf));  /* GroupA jtx main link, don't stop or re-start the link after init */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_rx_keep_link_mask_set(fpga, (~jrx_link_pd & 0xf));  /* GroupA jrx main link, don't stop or re-start the link after init */
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_rx_links_init(adi_fpga_apollo_device_t* fpga)
{
    int32_t err;
    uint32_t fpga_reg32, is_link_up, jrx_link_pd;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /* Skip setup if link is up */
    if (fpga->state_info.jrx[0].jesd_jesdv == 1) {
        adi_fpga_apollo_core_reg_get(fpga, 0x0208, &fpga_reg32);
        is_link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;
        if (is_link_up) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_OK);
        }
    }

    err = adi_fpga_apollo_core_rx_keep_link_mask_set(fpga, 0x0);   /* Do stop or re-start the FPGA jrx link before init */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_skip_rx_link_init_set(fpga, 0);     /* Clear skip-init to allow Tx init in bdir_start */
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.reg_read(JESD204B_RX_PWR_CTRL, &jrx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_capture_size_set(fpga, 1);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_capture_start(fpga);
    ADI_CMS_ERROR_RETURN(err);

    /* Wait for capture to finish */
    err = adi_fpga_apollo_core_capture_complete_wait(fpga);
    ADI_CMS_ERROR_RETURN(err);

    /* Check if link is up */
    if (fpga->state_info.jrx[0].jesd_jesdv == 1) {
        adi_fpga_apollo_core_reg_get(fpga, 0x0208, &fpga_reg32);
        is_link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;
        if (!is_link_up) {
            ADI_CMS_ERROR_RETURN(API_CMS_ERROR_LINK_DOWN);
        }
    }

    err = adi_fpga_apollo_core_skip_rx_link_init_set(fpga, 0x1);     /* Enable skip-init to prevent Tx link from reinitializing during capture_start. */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_rx_keep_link_mask_set(fpga, (~jrx_link_pd & 0xf));  /* GroupA jrx main link, don't stop or re-start the link after init */
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_tx_links_init(adi_fpga_apollo_device_t* fpga)
{
    int32_t err;
    uint32_t jtx_link_pd;

    ADI_CMS_NULL_PTR_CHECK(fpga);

	err = adi_fpga_apollo_core_tx_keep_link_mask_set(fpga, 0x0);   /* Do stop or re-start the FPGA jtx link before init */
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.reg_read(JESD204B_TX_PWR_CTRL, &jtx_link_pd);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_transmit_start(fpga);
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.delay_us(fpga, 100000);                    /* Important delay based on experiments (will confirm) */
    ADI_CMS_ERROR_RETURN(err);

	err = adi_fpga_apollo_core_tx_keep_link_mask_set(fpga, (~jtx_link_pd & 0xf));  /* GroupA jtx main link, don't stop or re-start the link after init */
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_core_sysref_setup(adi_fpga_apollo_device_t *fpga)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    /*
    * Setup FPGA SYSREF clocking
    */
    err = adi_fpga_apollo_private_write32_bitfield(fpga, SYSREF_GLBLCLK_RATIO, SYSREF_GLBLCLK_RATIO_MASK, 32);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, SYSREF_COUNT, SYSREF_GLBLCLK_RATIO_SET_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, SYSREF_COUNT, SYSREF_COUNT_EN_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);
    err = adi_fpga_apollo_private_write32_bitfield(fpga, SYSREF_COUNT, SYSREF_COUNT_START_MASK, 1);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

/* Convert FPGA phy lanes to Apollo phy lanes for ADS10 platform */
static uint32_t jrx_fpga_to_device_phy(uint32_t fpga_prbs_lock)
{
    uint32_t device_prbs_lock = 0;
    for (uint16_t i = 0; i < 24; i++)
    {
        device_prbs_lock |= ((fpga_prbs_lock >> jrx_lane_map[i]) & 0x01) << i;
    }
    return (device_prbs_lock);
}