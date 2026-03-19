/*!
 * \brief     FPGA Apollo capture functions
 *
 * \copyright copyright(c) 2023 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_APOLLO__
 */

/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_capture.h"
#include "adi_fpga_apollo_core.h"
#include "adi_utils.h"
#include "adi_fpga_apollo_rx_hw_tpl_to_samp.h"
#include "adi_fpga_apollo_rx_sw_tpl.h"


/*============= D E F I N E S ==============*/
#define TRANSFER_SIZE 1024*1024

static int32_t capture_execute_raw(adi_fpga_apollo_device_t *fpga, uint64_t cap_size_bytes);
static int32_t capture_get_raw(adi_fpga_apollo_device_t *fpga, uint8_t cap_buf[], uint32_t cap_size_bytes);
static int32_t check_link_status(adi_fpga_apollo_device_t *fpga_device);
static int32_t fifo_tracker(adi_fpga_apollo_device_t* fpga_device, uint8_t capture, uint64_t change);
static int32_t set_link_bits(adi_fpga_apollo_device_t *fpga, uint32_t *raw_frame_bits);
static int32_t calc_transf_size(adi_fpga_apollo_device_t *fpga, uint32_t *max_transf);
static int32_t create_cap_transf_buffers(adi_fpga_apollo_device_t *fpga, uint32_t *max_transf);
static int32_t expected_samples_per_conv_calc(adi_fpga_apollo_device_t *fpga, uint32_t num_64k_blks);

/*============= D A T A ====================*/

/*============= C O D E ====================*/

int32_t adi_fpga_apollo_capture_frame_populate(adi_fpga_apollo_device_t *fpga) {
    int32_t err = API_CMS_ERROR_OK;
    uint32_t frame_bytes_lcm = 0, max_lcm, raw_frame_bits = 0;
    adi_fpga_feature_flag_t feature_flags;
    adi_fpga_apollo_capture_frame_t *capture_frame;
    uint8_t i;
    uint8_t np, s, l, m;
    uint32_t sw_tpl_mult = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    if (fpga->state_info.capture_info.capture_frame.is_valid == 1) {
        /* clear if already populated */
        memset(&fpga->state_info.capture_info, 0, sizeof(adi_fpga_apollo_capture_t));
    }

    capture_frame = &fpga->state_info.capture_info.capture_frame;

    /* Get link_enabled, m, and np */

    for (i = 0; i<MAX_JESD_LINKS;i++){
        ADI_CMS_CHECK(fpga->state_info.jrx[i].valid == 0, API_CMS_ERROR_STRUCT_UNPOPULATED);
        capture_frame->link_enabled[i] = fpga->state_info.jrx[i].jesd_link_pd ? 0 : 1;
        capture_frame->link_converter_count[i] = capture_frame->link_enabled[i] ? fpga->state_info.jrx[i].jesd_m : 0;
        capture_frame->link_bits_per_sample[i] = capture_frame->link_enabled[i] ? fpga->state_info.jrx[i].jesd_np : 0;
        capture_frame->num_links_in_use += capture_frame->link_enabled[i];
    }

    /* Set fpga_link_bits*/
    err = set_link_bits(fpga, &raw_frame_bits);
    ADI_CMS_ERROR_RETURN(err);

    /* Calculate sw tpl mult */
    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);
    ADI_CMS_ERROR_RETURN(err); 

    if (!feature_flags.rx_hw_tl){
        for (i = 0; i<MAX_JESD_LINKS; i++){
            if (capture_frame->link_enabled[i]){
                np = fpga->state_info.jrx[i].jesd_np;
                s = fpga->state_info.jrx[i].jesd_s;
                l = fpga->state_info.jrx[i].jesd_l;
                m = fpga->state_info.jrx[i].jesd_m;
                ADI_CMS_CHECK(np == 0 || s == 0 || l == 0 || m == 0, API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
                sw_tpl_mult = adi_api_utils_lcm(adi_api_utils_lcm(adi_api_utils_lcm(adi_api_utils_lcm(np, s), l), m), FPGA_CAPTURE_SIZE_BLOCK_64KB) / FPGA_CAPTURE_SIZE_BLOCK_64KB;
                if (capture_frame->sw_tpl_mult == 0){
                    capture_frame->sw_tpl_mult = sw_tpl_mult;
                } else {
                    /* Make sw_tpl_mult lcm of all links */
                    capture_frame->sw_tpl_mult = adi_api_utils_lcm(capture_frame->sw_tpl_mult, sw_tpl_mult);
                }
            }
        }
    }

    /* Calculate lcm to get mult*/
    for (i = 0; i<MAX_JESD_LINKS; i++){
        if (capture_frame->link_enabled[i]){
            ADI_CMS_CHECK(raw_frame_bits == 0 || capture_frame->link_converter_count[i] == 0 || capture_frame->link_bits_per_sample[i] == 0,
                            API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
            max_lcm = adi_api_utils_lcm(raw_frame_bits,(capture_frame->link_converter_count[i]*capture_frame->link_bits_per_sample[i]));
            if (frame_bytes_lcm == 0){
                frame_bytes_lcm = max_lcm;
            } else {
                frame_bytes_lcm = adi_api_utils_lcm(frame_bytes_lcm, max_lcm);
            }
        }
    }

    capture_frame->frame_mult = frame_bytes_lcm/raw_frame_bits;

    /* Get per-link sample info, max, min, and frame size */
    capture_frame->link_min_samples_per_converter = 0xFFFFFFFF;
    capture_frame->frame_size_bytes = 0;

    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (capture_frame->link_enabled[i]) {
            ADI_CMS_CHECK(capture_frame->fpga_link_bits[i] == 0, API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
            capture_frame->frame_samples_per_conv[i] = (capture_frame->frame_mult*capture_frame->fpga_link_bits[i]) /
                                    (capture_frame->link_converter_count[i] * capture_frame->link_bits_per_sample[i]);

            if (capture_frame->frame_samples_per_conv[i] < capture_frame->link_min_samples_per_converter && capture_frame->frame_samples_per_conv[i])
                capture_frame->link_min_samples_per_converter = capture_frame->frame_samples_per_conv[i];

            capture_frame->frame_size_bytes += (capture_frame->frame_samples_per_conv[i]*capture_frame->link_converter_count[i] *
                                    capture_frame->link_bits_per_sample[i]) / 8;
        } else {
            capture_frame->frame_samples_per_conv[i] = 0;
        }
    }

    fpga->state_info.capture_info.capture_frame.is_valid = 1;

    return err;
}

static int32_t capture_execute_raw(adi_fpga_apollo_device_t *fpga, uint64_t cap_size_bytes)
{
    int32_t err;
    uint32_t num_64k_blks;
    adi_fpga_feature_flag_t feature_flags;
    uint32_t sw_tpl_mult = fpga->state_info.capture_info.capture_frame.sw_tpl_mult;

    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);
    ADI_CMS_ERROR_RETURN(err); 

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(cap_size_bytes == 0);

    /* Check if size exceeds fpga limit */
    err = adi_fpga_apollo_core_max_mem_size_check(fpga, cap_size_bytes);
    ADI_CMS_ERROR_RETURN(err);

    /* A new capture will clear the FIFO. Reset FIFO byte count */
    fifo_tracker(fpga, 1, 0);

    /* FPGA captures in chunks of 64KB. Determine number of 64KB blocks are necessary */
    num_64k_blks = (uint32_t)((cap_size_bytes / FPGA_CAPTURE_SIZE_BLOCK_64KB) +
        (cap_size_bytes % FPGA_CAPTURE_SIZE_BLOCK_64KB == 0 ? 0 : 1));
    
    if (!feature_flags.rx_hw_tl) {
        num_64k_blks = num_64k_blks % sw_tpl_mult == 0 ? num_64k_blks : (num_64k_blks + (sw_tpl_mult - (num_64k_blks % sw_tpl_mult)));
    }

    /* ADS10 FPGA link status */
    err = check_link_status(fpga);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_capture_size_set(fpga, num_64k_blks);
    ADI_CMS_ERROR_RETURN(err);

    err = adi_fpga_apollo_core_capture_start(fpga);
    ADI_CMS_ERROR_RETURN(err);

    err = fifo_tracker(fpga, 1, num_64k_blks * 64 * 1024);
    ADI_CMS_ERROR_RETURN(err);

    /* Wait for capture to finish */
    err = adi_fpga_apollo_core_capture_complete_wait(fpga);
    ADI_CMS_ERROR_RETURN(err);

    err = expected_samples_per_conv_calc(fpga, num_64k_blks);
    ADI_CMS_ERROR_RETURN(err);
    
    return API_CMS_ERROR_OK;
}


int32_t adi_fpga_apollo_capture_execute(adi_fpga_apollo_device_t *fpga, uint32_t num_samples)
{
    uint64_t capture_size_bytes;
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_apollo_capture_frame_t *cap_frame_info;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(num_samples == 0);
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.is_valid == 0, API_CMS_ERROR_STRUCT_UNPOPULATED);

    cap_frame_info = &fpga->state_info.capture_info.capture_frame; 

    capture_size_bytes = (uint64_t)(num_samples / cap_frame_info->link_min_samples_per_converter) * (uint64_t)cap_frame_info->frame_size_bytes;

    err = capture_execute_raw(fpga, capture_size_bytes);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}


static int32_t capture_get_raw(adi_fpga_apollo_device_t *fpga, uint8_t cap_buf[], uint32_t cap_size_bytes)
{

    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(cap_buf);
    ADI_CMS_INVALID_PARAM_CHECK(cap_size_bytes == 0);

    err = fifo_tracker(fpga, 0, cap_size_bytes);
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.mem_read(0, cap_size_bytes, cap_buf);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_capture_data_get(adi_fpga_apollo_device_t *fpga, uint32_t num_cap_bytes)
{

    int32_t err = API_CMS_ERROR_OK;
    uint8_t* raw_cap_buf = NULL;
    adi_fpga_feature_flag_t feature_flags;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(num_cap_bytes == 0);
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.is_valid == 0, API_CMS_ERROR_STRUCT_UNPOPULATED);

    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    raw_cap_buf = (uint8_t *)malloc(num_cap_bytes * sizeof(uint8_t));
    ADI_CMS_MEM_ALLOC_CHECK(raw_cap_buf);
    err = capture_get_raw(fpga, raw_cap_buf, num_cap_bytes); 
    ADI_CMS_ERROR_GOTO(err, end);
    // SW TPL In Progress
    if (!feature_flags.rx_hw_tl) {
        rx_jesd204_sw_tpl(&fpga->state_info, raw_cap_buf, num_cap_bytes);
    }

    err = adi_fpga_apollo_raw_cap_to_samples(&fpga->state_info, raw_cap_buf, num_cap_bytes);
    ADI_CMS_ERROR_GOTO(err, end);

end:
    ADI_CMS_MEM_ALLOC_FREE(raw_cap_buf);
    return err;
}

int32_t adi_fpga_apollo_capture_transfer_setup(adi_fpga_apollo_device_t *fpga, uint32_t *num_samples, uint32_t *max_transf)
{
    int32_t err = API_CMS_ERROR_OK;
    adi_fpga_feature_flag_t feature_flags;
    uint64_t upscaled_samples = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(num_samples);
    ADI_CMS_NULL_PTR_CHECK(max_transf);
    ADI_CMS_INVALID_PARAM_CHECK(*num_samples == 0);
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.is_valid == 0, API_CMS_ERROR_STRUCT_UNPOPULATED);

    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    if (fpga->state_info.rx[0].link_xdrc == 0) {fpga->state_info.rx[0].link_xdrc = 1;} /* TODO: update for ind links */
    if (fpga->state_info.rx[0].tot_xdrc == 0) {fpga->state_info.rx[0].tot_xdrc = 1;} /* TODO: update for ind links */

    /* If FSRC upscale num samples by FSRC ratio TODO: update for ind links */
    if (fpga->state_info.rx[0].fsrc) {
        upscaled_samples = (uint64_t)*num_samples * (uint64_t)fpga->state_info.rx[0].n / (uint64_t)fpga->state_info.rx[0].m;
    } else if (fpga->state_info.rx[0].sr) {
        upscaled_samples = *num_samples * fpga->state_info.rx[0].link_xdrc / fpga->state_info.rx[0].tot_xdrc;
    }

    if (fpga->state_info.rx[0].sr || fpga->state_info.rx[0].fsrc) {
        if (upscaled_samples > UINT32_MAX) {
            return API_CMS_ERROR_OVERFLOW;
        }
        *num_samples = (uint32_t)upscaled_samples;
    }

    /* Caculate maximum number of bytes to transfer in call to 'adi_fpga_apollo_capture_data_get' */
    err = calc_transf_size(fpga, max_transf);
    ADI_CMS_ERROR_RETURN(err);

    /* Allocate memory for transfer buffer based on max_transf */
    err = create_cap_transf_buffers(fpga, max_transf);
    ADI_CMS_ERROR_RETURN(err);

    return err;
}

int32_t adi_fpga_apollo_capture_transf_buff_num_samples_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, uint32_t *num_samples)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;
    uint16_t link_idx = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(num_samples);
    ADI_CMS_SINGLE_SELECT_CHECK(apollo_jtx_link);
    ADI_CMS_RANGE_CHECK(apollo_jtx_link, ADI_APOLLO_LINK_NONE, ADI_APOLLO_LINK_ALL);
    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (0x1 << i & apollo_jtx_link) {
            link_idx = i;
        }
    }
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.link_enabled[link_idx] == 0, API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
    ADI_CMS_RANGE_CHECK(vc_idx, 0, fpga->state_info.capture_info.capture_frame.link_converter_count[link_idx] - 1);

    *num_samples = fpga->state_info.capture_info.cap_transf_cnt[link_idx][vc_idx];

    return err;
}

int32_t adi_fpga_apollo_capture_transf_buff_num_bytes_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, uint32_t *num_bytes)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;
    uint16_t link_idx = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(num_bytes);
    ADI_CMS_SINGLE_SELECT_CHECK(apollo_jtx_link);
    ADI_CMS_RANGE_CHECK(apollo_jtx_link, ADI_APOLLO_LINK_NONE, ADI_APOLLO_LINK_ALL);
    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (0x1 << i & apollo_jtx_link) {
            link_idx = i;
        }
    }
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.link_enabled[link_idx] == 0, API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
    ADI_CMS_RANGE_CHECK(vc_idx, 0, fpga->state_info.capture_info.capture_frame.link_converter_count[link_idx] - 1);

    *num_bytes = fpga->state_info.capture_info.cap_transf_cnt[link_idx][vc_idx] * (fpga->state_info.capture_info.capture_frame.link_bits_per_sample[link_idx] == 8 ? 1 : 2);

    return err;
}

int32_t adi_fpga_apollo_capture_transf_buff_get(adi_fpga_apollo_device_t *fpga, uint16_t apollo_jtx_link, uint16_t vc_idx, int16_t cap_buf[], uint32_t num_samples)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;
    uint16_t link_idx = 0;
    uint32_t cap_buf_bytes = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_SINGLE_SELECT_CHECK(apollo_jtx_link);
    ADI_CMS_RANGE_CHECK(apollo_jtx_link, ADI_APOLLO_LINK_NONE, ADI_APOLLO_LINK_ALL);
    for (i = 0; i < MAX_JESD_LINKS; i++) {
        if (0x1 << i & apollo_jtx_link) {
            link_idx = i;
        }
    }
    ADI_CMS_CHECK(fpga->state_info.capture_info.capture_frame.link_enabled[link_idx] == 0, API_CMS_ERROR_PLATFORM_CAPTURE_INVALID_CONFIG);
    ADI_CMS_RANGE_CHECK(vc_idx, 0, fpga->state_info.capture_info.capture_frame.link_converter_count[link_idx] - 1);
    ADI_CMS_RANGE_CHECK(num_samples, 1, fpga->state_info.capture_info.cap_transf_cnt[link_idx][vc_idx]);

    cap_buf_bytes = num_samples * (fpga->state_info.capture_info.capture_frame.link_bits_per_sample[link_idx] == 8 ? 1 : 2);

	memcpy(cap_buf, fpga->state_info.capture_info.cap_transf_buff[link_idx][vc_idx], cap_buf_bytes);

    return err;
}

int32_t adi_fpga_apollo_capture_transfer_cleanup(adi_fpga_apollo_device_t *fpga)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i, j;

    ADI_CMS_NULL_PTR_CHECK(fpga);

    for (i = 0; i < MAX_JESD_LINKS; i++) {
        for (j = 0; j < fpga->state_info.capture_info.capture_frame.link_converter_count[i]; j++) {
            ADI_CMS_MEM_ALLOC_FREE(fpga->state_info.capture_info.cap_transf_buff[i][j]);
        }
        ADI_CMS_MEM_ALLOC_FREE(fpga->state_info.capture_info.cap_transf_buff[i]);
        ADI_CMS_MEM_ALLOC_FREE(fpga->state_info.capture_info.cap_transf_cnt[i]);
    }

    return err;
}

int32_t adi_fpga_apollo_capture_frame_info_get(adi_fpga_apollo_device_t* fpga, adi_fpga_apollo_capture_frame_t *cap_frame_info)
{
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(cap_frame_info);

    *cap_frame_info = fpga->state_info.capture_info.capture_frame;

    return API_CMS_ERROR_OK;
}

static int32_t check_link_status(adi_fpga_apollo_device_t* fpga_device)
{
    int32_t err;
    uint32_t fpga_reg32;
    uint8_t is_link_up;

    /* ADS10 FPGA link status */
    if (fpga_device->state_info.jrx[0].jesd_jesdv == 0) {
        return API_CMS_ERROR_OK;
    }
    err = adi_fpga_apollo_core_reg_get(fpga_device, 0x0208, &fpga_reg32);
    ADI_CMS_ERROR_RETURN(err);
    is_link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;

    /* If the link isn't already up, then start a throwaway capture to get link going */
    if (!is_link_up) {
        if (fpga_device->state_info.design_id != ADI_FPGA_APOLLO_DESIGN_STD_EVAL) {
            printf("WARNING: FGPA Rx Link is down, reinitializing...\n");
        }
        /* Start capture */
        err = adi_fpga_apollo_core_capture_size_set(fpga_device, 1);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_fpga_apollo_core_capture_start(fpga_device);
        ADI_CMS_ERROR_RETURN(err);

        /* Wait for capture to finish */
        err = adi_fpga_apollo_core_capture_complete_wait(fpga_device);
        ADI_CMS_ERROR_RETURN(err);

        err = adi_fpga_apollo_core_skip_rx_link_init_set(fpga_device, 0x1);     /* Enable skip-init to prevent Tx link from reinitializing during capture_start. */
        ADI_CMS_ERROR_RETURN(err);

        err = adi_fpga_apollo_core_tx_keep_link_mask_set(fpga_device, 0xf);  /* GroupA jtx main link, don't stop or re-start the link after init */
        ADI_CMS_ERROR_RETURN(err);
    }

    /* Recheck link if was initially down (can happen on first call after FPGA image load) */
    if (!is_link_up) {
        /* ADS10 FPGA link status */
        err = adi_fpga_apollo_core_reg_get(fpga_device, 0x0208, &fpga_reg32);
        ADI_CMS_ERROR_RETURN(err);
        is_link_up = (fpga_reg32 & 0x30) == 0x30 ? 1 : 0;
    }

    ADI_CMS_CHECK(!is_link_up, API_CMS_ERROR_LINK_DOWN);

    return API_CMS_ERROR_OK;
}

/*    FIFO Tracker   */

static int32_t fifo_tracker(adi_fpga_apollo_device_t* fpga, uint8_t capture, uint64_t change)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_CHECK(!capture && change > fpga->state_info.capture_info.fifo_queue_bytes, API_CMS_ERROR_ERROR);

    if (capture) {
        fpga->state_info.capture_info.fifo_queue_bytes = change;
    }
    else {
        fpga->state_info.capture_info.fifo_queue_bytes -= change;
    }

    return err;
}

static int32_t set_link_bits(adi_fpga_apollo_device_t *fpga, uint32_t *raw_frame_bits)
{
    uint32_t dual_link_bits, single_link_bits;
    adi_fpga_apollo_capture_frame_t *capture_frame = &fpga->state_info.capture_info.capture_frame;

    switch (fpga->state_info.design_id)
    {
        case ADI_FPGA_APOLLO_DESIGN_MM_REF:
        case ADI_FPGA_APOLLO_DESIGN_VCU128:
            dual_link_bits = 1024;
            single_link_bits = 2048;
            *raw_frame_bits = 4096;
            break;
        default:
            dual_link_bits = 2048;
            single_link_bits = 4096;
            *raw_frame_bits = 8192;
            break;
    }

    for (uint8_t a = 0; a<MAX_JESD_LINKS; a+=2) {

        if (capture_frame->link_enabled[a] && capture_frame->link_enabled[a+1]) {
            capture_frame->fpga_link_bits[a] = dual_link_bits;
            capture_frame->fpga_link_bits[a+1] = dual_link_bits;
        }
        else if (capture_frame->link_enabled[a]){
            capture_frame->fpga_link_bits[a] = single_link_bits;
            capture_frame->fpga_link_bits[a+1] = 0;
        }
        else if (capture_frame->link_enabled[a+1]){
            capture_frame->fpga_link_bits[a+1] = single_link_bits;
            capture_frame->fpga_link_bits[a] = 0;
        }
        else {
            capture_frame->fpga_link_bits[a] = 0;
            capture_frame->fpga_link_bits[a+1] = 0;
        }

    }

    return API_CMS_ERROR_OK;
}

static int32_t calc_transf_size(adi_fpga_apollo_device_t *fpga, uint32_t *max_transf)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t transf_divisor;                        // Number of bytes transfer size must be divisible by
    uint32_t transf_remainder;                      // Remainder of transfer size divided by divisor
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    adi_fpga_feature_flag_t feature_flags;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(max_transf);

    cap_frame_info = &fpga->state_info.capture_info.capture_frame;
    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);
    ADI_CMS_ERROR_RETURN(err);

    /**
     * frame_size_bytes = bytes necessary to deframe samples
     * sw_tpl_mult * fpga_link_bits[0] / 8 = bytes necessary to create framed samples for SW TPL
     */
    if (!feature_flags.rx_hw_tl)
        transf_divisor = adi_api_utils_lcm(cap_frame_info->frame_size_bytes, 
                          (cap_frame_info->sw_tpl_mult * cap_frame_info->fpga_link_bits[0]) / 8);
    else {
        transf_divisor = cap_frame_info->frame_size_bytes;
    }

    transf_remainder = TRANSFER_SIZE % transf_divisor;
    /* If remainder not 0 then reduce max transfer size */
    *max_transf = transf_remainder == 0 ? TRANSFER_SIZE : (TRANSFER_SIZE - transf_remainder);

    return err;
}

static int32_t create_cap_transf_buffers(adi_fpga_apollo_device_t *fpga, uint32_t *max_transf)
{
    /**
     * Allocate memory for capture transfer buffers
     * 
     * cap_transf_buff: 3D array of pointers to capture transfer buffers 
     * cap_transf_cnt: 2D array of counters for each capture transfer buffer
     * 
     * cap_transf_buff[link] = array of pointers to capture transfer buffers for each link
     *     link = number of links  
     * cap_transf_buff[link][vc] = array of pointers to capture transfer buffers for each vc
     *     vc = number of converters per link
     * cap_transf_buff[link][vc][cnt] = pointer to capture transfer buffer
     *     cnt = max number of bytes per vc: frame_samples_per_conv * number of cap frames * size of sample
     */
    int32_t err = API_CMS_ERROR_OK;
    uint32_t vc_per_link = 0;
    uint32_t vc_size = 0;
    adi_fpga_apollo_capture_frame_t *cap_frame_info;
    adi_fpga_apollo_capture_t *capture_info;
    uint32_t link, vc;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(max_transf);

    cap_frame_info = &fpga->state_info.capture_info.capture_frame;
    capture_info = &fpga->state_info.capture_info;

    for (link = 0; link < MAX_JESD_LINKS; link++) {
        vc_per_link = cap_frame_info->link_converter_count[link];
        /* Allocate num of vc's per link*/
        capture_info->cap_transf_buff[link] = malloc(vc_per_link * sizeof(*capture_info->cap_transf_buff[link]));
        ADI_CMS_MEM_ALLOC_CHECK(capture_info->cap_transf_buff[link]);
        capture_info->cap_transf_cnt[link] = malloc(vc_per_link * sizeof(uint32_t));
        ADI_CMS_MEM_ALLOC_CHECK(capture_info->cap_transf_cnt[link]);

        for (vc = 0; vc < vc_per_link; vc++) {
            vc_size = cap_frame_info->frame_samples_per_conv[link] * (*max_transf / cap_frame_info->frame_size_bytes) 
                                        * (cap_frame_info->link_bits_per_sample[link] == 8 ? 1 : 2);
            cap_frame_info->transf_samps_per_conv[link] = vc_size;
            /* Allocate maximum number of bytes per vc */
            capture_info->cap_transf_buff[link][vc] = malloc(vc_size * sizeof(uint8_t));
            ADI_CMS_MEM_ALLOC_CHECK(capture_info->cap_transf_buff[link][vc]);
            /* Set cnt for each vc to 0 */
            capture_info->cap_transf_cnt[link][vc] = 0;
        }
    }

    return err;
}

static int32_t expected_samples_per_conv_calc(adi_fpga_apollo_device_t *fpga, uint32_t num_64k_blks)
{

    int32_t err = API_CMS_ERROR_OK;
    uint32_t num_frames, samples_per_conv;
    adi_fpga_feature_flag_t feature_flags;
    uint32_t i = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(num_64k_blks == 0);

    num_frames = num_64k_blks * FPGA_CAPTURE_SIZE_BLOCK_64KB / fpga->state_info.capture_info.capture_frame.frame_size_bytes;
    err = adi_fpga_apollo_core_feature_flags_get(fpga, &feature_flags);

    for (i = 0; i < MAX_JESD_LINKS; i++) {
        samples_per_conv = 0;

        if (!fpga->state_info.capture_info.capture_frame.link_enabled[i]) {
            continue;
        }

        samples_per_conv = fpga->state_info.capture_info.capture_frame.frame_samples_per_conv[i] * num_frames;

        if (!feature_flags.rx_hw_tl && fpga->state_info.rx[i].fsrc) {
            samples_per_conv = ((uint64_t)samples_per_conv * (uint64_t)fpga->state_info.rx[i].m / (uint64_t)fpga->state_info.rx[i].n)
                                    + ((uint64_t)samples_per_conv * (uint64_t)fpga->state_info.rx[i].m % (uint64_t)fpga->state_info.rx[i].n ? 1 : 0);
        } else if (!feature_flags.rx_hw_tl && fpga->state_info.rx[i].sr) {
            samples_per_conv = samples_per_conv * fpga->state_info.rx[i].tot_xdrc / fpga->state_info.rx[i].link_xdrc;
        }
        fpga->state_info.capture_info.capture_frame.tot_samps_per_conv[i] = samples_per_conv;
    }

    return err;
}

int32_t adi_fpga_apollo_capture_fifo_bytes_get(adi_fpga_apollo_device_t *fpga, uint64_t *fifo_bytes)
{
    int32_t err = API_CMS_ERROR_OK;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(fifo_bytes);

    *fifo_bytes = fpga->state_info.capture_info.fifo_queue_bytes;

    return err;
}