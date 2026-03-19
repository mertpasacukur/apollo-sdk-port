/*!
 * \brief   FPGA vector group functions.  
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
#include "adi_utils.h"
#include "adi_fpga_apollo_vec_grp.h"

adi_fpga_apollo_vec_grp_t *adi_fpga_apollo_vec_grp_create(char *label)
{
    if (label == NULL) {
        return NULL;
    }

    adi_fpga_apollo_vec_grp_t *vec_grp = (adi_fpga_apollo_vec_grp_t *)malloc(sizeof(adi_fpga_apollo_vec_grp_t));
    if (vec_grp == NULL) {
        return NULL;
    }

    strncpy(vec_grp->label, label, MAX_VEC_GROUP_LABEL_LEN);

    return vec_grp;
}

int32_t adi_fpga_apollo_vec_grp_destroy(adi_fpga_apollo_vec_grp_t *vec_grp)
{
    if (vec_grp == NULL) {
        return API_CMS_ERROR_MEM_ALLOC;
    }

    free(vec_grp);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_init(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);

    uint64_t base_addr = (fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_VCU128 ||
                          fpga->state_info.design_id == ADI_FPGA_APOLLO_DESIGN_MM_REF)
                             ? BLOCK_RAM_TX_VEC_ADDR
                             : HBM_MEM_TX_VEC_ADDR;

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        vec_grp->vec[link_idx].base_addr = base_addr;
        vec_grp->vec[link_idx].len = 0;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_append_init(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, adi_fpga_apollo_vec_grp_t *vec_grp_prev)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_NULL_PTR_CHECK(vec_grp_prev);

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        vec_grp->vec[link_idx].base_addr = vec_grp_prev->vec[link_idx].base_addr + vec_grp_prev->vec[link_idx].len;
        vec_grp->vec[link_idx].len = 0;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_clear(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);

    strncpy(vec_grp->label, "?", MAX_VEC_GROUP_LABEL_LEN);
    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        vec_grp->vec[link_idx].base_addr = 0;
        vec_grp->vec[link_idx].len = 0;
    }
    
    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_start_addr_get(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t link, uint64_t *addr)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_NULL_PTR_CHECK(addr);
    ADI_CMS_MAX_SELECT_CHECK(link, ADI_FPGA_APOLLO_LINK_ALL);
    ADI_CMS_SINGLE_SELECT_CHECK(link);

    link_idx  = adi_api_utils_select_lsb_get(link);

    *addr = vec_grp->vec[link_idx].base_addr;

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_start_addr_set(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t addr)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_MAX_SELECT_CHECK(links, ADI_FPGA_APOLLO_LINK_ALL);

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        if ((1 << link_idx) & links) {
            vec_grp->vec[link_idx].base_addr = addr;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_len_get(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t link, uint64_t *len)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_NULL_PTR_CHECK(len);
    ADI_CMS_MAX_SELECT_CHECK(link, ADI_FPGA_APOLLO_LINK_ALL);
    ADI_CMS_SINGLE_SELECT_CHECK(link);
    
    link_idx  = adi_api_utils_select_lsb_get(link);

    *len = vec_grp->vec[link_idx].len;

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_len_set(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t len)
{
    uint16_t link_idx;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_MAX_SELECT_CHECK(links, ADI_FPGA_APOLLO_LINK_ALL);

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        if ((1 << link_idx) & links) {
            vec_grp->vec[link_idx].len = len;
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_vec_grp_len_incr(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t incr)
{
    uint16_t link_idx;
    
    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(vec_grp);
    ADI_CMS_MAX_SELECT_CHECK(links, ADI_FPGA_APOLLO_LINK_ALL);

    for (link_idx = 0; link_idx < MAX_JESD_LINKS; link_idx++) {
        if ((1 << link_idx) & links) {
            vec_grp->vec[link_idx].len += incr;
        }
    }

    return API_CMS_ERROR_OK;
}