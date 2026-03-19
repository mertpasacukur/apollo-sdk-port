/*!
 * \brief     FPGA vector group functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_VEC_GRP_H__
#define __ADI_FPGA_APOLLO_VEC_GRP_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_vec_grp_types.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLIENT_IGNORE

/**
 * \brief   Create an FPGA vector group object
 *
 * \note    Use adi_fpga_apollo_vec_grp_destroy() to free memory 
 * 
 * \param[in]   label           Brief description label for vector group
 *
 * \return      Pointer to \ref adi_fpga_apollo_vec_grp_t structure
 *              NULL if failed to create object
 */
adi_fpga_apollo_vec_grp_t *adi_fpga_apollo_vec_grp_create(char *label);

/**
 * \brief   Destroy a vector group object
 *
 * \param[in]   vec_grp         Ptr to vector group object \ref adi_fpga_apollo_vec_grp_t
 *
 * \returns     API_CMS_ERROR_OK   API completed successfully.
 */
int32_t adi_fpga_apollo_vec_grp_destroy(adi_fpga_apollo_vec_grp_t *vec_grp);

/**
 * \brief   Initialize a vector group object
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object \ref adi_fpga_apollo_vec_grp_t
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_init(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp);

/**
 * \brief   Initialize a vector group object relative to a previously loaded vector group.
 * 
 * \note        The purpose of this function is to allow loading multiple vector groups into FPGA
 *              memory and then selecting which one to play using the function \ref adi_fpga_apollo_core_ptn_vec_group_load().
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   vec_grp_prev    Ptr to a previously processed vector group object. \ref adi_fpga_apollo_vec_grp_t
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_append_init(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, adi_fpga_apollo_vec_grp_t *vec_grp_prev);


/**
 * \brief   Clears the start address and len for all links in a vector group
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object \ref adi_fpga_apollo_vec_grp_t
 *
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_clear(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp);

/**
 * \brief       Get the vector start address for the selected link in a vector group
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   link            Link select. Only one link selected per call. \ref adi_fpga_apollo_link_sel_e
 * \param[out]  addr            Ptr to return start address of vector
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_start_addr_get(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t link, uint64_t *addr);

/**
 * \brief   Set the vector start address for the selected link in a vector group
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   links           Link select. Programs all links selected. \ref adi_fpga_apollo_link_sel_e
 * \param[in]   addr            Start address of vector
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_start_addr_set(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t addr);

/**
 * \brief   Get the vector length (in bytes) for the selected link in a vector group
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   link            Link select. Only one link selected per call. \ref adi_fpga_apollo_link_sel_e
 * \param[out]  len             Ptr to return len of vector (in bytes)
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_len_get(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t link, uint64_t *len);

/**
 * \brief   Set the vector length (in bytes) for the selected link in a vector group
 *
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   links           Link select. Programs all links selected. \ref adi_fpga_apollo_link_sel_e
 * \param[in]   len             Length of vector (in bytes)
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_len_set(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t len);

/**
 * \brief   Increments the vector length (in bytes) for the selected link in a vector group
 *
 * \note    This function is typically called when processing large vectors in chunks.
 * 
 * \param[in]   fpga            Context variable
 * \param[in]   vec_grp         Ptr to vector group object containing per link vector playback info \ref adi_fpga_apollo_vec_grp_t
 * \param[in]   links           Link select. Programs all links selected. \ref adi_fpga_apollo_link_sel_e
 * \param[in]   incr            Add this number to current len of vector (in bytes)
 * 
 * \return      API_CMS_ERROR_OK   API Completed Successfully
*/
int32_t adi_fpga_apollo_vec_grp_len_incr(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_vec_grp_t *vec_grp, uint16_t links, uint64_t incr);

#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_VEC_GRP_H__
