/*!
 * \brief     FPGA Apollo GPIO types.
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

#ifndef __ADI_FPGA_APOLLO_GPIO_TYPES_H__
#define __ADI_FPGA_APOLLO_GPIO_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"


/*============= D E F I N E S ==============*/

#define ADI_FPGA_CMOS_GPIO_NUM                  35  //  Connects to Apollo CMOS Pads
#define ADI_FPGA_GPIO_NUM                       51  //  GPIO[34:00] + CMOS_SYNCPADS[50:35]
#define ADI_FPGA_AUX_GPIO_NUM                   6

#define REG_GPIO_R_1_ADDR                       0x0A00      // dut_gpio_r[31:00]
#define REG_GPIO_R_2_ADDR                       0x0A01      // dut_gpio_r[34:32]
#define REG_GPIO_W_1_ADDR                       0x0A02      // dut_gpio_w[31:00]
#define REG_GPIO_W_2_ADDR                       0x0A03      // dut_gpio_w[34:32]
#define REG_GPIO_D_1_ADDR                       0x0A04 	    // dut_gpio_d[31:00]
#define REG_GPIO_D_2_ADDR                       0x0A05 	    // dut_gpio_d[34:32]

#define REG_DUT_SYNCB_CTRL_ADDR                 0x0A0A
#define BF_DUT_SYNCINB_AB_SEL_INFO              0x0A0A, 0x0200      // dut_syncinb_ab_sel[1:0]
#define BF_DUT_SYNCOUTB_AB_SEL_INFO             0x0A0A, 0x0202      // dut_syncoutb_ab_sel[3:2]

#define REG_DUT_SYNCB_GPIO_R_ADDR               0x0A0B
#define BF_DUT_SYNCINB_GPIO_R_INFO              0x0A0B, 0x0800      // dut_syncinb_gpio_r[7:0]
#define BF_DUT_SYNCOUTB_GPIO_R_INFO             0x0A0B, 0x0808      // dut_syncoutb_gpio_r[15:8]

#define REG_DUT_SYNCB_GPIO_W_ADDR               0x0A0C
#define BF_DUT_SYNCINB_GPIO_W_INFO              0x0A0C, 0x0800      // dut_syncinb_gpio_w[7:0]
#define BF_DUT_SYNCOUTB_GPIO_W_INFO             0x0A0C, 0x0808      // dut_syncoutb_gpio_w[15:8]

#define REG_DUT_SYNCB_GPIO_D_ADDR               0x0A0D
#define BF_DUT_SYNCINB_GPIO_D_INFO              0x0A0D, 0x0800      // dut_syncinb_gpio_d[7:0]
#define BF_DUT_SYNCOUTB_GPIO_D_INFO             0x0A0D, 0x0808      // dut_syncoutb_gpio_d[15:8]

#define REG_AUX_GPIO_R_ADDR                     0x0A06
#define REG_AUX_GPIO_W_ADDR                     0x0A07
#define REG_AUX_GPIO_D_ADDR                     0x0A08



/*!
* \brief Enumerates SYNCOUTB_AB_SEL_X selection between a JESD signal or a GPIO.
*/
typedef enum {
    ADI_FPGA_SYNCOUTB_JESD_FMC  = 0,             /*!< Apollo SYNCOUTB_X_P/N connects to FPGA's JESD SYNCOUTB_X_P/N signal. */
    ADI_FPGA_SYNCOUTB_GPIO_FMC  = 1              /*!< Apollo SYNCOUTB_X_P/N connects to FPGA's GPIO pin. */
} adi_fpga_apollo_syncoutb_src_sel_e;

/*!
 * \brief Enumerates SYNCOUTB_AB_SEL_X Link Selection for both sides, A and B.
 */
typedef enum {
    ADI_FPGA_SYNCOUTB_LINK_AB_0 = 0x0,               /*!< Select Link 0 on both sides, A and B for SYNCOUTB_A/B. */
    ADI_FPGA_SYNCOUTB_LINK_AB_1 = 0x1                /*!< Select Link 1 on both sides, A and B for SYNCOUTB_A/B. */
} adi_fpga_apollo_syncoutb_link_sel_e;

/*!
* \brief Enumerates SYNCINB_AB_SEL_X selection between a JESD signal or a GPIO.
*/
typedef enum {
    ADI_FPGA_SYNCINB_JESD_FMC  = 0,             /*!< Apollo SYNCINB_X_P/N connects to FPGA's JESD SYNCINB_X_P/N signal. */
    ADI_FPGA_SYNCINB_GPIO_FMC  = 1              /*!< Apollo SYNCINB_X_P/N connects to FPGA's GPIO pin. */
} adi_fpga_apollo_syncinb_src_sel_e;

/*!
 * \brief Enumerates SYNCINB_AB_SEL_X Link Selection for both sides, A and B.
 */
typedef enum {
    ADI_FPGA_SYNCINB_LINK_AB_0 = 0x0,               /*!< Select Link 0 on both sides, A and B for SYNCINB_A/B. */
    ADI_FPGA_SYNCINB_LINK_AB_1 = 0x1                /*!< Select Link 1 on both sides, A and B for SYNCINB_A/B. */
} adi_fpga_apollo_syncinb_link_sel_e;


#endif // !__ADI_FPGA_APOLLO_GPIO_TYPES_H__
/*! @} */
