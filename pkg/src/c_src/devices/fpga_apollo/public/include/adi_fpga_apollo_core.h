/*!
 * \brief     Basic FPGA Apollo functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_CORE_H__
#define __ADI_FPGA_APOLLO_CORE_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_core_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief        Get version of FPGA
 *
 * \param[in]   fpga Context variable
 * \param[out]  fpgaVersion The FPGA version
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_core_version_get(adi_fpga_apollo_device_t *fpga,
                                        adi_fpga_apollo_version_t *fpgaVersion);

/**
 * \brief       FPGA waits or sleeps for microseconds.
 *
 * \param[in]   fpga        Context variable
 * \param[in]   wait_us     Wait / sleep time in microseconds.
 *
 * \return      API_CMS_ERROR_OK API Completed Successfully
 */
int32_t adi_fpga_apollo_core_wait_us(adi_fpga_apollo_device_t* fpga, uint32_t wait_us);

/**
 * \brief       Read FPGA register
 *
 * \param[in]   fpga Context variable
 * \param[in]   reg_offset Offset value of a register address
 * \param[out]  out_data   Read back value of register loacted at the offset address
 *
 * \return      API_CMS_ERROR_OK API Completed Successfully
 */
int32_t adi_fpga_apollo_core_reg_get(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg_offset, uint32_t* out_data);

/**
 * \brief       Writes data to FPGA register at reg_offset offset.
 *
 * \param[in]   fpga Context variable
 * \param[in]   reg_offset Offset value of a register address
 * \param[in]   data   Value to be written to the register located at the
 *                     reg_offset address.
 *
 * \return      API_CMS_ERROR_OK API Completed Successfully
 */
int32_t adi_fpga_apollo_core_reg_set(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg_offset, uint32_t data);

/**
 * \brief       Set FPGA bit field value.
 *
 * \param[in]   fpga        Context variable
 * \param[in]   reg         Register address
 * \param[in]   bf_mask     bit field mask denoting it's offset and length within the register
 * \param[in]   val         Value to be written to the bitfield located at the reg address.
 *
 * \return      API_CMS_ERROR_OK API Completed Successfully
 */
int32_t adi_fpga_apollo_core_bf_set(adi_fpga_apollo_device_t *fpga,
					                uint32_t reg, uint32_t bf_mask, uint32_t val);

/**
 * \brief       Read FPGA bit field value.
 *
 * \param[in]   fpga        Context variable
 * \param[in]   reg         Register address
 * \param[in]   bf_mask     bit field mask denoting it's offset and length within the register
 * \param[out]  val         Read back value of the bitfield located at the reg address.
 *
 * \return      API_CMS_ERROR_OK API Completed Successfully
 */
int32_t adi_fpga_apollo_core_bf_get(adi_fpga_apollo_device_t *fpga,
                                    uint32_t reg, uint32_t bf_mask, uint32_t *val);

/**
 * \brief       Configures FPGA JESD RX parameters
 *
 * \param[in]   fpga          Context variable
 * \param[in]   jrx_param   Array with JESD Parameters for all the links
 * \param[in]   length        jrx_param's length - it must be equal with
 *                            the numebr of FPGA JESD Rx links = JESD204B_RX_LINK_COUNT + 1
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jesd_rx_config(adi_fpga_apollo_device_t *fpga,
                                           adi_fpga_jesd_param_t jrx_param[],
                                           uint32_t length);

/**
 * \brief       Configures FPGA JESD TX parameters
 *
 * \param[in]   fpga          Context variable
 * \param[in]   jtx_param   Array with JESD Parameters for all the links
 * \param[in]   length        jtx_param's length - it must be equal with
 *                            the number of FPGA JESD Tx linls = JESD204B_TX_LINK_COUNT + 1
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jesd_tx_config(adi_fpga_apollo_device_t *fpga,
                                           adi_fpga_jesd_param_t jtx_param[],
                                           uint32_t length);

/**
 * \brief     Set FPGA JTx PRBS pattern
 *
 * \param[in]   fpga      Context variable
 * \param[in]   prbs	  Array with transceiver TX PRBS pattern check select
 * \param[in]   length    prbs array's length - the number of lanes
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_pattern_set(adi_fpga_apollo_device_t *fpga,
                                                    adi_apollo_fpga_prbs_e prbs[],
                                                    uint32_t length);

/**
 * \brief       Configure Transceiver Channel Driver
 *
 * \param[in]   fpga      Context variable
 * \param[in]   diff_amp  Array with GTH transceiver channel TXDIFFCTRL setting (PE set idx 15 = 760mv)
 * \param[in]   pre_cur   Array with GTH transceiver channel TXPRECURSOR setting (PE set idx 0)
 * \param[in]   post_cur  Array with GTH transceiver channel TXPOSTCURSOR setting (PE set idx 0)
 * \param[in]   length    Above arrays length; equal with the number of lanes.
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_drive_config(adi_fpga_apollo_device_t *fpga,
                                                uint8_t diff_amp[], uint8_t pre_cur[],
                                                uint8_t post_cur[], uint32_t length);

/**
 * \brief       Transmit PRBS pattern to the DUT
 *
 * \param[in]   fpga         Context variable
 * \param[in]   invert       Transceiver TX polarity
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jesd_tx_phy_prbs_start(adi_fpga_apollo_device_t *fpga,
                                                    bool invert);

/**
 * \brief   Configure and capture PRBS pattern sent by the DUT
 *
 * \param[in]   fpga            Context variable
 * \param[in]   prbs            Transceiver RX PRBS pattern check select
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_init(adi_fpga_apollo_device_t *fpga,
                                                   adi_apollo_fpga_prbs_e prbs);

/**
 * \brief   Request for cycle and error count update with optional counter reset
 *
 * \param[in]   fpga            Context variable
 * \param[in]   clear_errors    Clear the cycle and error counters if true
 *
 * \return      API_CMS_ERROR_OK                API Completed Successfully
 *              API_CMS_ERROR_OPERATION_TIMEOUT PRBS sync not detected on all lanes
 */
int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_request_update(adi_fpga_apollo_device_t *fpga, bool clear_errors);


/**
 * \brief   Prints a summary of lane cycle and error counts
 *
 * \param[in]   fpga            Context variable
 * \param[in]   prbs            Transceiver RX PRBS pattern check select
 * \param[in]   clear_errors    Clear the cycle and error counters if true
 * \param[out]  active_lanes    Number of active lanes
 * \param[out]  err_lanes       Number of lanes with errors
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 *              API_CMS_ERROR_OPERATION_TIMEOUT PRBS sync not detected on all lanes
 */
int32_t adi_fpga_apollo_core_jesd_rx_phy_prbs_print_errors(adi_fpga_apollo_device_t *fpga,
                                                           adi_apollo_fpga_prbs_e prbs,
                                                           bool clear_errors, uint16_t *active_lanes, uint16_t *err_lanes);


/**
 * \brief   Get FPGA JTx link up status
 *
 * \param[in]   fpga            Context variable
 * \param[out]  link_up         Link up status
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jtx_link_status_get(adi_fpga_apollo_device_t *fpga, uint8_t *link_up);

/**
 * \brief   Start FPGA capture status
 *
 * \param[in]   fpga            Context variable
 * \param[out]  status          Capture status byte
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_status(adi_fpga_apollo_device_t *fpga, uint8_t *status);

/**
 * \brief   Start FPGA capture
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_start(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Start FPGA capture for FSRC mode (first phase)
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_start_x(adi_fpga_apollo_device_t* fpga);


/**
 * \brief   Start FPGA capture for FSRC mode (second phase)
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_start_x2(adi_fpga_apollo_device_t* fpga);


/**
 * \brief   Stop FPGA capture
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_stop(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Set FPGA capture size
 *
 * \param[in]   fpga               Context variable
 * \param[in]   cap_size           No of bytes to capture / 64k
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_capture_size_set(adi_fpga_apollo_device_t *fpga, uint64_t cap_size);

/**
 * \brief       Waits for capture to complete.
 *
 * \param[in]   fpga               Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 *              API_CMS_ERROR_OPERATION_TIMEOUT  Timeout
 *
 */
int32_t adi_fpga_apollo_core_capture_complete_wait(adi_fpga_apollo_device_t *fpga);

/**
    * \brief       Waits for operation not running (e.g. after bdir start).
    *
    * \param[in]   fpga               Context variable
    *
    * \return      API_CMS_ERROR_OK   API Completed Successfully
    *              API_CMS_ERROR_OPERATION_TIMEOUT  Timeout
    */
int32_t adi_fpga_apollo_core_not_running_wait(adi_fpga_apollo_device_t *fpga);


/**
 * \brief   Start FPGA pattern read
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_pattern_read_start(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Stop FPGA pattern read
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_pattern_read_stop(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Load FPGA pattern
 *
 * \param[in]   fpga            Context variable
 * \param[in]   section_start   FPGA pattern load starting memory section
 * \param[in]   section_end     FPGA pattern load ending memory section
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_memory_section_select (adi_fpga_apollo_device_t *fpga,
                                                    uint8_t section_start, uint8_t section_end);
/**
 * \brief   Write data pattern to FPGA memory
 *
 * \param[in]   fpga            Context variable
 * \param[in]   mem_addr        FPGA data memory address
 * \param[in]   tx_data         pattern data buffer byte array
 * \param[in]   tx_data_bytes   pattern data buffer array size
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_memory_write(adi_fpga_apollo_device_t *fpga,
                                          uint32_t mem_addr, uint8_t tx_data[],
                                          uint32_t tx_data_bytes);

/**
 * \brief   Read captured data from FPGA memory.
 *
 * \param[in]   fpga       Context variable
 * \param[in]   mem_addr   FPGA data memory address. Not currently used as the
 *                         capture address is hardcoded to 0x0 in the FPGA.
 * \param[out]  data       Data to read from memory
 * \param[in]   data_len   Length of the data array
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_memory_read(adi_fpga_apollo_device_t *fpga,
                                         uint32_t mem_addr, uint8_t data[],
                                         uint32_t data_len);

/**
 * \brief   Start FPGA transmit
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_transmit_start(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Stop FPGA transmit
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_transmit_stop(adi_fpga_apollo_device_t *fpga);

/**
 * \brief   Start playing data pattern after starting transmit
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_ptn_play_start(adi_fpga_apollo_device_t* fpga);

/**
 * \brief   Stop playing data pattern without stopping transmit
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_ptn_play_stop(adi_fpga_apollo_device_t* fpga);

/**
 * \brief   Stop FSRC
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_tx_fsrc_stop(adi_fpga_apollo_device_t* fpga);

/**
 * \brief   FSRC change rate
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_tx_fsrc_change_rate(adi_fpga_apollo_device_t* fpga);


/**
 * \brief   Start the TX SYSREF sequencer
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_tx_fsrc_seq_start(adi_fpga_apollo_device_t* fpga);


/**
 * \brief    Set FPGA transmit pattern length
 *
 * \param[in]   fpga               Context variable
 * \param[in]   len                Transmit pattern length
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_pattern_len_set(adi_fpga_apollo_device_t *fpga, uint64_t len);

/**
 * \brief    Set FPGA transmit pattern address
 *
 * \param[in]   fpga               Context variable
 * \param[in]   addr               Transmit pattern address
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_pattern_addr_set(adi_fpga_apollo_device_t *fpga, uint32_t addr);

/**
 * \brief    Set FPGA transmit pattern address per side
 *
 * \param[in]   fpga               Context variable
 * \param[in]   mem_addr           Transmit pattern address
 * \param[in]   side_select        0x1 = A side or 0x2 = B side
 * \param[in]   len                Transmit pattern length
 * \param[in]   dual_link          Link type. 0:Single 1:Dual
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_transmit_link_config(adi_fpga_apollo_device_t *fpga,
                                                  uint32_t mem_addr, uint32_t side_select, uint32_t len, uint8_t dual_link);

/**
 * \brief    Set FPGA transmit options
 *
 * \note Sets Tx to loop loaded vector repeatably, and optionally use all memory for single link use cases
 *       Similar to adi_fpga_apollo_core_transmit_link_config(), but doesn't set memory start/len.
 * 
 * \param[in]   fpga               Context variable
 * \param[in]   dual_link          Link type. 0:Single 1:Dual
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_transmit_link_config2(adi_fpga_apollo_device_t *fpga, uint8_t dual_link);

/**
 * \brief    Bidirectional (Data transmit and capture) Start
 *
 * \param[in]   fpga               Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_bidir_start(adi_fpga_apollo_device_t *fpga);

/**
 * \brief    Bidirectional (Data transmit and capture) Stop
 *
 * \param[in]   fpga               Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_core_bidir_stop(adi_fpga_apollo_device_t *fpga);

/**
 * \brief    Skip Receive Link Initialization
 *
 *           Skip receive link initializationIf set and doing JESD204 capture, SYNCB is not asserted;
 *           CGS and ILAS are not performed. If set and doing C2C capture, link alignment is not performed
 *           Requires a previous successful capture and requires the transceiver
 *           line rate and JESD204 mode (if applicable) to stay the same as the previous capture.
 *
 * \param[in]   fpga               Context variable
 * \param[in]   skip_link_init     Whether or not to skip Rx link init.
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_skip_rx_link_init_set(adi_fpga_apollo_device_t *fpga, uint8_t skip_link_init);

/**
 * \brief    Rx keep link mask
 *
 * \param[in]   fpga                Context variable
 * \param[in]   mask                Each bit set indicates not to stop or restart a link are it's initialized.
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_rx_keep_link_mask_set(adi_fpga_apollo_device_t *fpga, uint8_t mask);

/**
 * \brief    Tx keep link mask
 *
 * \param[in]   fpga                Context variable
 * \param[in]   mask                Each bit set indicates not to stop or restart a link are it's initialized.
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_tx_keep_link_mask_set(adi_fpga_apollo_device_t *fpga, uint8_t mask);


/**
 * \brief    Sets the fifo ready state value
 *
 * \param[in]   fpga            Context variable
 * \param[in]   val             Value to set the fifo ready bit [0|1] (useful for debugging capture)
 */
int32_t adi_fpga_apollo_core_fifo_ready_set(adi_fpga_apollo_device_t *fpga, uint32_t val);

/**
 * \brief    Gets the fifo ready state value
 *
 * \param[in]   fpga            Context variable
 * \param[out]  val             Gets the fifo ready bit
*
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_fifo_ready_get(adi_fpga_apollo_device_t *fpga, uint32_t* val);

/**
 * \brief    Gets the fpga JTx link count.
 *
 * \param[in]   fpga            Context variable
 * \param[out]  link_count      Ptr to link count result. Usually 4 for Apollo, but can be 2 for some images
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_core_jtx_link_cnt_get(adi_fpga_apollo_device_t *fpga, uint32_t* link_count);

/**
 * \brief    Gets the fpga JRx link count.
 *
 * \param[in]   fpga            Context variable
 * \param[out]  link_count      Ptr to link count result. Usually 4 for Apollo, but can be 2 for some images
 */
int32_t adi_fpga_apollo_core_jrx_link_cnt_get(adi_fpga_apollo_device_t *fpga, uint32_t* link_count);

/**
 * \brief   Get FPGA feature flags.
 *
 * \param[in]   fpga                Context variable
 * \param[out]  feature_flags       Ptr to feature flag struct. \ref adi_fpga_feature_flag_t
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_feature_flags_get(adi_fpga_apollo_device_t *fpga, adi_fpga_feature_flag_t *feature_flags);

/**
 * \brief Sets the system indicator output function. (SMA J3 on ADS10)
 *
 * \param[in]   fpga            Context variable
 * \param[in]   indicator       Sets the system indicator output function. Rx ready or Tx pattern start. \ref adi_apollo_sys_indicator_e
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sys_indicator_set(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sys_indicator_e indicator);

/**
 * \brief Gets the system indicator output function. (SMA J3 on ADS10)
 *
 * \param[in]   fpga            Context variable
 * \param[out]  indicator       System indicator output function result ptr. Rx ready or Tx pattern start. \ref adi_fpga_apollo_sys_indicator_e
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sys_indicator_get(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sys_indicator_e *indicator);

/**
 * \brief Sets the ADS10 SYSREF source
 *
 * \param[in]   fpga            Context variable
 * \param[in]   sysref_src      SYSREF source. May be from AD9528, FMC or SMA J5  \ref adi_fpga_apollo_sysref_source_e
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sysref_src_set(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sysref_source_e sysref_src);

/**
 * \brief Gets the ADS10 SYSREF source
 *
 * \param[in]   fpga            Context variable
 * \param[out]  sysref_src      SYSREF source result ptr. May be from AD9528, FMC or SMA J5  \ref adi_fpga_apollo_sysref_source_e
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sysref_src_get(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_sysref_source_e* sysref_src);


/**
 * \brief Sets the data transmit skip data enable. If enabled, zeros are transmitted instead of pattern data.
 *
 * \param[in]   fpga    Context variable
 * \param[out]  skip    If true, zeros are transmitted instead of pattern data.
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_transmit_skip_data_en(adi_fpga_apollo_device_t *fpga, bool skip);

/**
 * \brief Gets the currently loaded FPGA image version
 *
 * Combines the date codes into one 32-bit value.
 * For example: 0x230201700 represents Feb 2, 2023
 *
 * \param[in]   fpga Context variable
 * \param[out]  image_ver The FPGA version
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_core_image_ver_get(adi_fpga_apollo_device_t* fpga, uint32_t* image_ver);

/**
 * \brief Sets enable state of the SYSREF sequencer ext trigger.
 *
 * \param[in]   fpga            Context variable
 * \param[in]   ext_trig_en   If true, the ext trig sysref sequence is enabled
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set(adi_fpga_apollo_device_t* fpga, uint32_t ext_trig_en);

/**
 * \brief Gets enable state of the SYSREF sequencer ext trigger.
 *
 * \param[in]   fpga            Context variable
 * \param[out]  ext_trig_en     If true, the ext trig sysref sequence is enabled
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_sysref_seq_ext_trig_enable_get(adi_fpga_apollo_device_t* fpga, uint32_t* ext_trig_en);

/**
 * \brief Store fpga platform identifier name in fpga state_info
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_platform_identify(adi_fpga_apollo_device_t* fpga);

/**
 * \brief Get the number of Sh and Emb errors for a specific link.
 *
 * \param[in]   fpga            Context variable
 * \param[in]   link_sel        JRX link to get errors of \ref adi_fpga_apollo_link_sel_e
 * \param[out]  sh_err_count    Number of Sh errors for the given link
 * \param[out]  emb_err_count   Number of Emb errors for the given link
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jrx_link_error_count_get(adi_fpga_apollo_device_t* fpga, uint16_t link_sel, uint32_t* sh_err_count, uint32_t* emb_err_count);

/**
 * \brief Resets Rx Sh and Rx Emb lock error count on all active links.
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_jrx_link_error_cnt_reset(adi_fpga_apollo_device_t* fpga);

/**
 * \brief       Check if the requested memory size is within the FPGA memory limits.
 *              If exceeds, set the size to the maximum allowed.
 *
 * \param[in]   fpga            Context variable
 * \param[in]   size_bytes      Requested memory size
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_core_max_mem_size_check(adi_fpga_apollo_device_t *fpga, uint64_t size_bytes);

/**
 * \brief Enables SPI2 SDO alternate GPIO (for FMCB HMC7044 read backs)
 *
 * \param[in]   fpga            Context variable
 * \param[in]   enable           If true, use alternate SDO
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_spi2_sdo_alt_enable_set(adi_fpga_apollo_device_t* fpga, uint8_t enable);

/**
 * \brief Initialize Tx/Rx links simultaneously
 * \note This function should only be used to bring up both tx and rx links after cals.
 * 
 * Equivalent to calling both adi_fpga_apollo_core_capture_start 
 * and adi_fpga_apollo_core_transmit_start. This sequence will reinitialize
 * the links if called multiple times. To avoid bringing down the rx links
 * use adi_fpga_apollo_core_ptn_play_start / stop instead of 
 * adi_fpga_apollo_core_transmit_start / stop for consecutive transmits. 
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_core_bidir_init(adi_fpga_apollo_device_t* fpga);

/**
 * \brief Initialize Rx links
 * \note This function should only be used to bring up the rx links after cals.
 *
 * This sequence will reinitialize and maintain the rx links. Consecutive calls 
 * to adi_fpga_apollo_core_capture_start will not reinitialize the links.
 * 
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_rx_links_init(adi_fpga_apollo_device_t* fpga);

/**
 * \brief Initialize Tx links
 * \note This function should only be used to bring up the tx links after cals.
 *
 * This sequence will reinitialize and maintain the tx links. Consecutive calls
 * to adi_fpga_apollo_core_transmit_start will not reinitialize the links; 
 * however, it's recommended to use adi_fpga_apollo_core_ptn_play_start / stop.
 *
 * \param[in]   fpga            Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_tx_links_init(adi_fpga_apollo_device_t* fpga);

/**
 * \brief    Setup the FPGA internal sysref counter
 *
 * \param[in]   fpga                Context variable
 * \param[in]   glbclk_ratio        The ratio of sysref period to the global clock period.
 *
 * \return API_CMS_ERROR_OK         API Completed Successfully
 * \return <0                       Failed. \ref adi_cms_error_e for details
 */
int32_t adi_fpga_apollo_core_internal_sysref_setup(adi_fpga_apollo_device_t *fpga, uint32_t glbclk_ratio);

/**
 * \brief    Set JRx lane mapping for dual link mode
 * 
 * \param[in]   fpga                Context variable
 * \param[in]   terminal            Target terminal. \ref adi_fpga_apollo_jesd_e
 * 
 * \return API_CMS_ERROR_OK         API Completed Successfully
 * \return <0                       Failed. \ref adi_cms_error_e for details
 */
int32_t adi_fpga_apollo_core_dual_link_lane_mapping_set(adi_fpga_apollo_device_t *fpga, uint16_t terminal);

#ifndef CLIENT_IGNORE

/**
 * \brief    Set FPGA Tx pattern start address and len for links in a vector group
 * 
 * \note    This function would typically be called prior to playing a vector with adi_fpga_apollo_core_bidir_init() or 
 *          adi_fpga_apollo_core_tx_links_init().
 *
 * \param[in]   fpga               Context variable
 * \param[in]   vec_grp            Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   links              Link select mask. \ref adi_fpga_apollo_link_sel_e
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_ptn_vec_group_load(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links);

/**
 * \brief    Set the FPGA Rx memory start address for data capture
 *
 * \param[in]   fpga               Context variable
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
 */
int32_t adi_fpga_apollo_core_rx_mem_addr_set(adi_fpga_apollo_device_t *fpga);


#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_CORE_H__
