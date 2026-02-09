/*!
 * \brief     FPGA Apollo GPIO Connectivity functions.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_FPGA_APOLLO_GPIO
 * \ingroup     ADI_FPGA_APOLLO
 * @{
 */
#ifndef __ADI_FPGA_APOLLO_GPIO_H__
#define __ADI_FPGA_APOLLO_GPIO_H__

/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_types.h"
#include "adi_fpga_apollo_gpio_types.h"

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief  Sets the FPGA GPIO's direction as OUTPUT and drives the GPIO pin to given state.
 *
 * \param[in] fpga              Context variable - Pointer to the FPGA device data structure.
 * \param[in] gpio_index        GPIO num : 0-31.
 *                              Connects to Apollo's CMOS GPIOs [0-30].
 * \param[in] gpio_state        GPIO's driven state.
 *                              1 (HIGH) or 0 (LOW).
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_gpio_output_set(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t gpio_state);

/**
 * \brief  Sets the FPGA GPIO directiosn as OUTPUT and drives the GPIO pins to given state.
 *
 * \param[in] fpga              Context variable - Pointer to the FPGA device data structure.
 * \param[in] gpio_mask         Mask of CMOS pins to set
 * \param[in] gpio_state        GPIO's driven state.
 *                              1 (HIGH) or 0 (LOW).
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_gpio_output_word_set(adi_fpga_apollo_device_t *fpga, uint64_t gpio_mask, uint64_t gpio_state);

/**
 * \brief  Sets the FPGA GPIO's direction as INPUT and reads the GPIO pin's current state.
 *
 * \param[in] fpga              Context variable - Pointer to the FPGA device data structure.
 * \param[in] gpio_index        GPIO num : 0-31.
 *                              Connects to Apollo's CMOS GPIOs [0-30].
 * \param[out] gpio_state       Current state of GPIO indexed by 'gpio_index'.
 *                              1 (HIGH) or 0 (LOW).
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_gpio_input_get(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t *gpio_state);

/**
 * \brief   Switches the Apollo SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) signal to corresponding
 *          SYNCINB CMOS GPIO or SYNCINB JESD on the FPGA.
 *
 * \param[in] fpga                  Context variable - Pointer to the FPGA device data structure.
 *
 * \param[in] syncinb_scr_sel       Source selection for the Apollo SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) connections.
 *                                  0: Apollo SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) switches to FPGA's JESD SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) signal.
 *                                  1: Apollo SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) switches to FPGA's CMOS GPIO pins.
 *                                  Refer \ref adi_fpga_apollo_syncinb_src_sel_e for valid options.
 *
 * \param[in] syncinb_link_sel      Link selection for the Apollo SYNCINB_Ax_P/N (and SYNCINB_Bx_P/N) connections.
 *                                  0: Apollo SYNCINB_A0_P/N (and SYNCINB_B0_P/N) can be switched to corresponding syncinb_scr_sel path.
 *                                  1: Apollo SYNCINB_A1_P/N (and SYNCINB_B1_P/N) can be switched to corresponding syncinb_scr_sel path.
 *                                  Refer \ref adi_fpga_apollo_syncinb_link_sel_e for valid options.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_syncinb_ab_sel(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_syncinb_src_sel_e syncinb_scr_sel, adi_fpga_apollo_syncinb_link_sel_e syncinb_link_sel);

/**
 * \brief   Switches the Apollo SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) signal to corresponding
 *          SYNCOUTB CMOS GPIO or SYNCOUTB JESD on the FPGA.
 *
 * \param[in] fpga                  Context variable - Pointer to the FPGA device data structure.
 * \param[in] syncoutb_scr_sel      Source selection for the Apollo SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) connections.
 *                                  0: Apollo SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) switches to FPGA's JESD SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) signal.
 *                                  1: Apollo SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) switches to FPGA's CMOS GPIO pin.
 *                                  Refer \ref adi_fpga_apollo_syncoutb_src_sel_e for valid options.
 *
 * \param[in] syncoutb_link_sel     Link selection for the Apollo SYNCOUTB_Ax_P/N (and SYNCOUTB_Bx_P/N) connections.
 *                                  0: Apollo SYNCOUTB_A0_P/N (and SYNCOUTB_B0_P/N) can be switched to corresponding syncoutb_scr_sel path.
 *                                  1: Apollo SYNCOUTB_A1_P/N (and SYNCOUTB_B1_P/N) can be switched to corresponding syncoutb_scr_sel path.
 *                                  Refer \ref adi_fpga_apollo_syncoutb_link_sel_e for valid options.
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_syncoutb_ab_sel(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_syncoutb_src_sel_e syncoutb_scr_sel, adi_fpga_apollo_syncoutb_link_sel_e syncoutb_link_sel);

/**
 * \brief  Sets the FPGA AUX GPIO's direction as INPUT and reads the GPIO pin's current state.
 *
 * \param[in]  fpga             Context variable - Pointer to the FPGA device data structure.
 * \param[in]  gpio_index       GPIO num : 0-5.
 * \param[out] gpio_state       Current state of GPIO indexed by 'gpio_index'.
 *                              1 (HIGH) or 0 (LOW).
 *
 * \return API_CMS_ERROR_OK                     API Completed Successfully
 * \return <0                                   Failed. \ref adi_cms_error_e for details.
 */
int32_t adi_fpga_apollo_gpio_aux_input_get(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t *gpio_state);

#ifndef CLIENT_IGNORE

#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif

#endif // !__ADI_FPGA_APOLLO_GPIO_H__
/*! @} */
