/*!
 * \brief     ADS10 Apollo examples common function implementation
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include <math.h>
#include <stdlib.h>
#include "adi_apollo.h"
#include "adi_apollo_hal.h"
#include "adi_utils.h"
#include "adi_apollo_bf_custom.h"
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core.h"
#include "adi_fpga_apollo_private.h"
#include "adi_fpga_apollo_fsrc_types.h"
#include "adi_fpga_apollo_capture.h"
#include "adi_apollo_linux_utilities.h"
#include "ads10_fw_provider.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_ads10_apollo_ex_inspect.h"
#include "adi_ads10_apollo_ex_adf4382.h"
#include "adi_ads10_apollo_ex_hmc7044.h"
#include "adi_ads10_apollo_ex_fpga.h"
#include "ads10_fpga.h"
#include "ads10_hal.h"

#ifdef INTERNAL_TESTING
#include "adi_apollo_ty_encl.h"
#endif

#define NUM_JESD_LINKS_BOTH_SIDES ADI_APOLLO_JESD_LINKS * ADI_APOLLO_NUM_SIDES

static int32_t display_version_info(adi_apollo_device_t *device);
static int32_t get_fpga_jtx_params_from_profile(adi_fpga_jesd_param_t fpga_jesd_param[], adi_apollo_jesd_rx_cfg_t jrx[], uint8_t total_links);
static int32_t get_fpga_jrx_params_from_profile(adi_fpga_jesd_param_t fpga_jesd_param[], adi_apollo_jesd_tx_cfg_t jtx[], uint8_t total_links);

static int32_t check_link_status_capture_mem(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint8_t *cap_buffer, uint32_t bytes_to_capture);

static int32_t adi_ads10_apollo_frames_to_files_setup(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_fpga_apollo_capture_frame_t *frame, char *file_name_base, FILE ** file_ptrs[], bool interleaved);
static int32_t adi_ads10_apollo_frames_to_files_write(adi_fpga_apollo_device_t *fpga_device, adi_fpga_apollo_capture_frame_t *frame, FILE * file_ptrs[], bool interleaved);
static int32_t adi_ads10_apollo_cleanup_frames_to_files(adi_fpga_apollo_capture_frame_t *frame, FILE * file_ptrs[], bool interleaved);
static int32_t print_tot_samples_per_conv(adi_fpga_apollo_device_t *fpga_device);
static int32_t hsci_buf_alloc(adi_apollo_device_t *device);

int32_t adi_ads10_apollo_ex_startup(adi_apollo_device_t *device, adi_apollo_top_t *dev_profile)
{
    int32_t err;

    ADI_CMS_NULL_PTR_CHECK(device);
    ADI_CMS_NULL_PTR_CHECK(dev_profile);

    err = adi_apollo_startup_execute(device, dev_profile, ADI_APOLLO_STARTUP_SEQ_DEFAULT);
    ADI_APOLLO_ERROR_RETURN(err);
    
    err = display_version_info(device);
    if (err != API_CMS_ERROR_OK) {
        printf("Error displaying version info. This may indicate device clock is incorrect.\n");
        return err;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_print_regs(adi_apollo_device_t *device, uint32_t reg, uint32_t nregs)
{
    int32_t err, i;
    uint8_t data8;

    for (i = 0; i < nregs; i++) {
        if (err = adi_apollo_hal_reg_get(device, reg + i, &data8), err != API_CMS_ERROR_OK) {
            printf("*error*\n");
            return err;
        }
        printf("0x%08X: 0x%02X\n", reg + i, data8);
        if ((i+1)%4 == 0) printf("\n");
    }
    printf("\n");

    return API_CMS_ERROR_OK;

}

uint32_t adi_ads10_apollo_ex_fpga_print_regs(adi_fpga_apollo_device_t *fpga_device, uint32_t reg, uint32_t nregs)
{
    int32_t err, i;
    uint32_t data32;

    for (i = 0; i < nregs; i++) {
        if (err = adi_fpga_apollo_core_reg_get(fpga_device, reg + i, &data32), err != API_CMS_ERROR_OK) {
            printf("*error*\n");
            return err;
        }
        printf("0x%08X: 0x%08X\n", reg + i, data32);
        if ((i + 1) % 4 == 0) printf("\n");
    }
    printf("\n");

    return API_CMS_ERROR_OK;
}



int32_t adi_ads10_apollo_ex_reg_test(adi_apollo_device_t *device)
{
    int32_t err;
    uint32_t i, data32;
    uint8_t data8, stat;
    adi_apollo_hal_protocol_e protocol;

    adi_apollo_hal_active_protocol_get(device, &protocol);

    /* Direct register SPI scratch loop rd/wr test */
    stat = 0;
    uint32_t direct_addr[] = { 0x4700000a, 0x4700000a, 0x47000200, 0x47000200 };    // HSCI can't write 0x4700000A, SPI only
    uint8_t  direct_data[] = { 0x55,       0xaa,       0xcc,       0x33 };

    for (i=0; i<sizeof(direct_addr) / sizeof(direct_addr[0]); i++) {

        if ((protocol == ADI_APOLLO_HAL_PROTOCOL_HSCI) && (direct_addr[i] <= 0x4700000Fu)) {
            continue;
        }

        if (err = adi_apollo_hal_reg_set(device, direct_addr[i], direct_data[i]), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (err = adi_apollo_hal_reg_get(device, direct_addr[i], &data8), err != API_CMS_ERROR_OK) {
            return err;
        }

        if (data8 != direct_data[i]) {
            stat = 1;
            break;
        }

    }

    printf("Test direct register %s\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    /* Indirect register SPI loop rd/wr test */
    stat = 0;
    uint32_t indirect_addr[] = { INDIRECT_REG_TEST_ADDR + 0, INDIRECT_REG_TEST_ADDR + 1, 
                                 INDIRECT_REG_TEST_ADDR + 2, INDIRECT_REG_TEST_ADDR + 3 }; /* indirect register address */
    uint8_t  indirect_data[] = { 0x12,       0x34,       0x56,       0x78 };
    for (i = 0; i < sizeof(indirect_addr) / sizeof(indirect_addr[0]); i++) {
        if (err = adi_apollo_hal_reg_set(device, indirect_addr[i], indirect_data[i]), err != API_CMS_ERROR_OK) {
            return err;
        }
        if (err = adi_apollo_hal_reg_get(device, indirect_addr[i], &data8), err != API_CMS_ERROR_OK) {
            return err;
        }

        if (data8 != indirect_data[i]) {
        stat = 1;
        break;
        }
    }

    printf("Test indirect register %s\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    /* 32-bit ARM mem rd/wr test */
    stat = 0;
    uint32_t arm_addr[] = { ARM_REG_TEST_BASE_ADDR + 0, ARM_REG_TEST_BASE_ADDR + 4, 
                            ARM_REG_TEST_BASE_ADDR + 8, ARM_REG_TEST_BASE_ADDR + 12 }; /* ARM core1 register addresses */
    uint32_t arm_data[] = { 0x55aa55aa, 0xdeadbeef, 0xbeefdead, 0xaa55aa55 };
    for (i = 0; i < sizeof(arm_addr) / sizeof(arm_addr[0]); i++) {
       if (err = adi_apollo_hal_reg32_set(device, arm_addr[i], arm_data[i]), err != API_CMS_ERROR_OK) {
           return err;
       }
       if (err = adi_apollo_hal_reg32_get(device, arm_addr[i], &data32), err != API_CMS_ERROR_OK) {
           return err;
       }

       if (data32 != arm_data[i]) {
           stat = 1;
           break;
       }
    }

    printf("Test ARM memory %s\n", (0 == stat) ? "Passed" : "*** FAILED ***");
    if (stat != 0) {
        return API_CMS_ERROR_ERROR;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_fpga_reg_test(adi_fpga_apollo_device_t *fpga_device)
{
    int32_t err;
    uint32_t out_data = 0, offset = 0;

    offset = 0x100;
    if (err = adi_fpga_apollo_core_reg_get(fpga_device, offset, &out_data), err != API_CMS_ERROR_OK) {
        return err;
    }
    printf("FPGA reg at offset 0x%X value is 0x%04X\n", offset, out_data);

    offset = 0x101;
    if (err = adi_fpga_apollo_core_reg_get(fpga_device, offset, &out_data), err != API_CMS_ERROR_OK) {
        return err;
    }
    printf("FPGA reg at offset 0x%X value is 0x%04X\n", offset, out_data);

    return API_CMS_ERROR_OK;
}

/* Configure the ADS10 FPGA JESD Rx and Tx Links */
int32_t adi_ads10_apollo_ex_fpga_jesd_configure(adi_fpga_apollo_device_t *fpga_device, adi_apollo_jesd_tx_cfg_t jtx[], adi_apollo_jesd_rx_cfg_t jrx[])
{
    int32_t err;
    adi_fpga_jesd_param_t fpga_jesd_param[NUM_JESD_LINKS_BOTH_SIDES];

    /* Identify FPGA platform */
    adi_fpga_apollo_core_platform_identify(fpga_device);

    printf("FPGA Design: %d\n", fpga_device->state_info.design_id);

    /* Configure number of links based on the FPGA Image loaded */
    err = adi_fpga_apollo_core_jtx_link_cnt_get(fpga_device, &fpga_device->state_info.jtx_link_count);
    if (err != 0) {
        printf("adi_fpga_apollo_core_jtx_link_cnt_get() failed: %d\n", err);
    }
    err = adi_fpga_apollo_core_jrx_link_cnt_get(fpga_device, &fpga_device->state_info.jrx_link_count);
    if (err != 0) {
        printf("adi_fpga_apollo_core_jrx_link_cnt_get() failed: %d\n", err);
    }

    /* Config fpga jesd Rx params from the Apollo jtx profile (ADC) */
    err = get_fpga_jrx_params_from_profile(fpga_jesd_param, jtx, NUM_JESD_LINKS_BOTH_SIDES);
    if (err != 0) {
        printf("get_fpga_jrx_params_from_profile() failed: %d\n", err);
        return err;
    }
    /* Configure FPGA JRx links (for ADC cap samples) */
    err = adi_fpga_apollo_core_jesd_rx_config(fpga_device, fpga_jesd_param, NUM_JESD_LINKS_BOTH_SIDES);
    if (err != 0) {
        printf("adi_fpga_apollo_core_jesd_rx_config() failed: %d\n", err);
        return err;
    }

    /* Config fpga jesd Tx params from the Apollo jrx profile (DAC) */
    err = get_fpga_jtx_params_from_profile(fpga_jesd_param, jrx, NUM_JESD_LINKS_BOTH_SIDES);
    if (err != 0) {
        printf("get_fpga_jtx_params_from_profile() failed: %d\n", err);
        return err;
    }
    /* Configure FPGA JTx Links (for DAC vec samples)*/
    err = adi_fpga_apollo_core_jesd_tx_config(fpga_device, fpga_jesd_param, NUM_JESD_LINKS_BOTH_SIDES);
    if (err != 0) {
        printf("adi_fpga_apollo_core_jesd_tx_config() failed: %d\n", err);
        return err;
    }

    /* Set FPGA JTx Drive CTRL/Pre Cursor/Post Cursor */
#ifdef VCU128
    err = adi_ads10_apollo_ex_fpga_tx_phy_drive_config_all(fpga_device, 0x1E, 0x0, 0x0); // Default values for VCU128
    ADI_CMS_ERROR_RETURN(err);
#endif

#ifdef ADS10
    err = adi_ads10_apollo_ex_fpga_tx_phy_drive_config_all(fpga_device, 0x1C, 0x0C, 0x00); // Default values for ADS10
    ADI_CMS_ERROR_RETURN(err);
#endif

    return API_CMS_ERROR_OK;
}

static int32_t get_fpga_jtx_params_from_profile(adi_fpga_jesd_param_t fpga_jesd_param[],
                                                adi_apollo_jesd_rx_cfg_t jrx[],
                                                uint8_t total_links)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t sideIdx = 0, linkIdx =0;

    for(int link=0; link< total_links; link++) {
        sideIdx = link/2;
        linkIdx = link%2;
        fpga_jesd_param[link].jesd_l = jrx[sideIdx].rx_link_cfg[linkIdx].l_minus1 + 1;
        fpga_jesd_param[link].jesd_m = jrx[sideIdx].rx_link_cfg[linkIdx].m_minus1 + 1;
        fpga_jesd_param[link].jesd_f = jrx[sideIdx].rx_link_cfg[linkIdx].f_minus1 + 1;
        fpga_jesd_param[link].jesd_s = jrx[sideIdx].rx_link_cfg[linkIdx].s_minus1 + 1;
        fpga_jesd_param[link].jesd_n = jrx[sideIdx].rx_link_cfg[linkIdx].n_minus1 + 1;
        fpga_jesd_param[link].jesd_np = jrx[sideIdx].rx_link_cfg[linkIdx].np_minus1 + 1;
        fpga_jesd_param[link].jesd_k = jrx[sideIdx].rx_link_cfg[linkIdx].k_minus1 + 1;
        fpga_jesd_param[link].jesd_e = jrx[sideIdx].rx_link_cfg[linkIdx].e_minus1 + 1;
        fpga_jesd_param[link].jesd_scr = jrx[sideIdx].rx_link_cfg[linkIdx].scr;
        fpga_jesd_param[link].jesd_cs = jrx[sideIdx].rx_link_cfg[linkIdx].cs;
        fpga_jesd_param[link].jesd_subclass = jrx[sideIdx].common_link_cfg.subclass;
        fpga_jesd_param[link].jesd_jesdv = jrx[sideIdx].common_link_cfg.ver;
        fpga_jesd_param[link].jesd_dual_link = jrx[sideIdx].common_link_cfg.dual_link;
        fpga_jesd_param[link].jesd_hd = jrx[sideIdx].rx_link_cfg[linkIdx].high_dens;
        fpga_jesd_param[link].jesd_cf = 0;
        fpga_jesd_param[link].jesd_mode_s_sel = jrx[sideIdx].rx_link_cfg[linkIdx].quick_mode_id;
        fpga_jesd_param[link].jesd_link_pd = !(jrx[sideIdx].rx_link_cfg[linkIdx].link_in_use);
        fpga_jesd_param[link].jesd_ns = jrx[sideIdx].rx_link_cfg[linkIdx].ns_minus1 + 1;
    }
    return err;
}

static int32_t get_fpga_jrx_params_from_profile(adi_fpga_jesd_param_t fpga_jesd_param[],
                                                adi_apollo_jesd_tx_cfg_t jtx[],
                                                uint8_t total_links)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t sideIdx = 0, linkIdx =0;

    for(int link=0; link< total_links; link++) {
        sideIdx = link/2;
        linkIdx = link%2;
        fpga_jesd_param[link].jesd_l = jtx[sideIdx].tx_link_cfg[linkIdx].l_minus1 + 1;
        fpga_jesd_param[link].jesd_m = jtx[sideIdx].tx_link_cfg[linkIdx].m_minus1 + 1;
        fpga_jesd_param[link].jesd_f = jtx[sideIdx].tx_link_cfg[linkIdx].f_minus1 + 1;
        fpga_jesd_param[link].jesd_s = jtx[sideIdx].tx_link_cfg[linkIdx].s_minus1 + 1;
        fpga_jesd_param[link].jesd_n = jtx[sideIdx].tx_link_cfg[linkIdx].n_minus1 + 1;
        fpga_jesd_param[link].jesd_np = jtx[sideIdx].tx_link_cfg[linkIdx].np_minus1 + 1;
        fpga_jesd_param[link].jesd_k = jtx[sideIdx].tx_link_cfg[linkIdx].k_minus1 + 1;
        fpga_jesd_param[link].jesd_e = jtx[sideIdx].tx_link_cfg[linkIdx].e_minus1 + 1;
        fpga_jesd_param[link].jesd_scr = jtx[sideIdx].tx_link_cfg[linkIdx].scr;
        fpga_jesd_param[link].jesd_cs = jtx[sideIdx].tx_link_cfg[linkIdx].cs;
        fpga_jesd_param[link].jesd_subclass = jtx[sideIdx].common_link_cfg.subclass;
        fpga_jesd_param[link].jesd_jesdv = jtx[sideIdx].common_link_cfg.ver;
        fpga_jesd_param[link].jesd_dual_link = jtx[sideIdx].common_link_cfg.dual_link;
        fpga_jesd_param[link].jesd_hd = jtx[sideIdx].tx_link_cfg[linkIdx].high_dens;
        fpga_jesd_param[link].jesd_cf = 0;
        fpga_jesd_param[link].jesd_mode_s_sel = jtx[sideIdx].tx_link_cfg[linkIdx].quick_mode_id;
        fpga_jesd_param[link].jesd_link_pd = !(jtx[sideIdx].tx_link_cfg[linkIdx].link_in_use);
        fpga_jesd_param[link].jesd_ns = jtx[sideIdx].tx_link_cfg[linkIdx].ns_minus1 + 1;
    }
    return err;
}


int32_t adi_ads10_apollo_ex_configure_hal(adi_apollo_device_t *apollo_device, adi_fpga_apollo_hal_config_t *ads10_platform, uint8_t enable_hsci)
{  
    adi_apollo_hal_t *hal = &apollo_device->hal_info;
    adi_apollo_hal_regio_spi_desc_t *spi0_desc = &hal->spi0_desc;
    adi_apollo_hal_regio_spi_desc_t *spi1_desc = &hal->spi1_desc;
    adi_apollo_hal_regio_hsci_desc_t *hsci_desc = &hal->hsci_desc;
    hal->dev_hal_info = ads10_platform;
    hal->log_write = &ads10_log_write;
    hal->delay_us = &ads10_wait_us;
    hal->reset_pin_ctrl = &ads10_hw_rst_pin_ctrl_apollo;

    /* SPI 0 Config */
    spi0_desc->is_used = 1;
    spi0_desc->spi_config.addr_inc = ADI_APOLLO_DEVICE_SPI_ADDR_INC_AUTO;
    spi0_desc->spi_config.msb = ADI_APOLLO_DEVICE_SPI_MSB_FIRST;
    spi0_desc->spi_config.sdo = ADI_APOLLO_DEVICE_SPI_SDO;
    spi0_desc->dev_obj = ads10_platform->spi0;
    spi0_desc->xfer = NULL;
    spi0_desc->read = &ads10_spi_read;
    spi0_desc->write = &ads10_spi_write;
    spi0_desc->poll_read = NULL;
    spi0_desc->rd_stream_en = 1;    /* 1: api gens stream rd txns, 0: no-streaming */
    spi0_desc->wr_stream_en = 1;    /* 1: api gens stream wr txns, 0: no-streaming */

    ads10_apollo_hal_config_data(apollo_device, 0, 1);     /* chip selects: SPI0=0, SPI1=1 */

    /* SPI1 config - not used */
    spi1_desc->is_used = 0;

    /* HSCI config */
    if (enable_hsci == 1) {
        hsci_desc->is_used = 1;
        hsci_desc->hsci_config.auto_linkup_en = 1;
        hsci_desc->auto_linkup = &ads10_hsci_auto_linkup;
        hsci_desc->alink_tbl_get = &ads10_hsci_alink_tbl_get;
        hsci_desc->manual_linkup = &ads10_hsci_manual_linkup;
        hsci_desc->read = &ads10_hsci_read;
        hsci_desc->write = &ads10_hsci_write;
        hsci_desc->poll_read = NULL;
        
#if defined(ADI_MIN_STACK_ALLOC)
        ADI_CMS_ERROR_RETURN(hsci_buf_alloc(apollo_device));
#endif
    }


    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_configure_startup(adi_apollo_device_t *apollo_device)
{
    adi_apollo_startup_t *startup_info = &apollo_device->startup_info;

    /* 
     * Configure the startup info. A FW provider is used by adi_apollo_startup_execute() to obtain
     * firmware binaries in a platform agnostic manner. 
    */
    startup_info->fw_provider = ads10_fw_provider_create(apollo_device, FW_IMAGES_DIR);
    startup_info->open = ads10_fw_provider_open;   // Optional. Called before obtaining a FW binary. Set to NULL if unused.
    startup_info->close = ads10_fw_provider_close; // Optional. Called after obtaining a FW binary. Set to NULL if unused.
    startup_info->get = ads10_fw_provider_get;     // Required. Used by adi_apollo_startup_execute() to get FW bytes.

    return API_CMS_ERROR_OK;
}


int32_t adi_ads10_apollo_ex_configure_fpga_hal(adi_fpga_apollo_device_t *fpga_device, adi_fpga_apollo_hal_config_t *ads10_platform)
{
    adi_apollo_fpga_hal_t *hal = &fpga_device->hal_info;

    /* Assign the read and write ADS10 FPGA HAL implementations */
    hal->reg_read = &ads10_axi_reg_read32;
    hal->reg_write = &ads10_axi_reg_write32;
    hal->delay_us = &ads10_wait_us;
    hal->mem_write = &ads10_fpga_mem_write;
    hal->mem_read = &ads10_fpga_mem_read;

    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_fpga_pre_reset(adi_fpga_apollo_device_t *fpga_device)
{
    int32_t err = API_CMS_ERROR_OK;

    adi_fpga_apollo_private_write32_bitfield(fpga_device, SEQ_CTRL_2, SEQ_FIRST_TRIG_CNT_MASK, 1);
    adi_fpga_apollo_private_write32_bitfield(fpga_device, SEQ_CTRL_2, SEQ_SECOND_TRIG_CNT_MASK, 1);     // stops trig out after fpga load (trigger is blasted continuously after loading image)
    
    adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(fpga_device, 0);    // Disable the sysref seq ext trig

    return err;
}
static int32_t display_version_info(adi_apollo_device_t *device)
{
    int32_t err;
    uint16_t maj, min, rc;
    adi_apollo_mailbox_resp_get_fw_version_t fw_ver;
    adi_apollo_mailbox_cmd_ping_t ping_cmd;
    adi_apollo_mailbox_resp_ping_t ping_resp;

    if (err = adi_apollo_device_api_revision_get(device, &maj, &min, &rc), err != API_CMS_ERROR_OK) {
        printf("API revision get error\n");
        return err;
    }
    printf("API ver: %d.%d.%d\n", maj, min, rc);
    adi_apollo_hal_log_write(device, ADI_CMS_LOG_API, "API ver: %d.%d.%d", maj, min, rc);

    if (err = adi_apollo_mailbox_ready_check(device), err != API_CMS_ERROR_OK) {
        printf("Mailbox Ready error\n");
        return err;
    }

    ping_cmd.echo_data = 0x00000000; /* Ping core 1 (echo_data < 0x10000000, increment by 1) */
    if (err = adi_apollo_mailbox_ping(device, &ping_cmd, &ping_resp), err != API_CMS_ERROR_OK) {
        printf("Mailbox ping (core 1) error\n");
        return err;
    }
    printf("Ping (core 1) cmd/res: 0x%08x/0x%08x\n", ping_cmd.echo_data, ping_resp.echo_data);

    ping_cmd.echo_data = 0x12345678; /* Ping core 0 (echo_data >= 0x10000000, increment by 2) */
    if (err = adi_apollo_mailbox_ping(device, &ping_cmd, &ping_resp), err != API_CMS_ERROR_OK) {
        printf("Mailbox ping (core 0) error\n");
        return err;
    }
    printf("Ping (core 0) cmd/res: 0x%08x/0x%08x\n", ping_cmd.echo_data, ping_resp.echo_data);

    if (err = adi_apollo_mailbox_get_fw_version(device, &fw_ver), err != API_CMS_ERROR_OK) {
        printf("Mailbox fw error\n");
        return err;
    }
    printf("FW ver: %04d%02d%02d.%d.%d\n", fw_ver.year, fw_ver.month, fw_ver.day, fw_ver.minor, fw_ver.build);
    adi_apollo_hal_log_write(device, ADI_CMS_LOG_API, "FW ver: %04d%02d%02d.%d.%d\n", fw_ver.year, fw_ver.month, fw_ver.day, fw_ver.minor, fw_ver.build);


    return API_CMS_ERROR_OK;
}

int32_t adi_ads10_apollo_ex_fpga_capture(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_fpga_apollo_device_t *fpga_device, uint32_t num_samples, char *file_name_base, bool interleaved)
{
    int32_t err;
    uint32_t current_transfer_count;
    adi_fpga_apollo_capture_frame_t cap_frame_info;         /* Struct defining how a capture samples are packed */
    uint32_t max_transf = 0;                                /* Max number of Bytes to transfer at a time */
    FILE ** fp;

    printf("User requested a minimum of %d samples to be captured per virtual converter\n", num_samples);

    /* Setup transfer of capture. */
    err = adi_fpga_apollo_capture_transfer_setup(fpga_device, &num_samples, &max_transf);
    ADI_CMS_ERROR_GOTO(err, end);

    /* Execute capture, data will be in FPGA memory */
    err = adi_fpga_apollo_capture_execute(fpga_device, num_samples);
    ADI_CMS_ERROR_GOTO(err, end1);
    printf("Capture Done\n");

    /* Get the capture frame info */
    err = adi_fpga_apollo_capture_frame_info_get(fpga_device, &cap_frame_info);
    ADI_CMS_ERROR_RETURN(err);

    /* Print the number of samples captured per virtual converter for each link */
    err = print_tot_samples_per_conv(fpga_device);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_ads10_apollo_frames_to_files_setup(device, profile, &cap_frame_info, file_name_base, &fp, interleaved);
    ADI_CMS_ERROR_GOTO(err, end);

    printf("Starting transfer of %lld bytes...\n", fpga_device->state_info.capture_info.fifo_queue_bytes);

    current_transfer_count = max_transf;
    while (fpga_device->state_info.capture_info.fifo_queue_bytes > 0) {

        if (current_transfer_count > fpga_device->state_info.capture_info.fifo_queue_bytes) {
            current_transfer_count = fpga_device->state_info.capture_info.fifo_queue_bytes;
        }

        /* Get the data from the FPGA */
        err = adi_fpga_apollo_capture_data_get(fpga_device, current_transfer_count);
        ADI_CMS_ERROR_GOTO(err, end);

        err = adi_ads10_apollo_frames_to_files_write(fpga_device, &cap_frame_info, fp, interleaved);
        ADI_CMS_ERROR_GOTO(err, end);
    }

    printf("Transfer complete\n");

end:
    adi_ads10_apollo_cleanup_frames_to_files(&cap_frame_info, fp, interleaved);
end1:    
    adi_fpga_apollo_capture_transfer_cleanup(fpga_device);

    return err;
}

static int32_t adi_ads10_apollo_frames_to_files_setup(adi_apollo_device_t *device, adi_apollo_top_t *profile, adi_fpga_apollo_capture_frame_t *frame, char *file_name_base, FILE ** file_ptrs[], bool interleaved){
    int32_t err = API_CMS_ERROR_OK;
    uint32_t file_count= 0;
    char fname[MAX_PATH_LEN];
    FILE ** fps;
    for (int i = 0; i<MAX_JESD_LINKS; i++) {
        if (interleaved && (frame->link_converter_count[i] % 2 != 1)) {
            file_count += frame->link_converter_count[i] / 2;
	} else {
            file_count += frame->link_converter_count[i];
	}
    }

    fps = (FILE**)malloc(sizeof(FILE*)*file_count);
    if (fps == NULL) {
        printf("Memory allocation error\n");
        return API_CMS_ERROR_MEM_ALLOC;
    }

    file_count = 0;

    adi_ads10_apollo_rx_channel_info_t channel;
    adi_ads10_apollo_ex_inspect_rx_channel_get(device, profile, &channel);

    for (int i = 0; i<MAX_JESD_LINKS;i++){
        if (interleaved && (frame->link_converter_count[i] % 2 != 1)) {
            for (int j = 0; j<frame->link_converter_count[i] / 2; j++) {
                snprintf(fname, MAX_PATH_LEN, "%s/%s_L%d_IQ%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, file_name_base, i, j, frame->link_bits_per_sample[i], channel.data_rate_hz, channel.nco_freq_hz);
                fps[file_count] = fopen(fname,"w+");

                if (fps[file_count] == NULL) {
                    err = API_CMS_ERROR_FILE_OPEN;
                }
                file_count++;
            }
        } else {
            for (int j = 0; j<frame->link_converter_count[i]; j++) {
                snprintf(fname, MAX_PATH_LEN, "%s/%s_L%d_m%d_NP%d_DR%lld_CF%lld.txt", OUTPUT_DIR, file_name_base, i, j, frame->link_bits_per_sample[i], channel.data_rate_hz, channel.nco_freq_hz);
                fps[file_count] = fopen(fname,"w+");

                if (fps[file_count] == NULL) {
                    err = API_CMS_ERROR_FILE_OPEN;
                }
                file_count++;
            }
        }
    }
    *file_ptrs = fps;

    return err;
}

static int32_t adi_ads10_apollo_frames_to_files_write(adi_fpga_apollo_device_t *fpga_device, adi_fpga_apollo_capture_frame_t *frame, FILE * file_ptrs[], bool interleaved) {

    int32_t err = API_CMS_ERROR_OK;
    uint64_t idx = 0;
    uint32_t fp_tracker = 0;
    uint32_t link_idx, vc_idx;
    uint32_t num_samples_vc_i = 0, num_samples_vc_q = 0;
    int16_t sample;
    adi_fpga_apollo_capture_t *capture_info = &fpga_device->state_info.capture_info;

    /* Iterate through links */
    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        if (capture_info->capture_frame.link_enabled[link_idx] == 0) {
            continue;
        }
        if (interleaved && (frame->link_converter_count[link_idx] % 2 != 1)) {
            /* if interleaving, iterate by pairs of vc's */
            for (vc_idx = 0; vc_idx < frame->link_converter_count[link_idx]; vc_idx+=2 ) {
                /* get size of virtual conv trnasfer buffers */
                err = adi_fpga_apollo_capture_transf_buff_num_samples_get(fpga_device, 0x1 << link_idx, vc_idx, &num_samples_vc_i);
                ADI_CMS_ERROR_RETURN(err);
                err = adi_fpga_apollo_capture_transf_buff_num_samples_get(fpga_device, 0x1 << link_idx, vc_idx + 1, &num_samples_vc_q);
                ADI_CMS_ERROR_RETURN(err); 
                if (num_samples_vc_i != num_samples_vc_q) {
                    printf("Error: I and Q buffer sizes do not match\n");
                    return API_CMS_ERROR_INVALID_PARAM;
                }
                /* write captured samples to files */
                if (capture_info->capture_frame.link_bits_per_sample[link_idx] == 8) {
                    while (idx < num_samples_vc_i) {
                        fprintf(file_ptrs[fp_tracker], "%d\n", capture_info->cap_transf_buff[link_idx][vc_idx][idx]);
                        fprintf(file_ptrs[fp_tracker], "%d\n", capture_info->cap_transf_buff[link_idx][vc_idx + 1][idx]);
                        idx++;
                    }       
                } else {
                    while (idx < num_samples_vc_i) {
                        sample = capture_info->cap_transf_buff[link_idx][vc_idx][idx*2 + 1] << 8 | capture_info->cap_transf_buff[link_idx][vc_idx][idx*2];
                        fprintf(file_ptrs[fp_tracker], "%d\n", sample);
                        sample = capture_info->cap_transf_buff[link_idx][vc_idx + 1][idx*2 + 1] << 8 | capture_info->cap_transf_buff[link_idx][vc_idx + 1][idx*2]; 
                        fprintf(file_ptrs[fp_tracker], "%d\n", sample);
                        idx++;
                    }
                }
                idx = 0;
                fp_tracker++;
            }
        } else {
            /* if not interleaving, iterate through each vc */
            for (vc_idx = 0; vc_idx < frame->link_converter_count[link_idx]; vc_idx++) {
                /* get size of virtual conv transfer buffers */
                err = adi_fpga_apollo_capture_transf_buff_num_samples_get(fpga_device, 0x1 << link_idx, vc_idx, &num_samples_vc_i);
                ADI_CMS_ERROR_RETURN(err);
                /* write captured samples to files */
                if (capture_info->capture_frame.link_bits_per_sample[link_idx] == 8) {
                    while (idx < num_samples_vc_i) {
                        fprintf(file_ptrs[fp_tracker], "%d\n", capture_info->cap_transf_buff[link_idx][vc_idx][idx]);
                        idx++;
                    }
                } else {
                    while (idx < num_samples_vc_i) {
                        fprintf(file_ptrs[fp_tracker], "%d\n", capture_info->cap_transf_buff[link_idx][vc_idx][idx*2 + 1] << 8 | capture_info->cap_transf_buff[link_idx][vc_idx][idx*2]);
                        idx++;
                    }
                }
                idx = 0;
                fp_tracker++;
            }
        }
    }

    return err;
}

static int32_t adi_ads10_apollo_cleanup_frames_to_files(adi_fpga_apollo_capture_frame_t *frame, FILE * file_ptrs[], bool interleaved) {
    int32_t err = API_CMS_ERROR_OK;
    uint32_t file_count = 0;

    for (int i = 0; i<MAX_JESD_LINKS; i++) {
        if (interleaved && (frame->link_converter_count[i] % 2 != 1))
            file_count += frame->link_converter_count[i] /  2;
        else
            file_count += frame->link_converter_count[i];
    }

    for (int i = 0; i<file_count; i++) {
        if (file_ptrs[i] != NULL)
            fclose(file_ptrs[i]);
    }

    free(file_ptrs);

    return err;
}


int32_t adi_ads10_apollo_ex_fpga_capture_to_array_12b(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint32_t *samples_per_conv, uint16_t *adc_iq_data[], uint32_t num_adcs)
{
    int32_t err;
    uint32_t capture_size = 0x10;
    int16_t cap_sample;

    uint16_t* adc_a0;
    uint16_t* adc_a1;
    uint16_t* adc_b0;
    uint16_t* adc_b1;

    if (err = adi_fpga_apollo_core_capture_size_set(fpga_device, capture_size * 4), err != API_CMS_ERROR_OK) {
        printf("Error from adi_fpga_apollo_core_capture_size_set()\n");
        return err;
    }

    uint32_t bytes_to_capture = capture_size * 64 * 1024 * 3;       // x3 for int multiple of 12-bit samples
    uint8_t* cap_buffer = (uint8_t*)malloc(bytes_to_capture);
    if (cap_buffer == NULL) {
        printf("Memory allocation error\n");
        return API_CMS_ERROR_ERROR;
    }
    memset(cap_buffer, 0x00, bytes_to_capture);

    printf("# bytes to capture = %d\n", bytes_to_capture);

    *samples_per_conv = (bytes_to_capture / 12);              // (bytes * 8bits/byte)/(12bits/sample * 4 vcs * 2 sides)
    adc_a0 = (uint16_t*)malloc(*samples_per_conv * 2 * 2);    // 2 bytes/sample, I/Q interleaved
    adc_a1 = (uint16_t*)malloc(*samples_per_conv * 2 * 2);
    adc_b0 = (uint16_t*)malloc(*samples_per_conv * 2 * 2);
    adc_b1 = (uint16_t*)malloc(*samples_per_conv * 2 * 2);
    adc_iq_data[0] = adc_a0;
    adc_iq_data[1] = adc_a1;
    adc_iq_data[2] = adc_b0;
    adc_iq_data[3] = adc_b1;

    printf("# samples per vc = %d\n", *samples_per_conv);

    if (err = check_link_status_capture_mem(device, fpga_device, cap_buffer, bytes_to_capture), err != API_CMS_ERROR_OK) {
        goto end;
    }

    /* 
     * Create I/Q files for each channel. Data is interleaved [I, Q, I, Q, ...]
     * Note: assumes 4 virtual converters for side (i.e. jesd m = 4)
     */

    uint32_t i = 0;
    uint8_t buf[2][3] = { {0}, {0} };
    uint32_t b_idx[2] = { 0 };
    uint32_t smpl_cnt[2] = { 0 };
    uint8_t side = 0;       // 0=A, 1=B
    uint32_t side_byte_boundary = 512;

    for (i = 0; i < bytes_to_capture; i++) {
        buf[side][b_idx[side]] = cap_buffer[i];

        if ((b_idx[side]+1) % 3 == 0) {
            cap_sample = ((buf[side][1] & 0x0f) << 8) | buf[side][0];
            if (cap_sample & 0x800) { 
                cap_sample |= 0xF000; 
            }
            *(adc_iq_data[(side * 2) + 0] + smpl_cnt[side]/2) = cap_sample;
            smpl_cnt[side]++;

            cap_sample = (buf[side][2] << 4) | (buf[side][1] >> 4);
            if (cap_sample & 0x800) { 
                cap_sample |= 0xF000; 
            }
            *(adc_iq_data[(side * 2) + 1] + smpl_cnt[side]/2) = cap_sample;
            smpl_cnt[side]++;
            
            b_idx[side] = 0;
        } else {
            b_idx[side]++;
        }

        /* switch side if on raw cap side byte boundary */
        if ((i+1) % side_byte_boundary == 0) {
            side = (side == 0) ? 1 : 0;
        }

    }

end:
    free(cap_buffer);

    return err;
}

int32_t adi_ads10_apollo_ex_fpga_capture_to_array(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint32_t* samples_per_conv, uint16_t* adc_iq_data[], uint32_t num_adcs)
{
    int32_t err;
    uint32_t capture_size = 0x10;
    int16_t cap_sample;

    uint16_t* adc_a0;
    uint16_t* adc_a1;
    uint16_t* adc_b0;
    uint16_t* adc_b1;

    if (err = adi_fpga_apollo_core_capture_size_set(fpga_device, capture_size * 4), err != API_CMS_ERROR_OK) {
        printf("Error from adi_fpga_apollo_core_capture_size_set()\n");
        return err;
    }

    uint32_t bytes_to_capture = capture_size * 64 * 1024;
    uint8_t* cap_buffer = (uint8_t*)malloc(bytes_to_capture);
    if (cap_buffer == NULL) {
        printf("Memory allocation error\n");
        return API_CMS_ERROR_ERROR;
    }
    memset(cap_buffer, 0x00, bytes_to_capture);

    printf("# bytes to capture = %d\n", bytes_to_capture);

    *samples_per_conv = ((bytes_to_capture / (4 * 2)));         // 4 vcs * 2 sides
    adc_a0 = (uint16_t*)malloc(*samples_per_conv * 2);          // 2 bytes/sample
    adc_a1 = (uint16_t*)malloc(*samples_per_conv * 2);
    adc_b0 = (uint16_t*)malloc(*samples_per_conv * 2);
    adc_b1 = (uint16_t*)malloc(*samples_per_conv * 2);
    adc_iq_data[0] = adc_a0;
    adc_iq_data[1] = adc_a1;
    adc_iq_data[2] = adc_b0;
    adc_iq_data[3] = adc_b1;

    if (err = check_link_status_capture_mem(device, fpga_device, cap_buffer, bytes_to_capture), err != API_CMS_ERROR_OK) {
        goto end;
    }

    /* 
     * Create I/Q files for each channel. Data is interleaved [I, Q, I, Q, ...]
     * Note: assumes 4 virtual converters for side (i.e. jesd m = 4)
     */
    for (int i = 0; i < bytes_to_capture; i += 4)
    {
        if ((i / 512) % 2 == 0)
        {
            if ((i / 4) % 2 == 0)
            {
                // side A, A0 I&Q
                cap_sample = cap_buffer[i + 0] | (cap_buffer[i + 1] << 8);
                *adc_a0++ = cap_sample;
                cap_sample = cap_buffer[i + 2] | (cap_buffer[i + 3] << 8);
                *adc_a0++ = cap_sample;
            }
            else
            {
                // side A, A1 I&Q
                cap_sample = cap_buffer[i + 0] | (cap_buffer[i + 1] << 8);
                *adc_a1++ = cap_sample;
                cap_sample = cap_buffer[i + 2] | (cap_buffer[i + 3] << 8);
                *adc_a1++ = cap_sample;
            }
        }
        else
        {
            if ((i / 4) % 2 == 0)
            {
                // side B, B0 I&Q
                cap_sample = cap_buffer[i + 0] | (cap_buffer[i + 1] << 8);
                *adc_b0++ = cap_sample;
                cap_sample = cap_buffer[i + 2] | (cap_buffer[i + 3] << 8);
                *adc_b0++ = cap_sample;
            }
            else
            {
                // side B, B1 I&Q
                cap_sample = cap_buffer[i + 0] | (cap_buffer[i + 1] << 8);
                *adc_b1++ = cap_sample;
                cap_sample = cap_buffer[i + 2] | (cap_buffer[i + 3] << 8);
                *adc_b1++ = cap_sample;
            }
        }
    }

end:
    free(cap_buffer);

    return err;
}

double adi_ads10_apollo_ex_fsrc_ratio_get(const adi_apollo_fsrc_cfg_t *fsrc_cfg, int *fsrc_n, int *fsrc_m)
{
    uint32_t fsrc_m32 = 1, fsrc_n32 = 1;

    *fsrc_m = 1;
    *fsrc_n = 1;

    if (fsrc_cfg->enable && !fsrc_cfg->mode_1x) {

        adi_api_utils_ratio_composition(fsrc_cfg->fsrc_rate_int, fsrc_cfg->fsrc_rate_frac_a, fsrc_cfg->fsrc_rate_frac_b, &fsrc_m32, &fsrc_n32, 48);
        *fsrc_m = (int)fsrc_m32;
        *fsrc_n = (int)fsrc_n32;
    }

    return ((double)fsrc_n32) / fsrc_m32;
}

int32_t adi_ads10_apollo_ex_cnco_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t cncos, double rate, double cnco_freq)
{
    int32_t ftw;

    ftw = (cnco_freq/rate)*(1ull<<32);
    return adi_apollo_cnco_ftw_set(device, terminal, cncos, 0, 1, ftw);
}

int32_t adi_ads10_apollo_ex_cnco_freq_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t cncos, bool mod_en, uint64_t fs, uint64_t fo)
{
    int32_t err;
    adi_apollo_coarse_nco_chan_pgm_t config;
    uint64_t x, a, b;

    adi_api_utils_ratio_decomposition(fo, fs, 32, &x, &a, &b);
    config.drc_phase_inc = (uint32_t)x;
    config.drc_phase_inc_frac_a = (mod_en) ? (uint32_t)a : 0;
    config.drc_phase_inc_frac_b = (uint32_t)b;

    printf("fo: %lld, fs: %lld, x: %d, a: %d, b: %d\n", fo, fs, config.drc_phase_inc, config.drc_phase_inc_frac_a, config.drc_phase_inc_frac_b);

    err = adi_apollo_cnco_ftw_set(device, terminal, cncos, 0, 1, config.drc_phase_inc);
    ADI_APOLLO_ERROR_RETURN(err);

    err = adi_apollo_cnco_mod_set(device, terminal, cncos, config.drc_phase_inc_frac_a, config.drc_phase_inc_frac_b);
    ADI_APOLLO_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;

}

int32_t adi_ads10_apollo_ex_fnco_set(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, const uint16_t fncos, double rate, double fnco_freq)
{
    int64_t ftw;
    
    ftw = (fnco_freq/rate)*(1ll<<48);
    return adi_apollo_fnco_main_phase_inc_set(device, terminal, fncos, ftw);
}

int32_t adi_ads10_apollo_ex_reg_dump(adi_apollo_device_t* device, char* file_name, uint32_t first, uint8_t regs[], uint32_t len)
{
    int32_t err;
    uint8_t reg8;
    uint32_t i=0;
    uint32_t r;
    FILE* fp;
    char fname[MAX_PATH_LEN];
    uint8_t write_to_file = (file_name != NULL);

    if (write_to_file) {
        snprintf(fname, MAX_PATH_LEN, "%s/%s", OUTPUT_DIR, file_name);
        if (fp = fopen(fname, "w+"), fp == NULL) {
            printf("Can't create output file %s\n", fname);
            err = API_CMS_ERROR_FILE_OPEN;
            goto end;
        }
        printf("Write Apollo reg dump to file %s...\n", fname);
    }

    for (i = 0; i < len; i++) {
        r = i + first;
        adi_apollo_hal_reg_get(device, r, &reg8);
        regs[i] = reg8;

        if (write_to_file) {
            fprintf(fp, "0x%04x 0x%02x\n", r, reg8);
        }
    }

    if (write_to_file) {
        fclose(fp);
    }

end:
    return err;
}

int32_t adi_ads10_apollo_ex_fpga_reg_dump(adi_fpga_apollo_device_t* fpga_device, char* file_name, uint32_t first, uint32_t regs[], uint32_t len)
{
    int32_t err;
    uint32_t i=0;
    uint32_t reg32;
    uint32_t r;
    FILE* fp;
    char fname[MAX_PATH_LEN];
    uint8_t write_to_file = (file_name != NULL);

    if (write_to_file) {
        snprintf(fname, MAX_PATH_LEN, "%s/%s", OUTPUT_DIR, file_name);
        if (fp = fopen(fname, "w+"), fp == NULL) {
            printf("Can't create output file %s\n", fname);
            err = API_CMS_ERROR_FILE_OPEN;
            goto end;
        }
        printf("Write FPGA reg dump to file %s...\n", fname);
    }

    for (i=0; i < len; i++) {
        r = i + first;
        adi_fpga_apollo_core_reg_get(fpga_device, r, &reg32);
        regs[i] = reg32;

        if (write_to_file) {
            fprintf(fp, "0x%04x 0x%08x\n", r, reg32);
        }
    }

    if (write_to_file) {
        fclose(fp);
    }

end:
    return err;
}


int32_t adi_ads10_ex_pfilt_coeff_file_load(adi_apollo_device_t *device, adi_apollo_terminal_e terminal, uint8_t pfilts, uint8_t pfilt_banks, char *file_path)
{
    int32_t err = API_CMS_ERROR_OK;
    int i = 0;
    uint16_t coeffs[ADI_APOLLO_PFILT_COEFF_NUM] = { 0 };
    const int max_num_str_len = 16;
    char num_str[max_num_str_len];
    char* end_ptr;
    FILE *fp;

    if (file_path==NULL || coeffs == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    if (fp = fopen(file_path, "r"), fp == NULL) {
        printf("Can't open file %s\n", file_path);
        err = API_CMS_ERROR_FILE_OPEN;
        goto end;
    }

    printf("Load Filter: %s\n", file_path);
    while ((fgets(num_str, max_num_str_len, fp) != NULL) && (i < ADI_APOLLO_PFILT_COEFF_NUM)) {

        coeffs[i] = strtol(num_str, &end_ptr, 0);
        i++;
    }
    fclose(fp);

    if (i != ADI_APOLLO_PFILT_COEFF_NUM) {
        printf("Expected %d coeffs but read %d\n", ADI_APOLLO_PFILT_COEFF_NUM, i);
        err = API_CMS_ERROR_ERROR;
        goto end;
    }

    err = adi_apollo_pfilt_coeff_pgm(device, terminal, pfilts, pfilt_banks, coeffs, ADI_APOLLO_PFILT_COEFF_NUM);

end:
    return err;
}


int32_t adi_ads10_ex_dp_info_get(adi_apollo_device_t *device,  adi_apollo_top_t *profile, adi_apollo_terminal_e terminal, uint32_t side, uint32_t idx, adi_ads10_apollo_dp_info_t *dp_info)
{
    int32_t err = API_CMS_ERROR_OK;

    if (device == NULL || profile == NULL || dp_info == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    if ( (terminal > 1) || (side > 1) || (idx > 7) ) {
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }

    /* Get sampling freq from profile */
    dp_info->fclk = profile->clk_cfg.dev_clk_freq_kHz / 1e3;
    dp_info->adc_sample_rate = profile->adc_config[side].adc_sampling_rate_Hz / 1e6;
    dp_info->dac_sample_rate = profile->dac_config[side].dac_sampling_rate_Hz / 1e6;
    dp_info->terminal = terminal;

    if (terminal == ADI_APOLLO_RX) {
        adi_apollo_cddc_dcm_bf_to_val(device, profile->rx_path[side].rx_cddc[idx].drc_ratio, &dp_info->cdrc);
        adi_apollo_fddc_dcm_bf_to_val(device, profile->rx_path[side].rx_fddc[idx].drc_ratio, &dp_info->fdrc);
        dp_info->coarse_ftw = profile->rx_path[side].rx_cddc[idx].nco[0].nco_phase_inc;
        dp_info->fine_ftw = profile->rx_path[side].rx_fddc[idx].nco[0].nco_phase_inc;
        dp_info->fsrc_en = profile->rx_path[side].rx_fsrc[idx].enable;
        dp_info->fsrc_1x_en = profile->rx_path[side].rx_fsrc[idx].mode_1x;
        dp_info->fsrc_ratio = adi_ads10_apollo_ex_fsrc_ratio_get(&profile->rx_path[side].rx_fsrc[idx], &dp_info->fsrc_n, &dp_info->fsrc_m);
        dp_info->pfilt_en = profile->rx_path[side].rx_pfilt[idx].enable;
        dp_info->cfir_en = profile->rx_path[side].rx_cfir[idx].enable;
        dp_info->cnco_freq = ((dp_info->adc_sample_rate) * dp_info->coarse_ftw) / (1ull << 32);
        dp_info->fnco_freq = ((dp_info->adc_sample_rate / dp_info->cdrc) * dp_info->fine_ftw) / (1ull << 48);
        dp_info->fdata = dp_info->adc_sample_rate / (dp_info->cdrc * dp_info->fdrc);       // JESD data rate
    } else {
        adi_apollo_cduc_interp_bf_to_val(device, profile->tx_path[side].tx_cduc[idx].drc_ratio, &dp_info->cdrc);
        adi_apollo_fduc_interp_bf_to_val(device, profile->tx_path[side].tx_fduc[idx].drc_ratio, &dp_info->fdrc);
        dp_info->coarse_ftw = profile->tx_path[side].tx_cduc[idx].nco[0].nco_phase_inc;
        dp_info->fine_ftw = profile->tx_path[side].tx_fduc[idx].nco[0].nco_phase_inc;
        dp_info->fsrc_en = profile->tx_path[side].tx_fsrc[idx].enable;
        dp_info->fsrc_1x_en = profile->tx_path[side].tx_fsrc[idx].mode_1x;
        dp_info->fsrc_ratio = adi_ads10_apollo_ex_fsrc_ratio_get(&profile->tx_path[side].tx_fsrc[idx], &dp_info->fsrc_n, &dp_info->fsrc_m);
        dp_info->pfilt_en = profile->tx_path[side].tx_pfilt[idx].enable;
        dp_info->cfir_en = profile->tx_path[side].tx_cfir[idx].enable;
        dp_info->cnco_freq = ((dp_info->dac_sample_rate) * dp_info->coarse_ftw) / (1ull << 32);
        dp_info->fnco_freq = ((dp_info->dac_sample_rate / dp_info->cdrc) * dp_info->fine_ftw) / (1ull << 48);
        dp_info->fdata = dp_info->dac_sample_rate / (dp_info->cdrc * dp_info->fdrc);       // JESD data rate
    }

    dp_info->total_drc = dp_info->cdrc * dp_info->fdrc;
    dp_info->fsrc_data_rate = dp_info->fdata / dp_info->fsrc_ratio;         // FSRC data rate

 end:
    return err;
}

int32_t adi_ads10_apollo_ex_samples_16x2_stride_to_file(char* file_name, uint32_t data_offset, uint32_t stride, uint32_t data[], uint32_t len)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i;
    int16_t s0, s1;
    char fname[MAX_PATH_LEN];
    FILE* fp;

    if (file_name == NULL || data == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }


    snprintf(fname, MAX_PATH_LEN, "%s/%s", OUTPUT_DIR, file_name);
    if (fp = fopen(fname, "w+"), fp == NULL) {
        printf("Can't create output file %s\n", fname);
        err = API_CMS_ERROR_FILE_OPEN;
        goto end;
    }
    printf("Write samples to file %s...\n", fname);

    for (i = data_offset; i < len; i+=stride) {
        s0 = (int16_t)((data[i] >> 0) & 0x0000ffff);
        s1 = (int16_t)((data[i] >> 16) & 0x0000ffff);
        fprintf(fp, "%d\n%d\n", s0, s1);
    }

    fclose(fp);

end:
    return err;
}

int32_t adi_ads10_apollo_ex_samples_16x2_to_file(char* file_name, uint32_t data[], uint32_t len)
{
    return adi_ads10_apollo_ex_samples_16x2_stride_to_file(file_name, 0, 1, data, len);
}

int32_t adi_ads10_apollo_ex_samples_to_file(char* file_name, int16_t data[], uint32_t len, bool de_interleave)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i;
    char fname0[MAX_PATH_LEN];
    char fname1[MAX_PATH_LEN];
    FILE* fp0 = NULL;
    FILE* fp1 = NULL;

    if ((len < 2) || (len % 2) != 0) {
        printf("len must be > 0 and multiple of 2");
        return API_CMS_ERROR_INVALID_PARAM;
    }

    snprintf(fname0, MAX_PATH_LEN, "%s/%s%s.txt", OUTPUT_DIR, file_name, de_interleave ? "_I" : "");
    if (fp0 = fopen(fname0, "w+"), fp0 == NULL) {
        printf("Can't create output file %s\n", fname0);
        err = API_CMS_ERROR_FILE_OPEN;
        goto end;
    }
    printf("Write samples to file %s...\n", fname0);

    if (de_interleave) {
        snprintf(fname1, MAX_PATH_LEN, "%s/%s%s.txt", OUTPUT_DIR, file_name, "_Q");
        if (fp1 = fopen(fname1, "w+"), fp1 == NULL) {
            printf("Can't create output file %s\n", fname1);
            err = API_CMS_ERROR_FILE_OPEN;
            goto end0;
        }
        printf("Write samples to file %s...\n", fname1);
    }

    for (i = 0; i < len; i++) {
        fprintf(fp0, "%d\n", data[i]);
        if (de_interleave) {
            i++;
            fprintf(fp1, "%d\n", data[i]);
        }
    }

    if (fp1 != NULL) {
        fclose(fp1);
    }

 end0:
    if (fp0 != NULL) {
        fclose(fp0);
    }

end:
    return err;
}

int32_t adi_ads10_apollo_ex_jesd_rx_status(adi_apollo_device_t* device, uint16_t link)
{
    int32_t err;
    uint16_t status = 0;
    adi_apollo_jesd_rx_inspect_t jrx_status;

    printf("Jrx Link Status: %s\n",
    (link == ADI_APOLLO_LINK_A0) ? "A0" : (link == ADI_APOLLO_LINK_A1) ? "A1" :
    (link == ADI_APOLLO_LINK_B0) ? "B0" : (link == ADI_APOLLO_LINK_B1) ? "B1" : "?");
    
    err = adi_apollo_jrx_link_inspect(device, link, &jrx_status);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }
	
    if (jrx_status.ver == ADI_APOLLO_JESD_204C) {
        printf("Apollo JESD204C version selected\n");
        for (int i = 0; i < ADI_APOLLO_JESD_MAX_LANES_PER_SIDE; i++) {
            err = adi_apollo_jrx_j204c_lane_status_get(device, link, i, &status);
            if (err != API_CMS_ERROR_OK) {
                return err;
            }
            if (0x6 == (status & 0x7)) {
                printf("JRx lane status[%02d]: Link ready\n", i);
            }
            else {
                printf("JRx lane status[%02d]: Link not ready (%d)\n", i, status & 0x7);
            }
        }
	} 
    else {
        printf("Apollo JESD204B version selected\n");
        for (int i = 0; i < ADI_APOLLO_JESD_MAX_LANES_PER_SIDE; i++) {
            err = adi_apollo_jrx_j204b_lane_status_get(device, link, i, &status);
            if (err != API_CMS_ERROR_OK) {
                return err;
            }
            if (0x38 == (status & 0x3C)) {
                printf("JRx lane status[%02d]: Link ready\n", i);
            }
            else {
                printf("JRx lane status[%02d]: Link not ready (%d)\n", i, status & 0x3C);
            }
        }
	}

    err = adi_apollo_jrx_link_status_get(device, link, &status);
    if (err != API_CMS_ERROR_OK) {
        return err;
    }

    printf("JRx User Ready status: 0x%x\n", (status & 0x20) >> 5);
    printf("JRx SYSREF Phase lock: 0x%x\n", (status & 0x40) >> 6);

    return ((status & 0x20) >> 5) == 0 ? API_CMS_ERROR_LINK_DOWN : API_CMS_ERROR_OK;
}

static int32_t check_link_status_capture_mem(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, uint8_t *cap_buffer, uint32_t bytes_to_capture)
{
    int32_t err;
    uint8_t is_link_up;
    uint8_t loop_count = 0;
    uint8_t cap_status;
    uint32_t use_sysref_seq_ext_trig = 0;

    /* ADS10 FPGA link status */
    adi_fpga_apollo_core_jtx_link_status_get(fpga_device, &is_link_up);
    printf("LINK is %s\n", is_link_up ? "UP" : "DOWN");

    /* If the link isn't already up, then start a throwaway capture to get link going */
    if (!is_link_up) {
        /* Start capture */
        if (err = adi_fpga_apollo_core_capture_start(fpga_device), err != API_CMS_ERROR_OK) {
            printf("Error from adi_fpga_apollo_core_capture_start()\n");
            goto end;
        }

        /* allow for some link activity  */
        adi_apollo_hal_delay_us(device, 5000000);
    }

    /* Recheck link if was initially down (can happen on first call after FPGA image load) */
    if (!is_link_up) {
        /* ADS10 FPGA link status */
        adi_fpga_apollo_core_jtx_link_status_get(fpga_device, &is_link_up);
        printf("LINK is %s\n", is_link_up ? "UP" : "DOWN");
    }

    if (!is_link_up) {
        printf("Aborting capture because links not up\n");
        err = API_CMS_ERROR_LINK_DOWN;
        goto end;
    }

    /* Start capture - For SC1 use sysref seq to generate trigger */
    adi_fpga_apollo_core_sysref_seq_ext_trig_enable_get(fpga_device, &use_sysref_seq_ext_trig);
    if (use_sysref_seq_ext_trig) {
        if (err = adi_fpga_apollo_core_capture_start_x(fpga_device), err != API_CMS_ERROR_OK) {
            printf("Error from adi_fpga_apollo_core_capture_start_x()\n");
            goto end;
        }

        // Allow Apollo to respond to trigger
        adi_apollo_clk_mcs_trig_sync_enable(device, 1);

        if (err = adi_fpga_apollo_core_capture_start_x2(fpga_device), err != API_CMS_ERROR_OK) {
            printf("Error from adi_fpga_apollo_core_capture_start_x2()\n");
            goto end;
        }

        /*
         * Wait for invalids to clear
         */
        #include "adi_apollo_bf_jtx_dformat.h"
        adi_apollo_hal_bf_wait_to_clear(device, BF_FORCE_INVALID_EN_INFO(JTX_DFORMAT_JTX_TOP_RX_DIGITAL0, 0), 1000000, 100);
        adi_apollo_hal_bf_wait_to_clear(device, BF_FORCE_INVALID_EN_INFO(JTX_DFORMAT_JTX_TOP_RX_DIGITAL1, 0), 1000000, 100);

        /*
         * Wait for the trigger sync to finish.
         */
        #include "adi_apollo_bf_mcs_sync.h"
        adi_apollo_hal_bf_wait_to_set(device, BF_TRIGGER_SYNC_DONE_A0_INFO(MCS_SYNC_MCSTOP0), 1000000, 100);   // Will set when Apollo recvs a trigger pulse

    } else {
        if (err = adi_fpga_apollo_core_capture_start(fpga_device), err != API_CMS_ERROR_OK) {
            printf("Error from adi_fpga_apollo_core_capture_start()\n");
            goto end;
        }
    }
    
    /* Wait for capture to finish */
    do {
        loop_count++;

        if (err = adi_fpga_apollo_core_capture_status(fpga_device, &cap_status), err != API_CMS_ERROR_OK) {
            printf("Error from adi_fpga_apollo_core_capture_status()\n");
            goto end;
        }
        printf("cap_running: %d; cap_stopped: %d\n", (cap_status >> 1) & 0x1, (cap_status >> 3) & 0x1);

        if (loop_count > 50) {
            printf("Capture complete error\n");
            goto end;
        }

        adi_apollo_hal_delay_us(device, 10000);
    } while (cap_status);

    uint32_t fifo_rdy;
    adi_fpga_apollo_core_fifo_ready_get(fpga_device, &fifo_rdy);
    printf("The fifo is %s\n", fifo_rdy ? "READY" : "NOT READY");
    if (!fifo_rdy) {
        printf("Capture FIFO not ready\n");
        err = API_CMS_ERROR_PLATFORM_READ;
        goto end;
    }

    if (err = ads10_fpga_mem_read(0, bytes_to_capture, cap_buffer), err != API_CMS_ERROR_OK) {
        printf("Error from ads10_fpga_mem_read\n");
        goto end;
    }

    printf("Capture Buf (first 16):\n");
    for (int i = 0; i < 16; i++)
    {
        printf("0x%0x ", cap_buffer[i]);
    }
    printf("\n");

end:
    return err;
}

static int32_t print_tot_samples_per_conv(adi_fpga_apollo_device_t *fpga_device)
{
    adi_fpga_apollo_capture_frame_t *cap_frame_info;

    ADI_CMS_NULL_PTR_CHECK(fpga_device);

    cap_frame_info = &fpga_device->state_info.capture_info.capture_frame;

    for (int i = 0; i < MAX_JESD_LINKS; i++) {
        if (cap_frame_info->link_enabled[i]) {
            printf("Link %d: %d expected samples per converter\n", i, cap_frame_info->tot_samps_per_conv[i]);
        }
    }

    return API_CMS_ERROR_OK;
}
static __maybe_unused  bool is_fpga_jrx_linkup(adi_fpga_apollo_device_t *fpga_device)
{
    uint8_t is_link_up;
    uint32_t fpga_reg32;

    adi_fpga_apollo_core_reg_get(fpga_device, 0x0208, &fpga_reg32);
    printf("REG208: 0x%08x (bits 4 (Jesd204c Rx Sync Header Lock) & 5 (Rx Extended Multiblock Lock))\n", fpga_reg32);
    is_link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;

    return is_link_up;
}

static __maybe_unused bool is_apollo_jrx_linkup(adi_apollo_device_t *device, uint16_t link, uint8_t n_lanes)
{
    uint8_t is_link_up = true;
    uint16_t status = 0;

    for (int i = 0; i < n_lanes; i++) {
        adi_apollo_jrx_j204c_lane_status_get(device, link, i, &status);
        if (6 == (status & 0x7)) {
            printf("JRx lane status[%d]: Link ready\n", i);
        } else {
            is_link_up = false;
            printf("JRx lane status[%d]: Link not ready (%d)\n", i, status & 0x7);
        }
    }
    adi_apollo_jrx_link_status_get(device, link, &status);
    printf("JRx User Ready status: 0x%x\n", (status & 0x20) >> 5);
    printf("JRx SYSREF Phase lock: 0x%x\n", (status & 0x40) >> 6);

    is_link_up &= ((status & 0x20) != 0) && ((status & 0x40) != 0);

    return is_link_up;
}

static __maybe_unused int32_t hsci_buf_alloc(adi_apollo_device_t *device)
{
    uint8_t *buff;
    uint32_t buff_len;
    
    /* Allocate optional HSCI transaction buffer */
    buff_len = ADI_APOLLO_HAL_REGIO_HSCI_STREAM_DEFAULT_SIZE;
    buff = (uint8_t *)malloc(buff_len * sizeof(uint8_t));
    ADI_CMS_MEM_ALLOC_CHECK(buff);

    return  adi_apollo_hal_buffer_set(device, ADI_APOLLO_HAL_PROTOCOL_HSCI, buff, buff_len);
}
