/*!
 * \brief     FPGA Apollo Clock types.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup  ADI_FPGA_APOLLO_CLK
 * \ingroup     ADI_FPGA_APOLLO
 * @{
 */

#ifndef __ADI_FPGA_APOLLO_CLK_TYPES_H__
#define __ADI_FPGA_APOLLO_CLK_TYPES_H__

/*============= I N C L U D E S ============*/
#include "adi_cms_api_common.h"
#include "adi_fpga_apollo_common_types.h"


typedef enum {
    ADI_FPGA_APOLLO_DIV_1   = 0x0,                        /*!< /1 */
    ADI_FPGA_APOLLO_DIV_2   = 0x1,                        /*!< /2 */
    ADI_FPGA_APOLLO_DIV_4   = 0x2,                        /*!< /4 */
    ADI_FPGA_APOLLO_DIV_8   = 0x3,                        /*!< /8 */
} adi_fpga_apollo_clk_div_sel;

/**
* \brief FPGA clk parameters per link
*/
typedef struct {
	uint32_t line_rate; /*! Line Rate KHz */
	uint8_t	link_div; 	/*! Line rate divide ratio 0:/1 | 1:/2 | 2:/4 | 3:/8 */
	uint8_t ver;		/*! 0:204B, 1:204C */
	uint16_t terminal;	/*! 0:JRX, 1:JTX */
	uint8_t side;		/*! 0:Side A, 1:Side B *///
} adi_fpga_clk_div_t;

/**
* \brief FPGA clk parameters
*/
typedef struct {
	adi_fpga_clk_div_t clk_div[MAX_JESD_LINKS];     /*!< Parameters per link */
	uint32_t max_link_rate_khz;                     /*!< Maximum line rate */ 
	uint32_t min_link_div;                          /*!< Minimum FPGA link div required */
    uint8_t is_prbs;                                /*!< 0:False 1:True */
    uint8_t ref_clk_div;                            /*!< Cumulative reference clk divider */ 
} adi_fpga_clk_info_t;

#endif // !__ADI_FPGA_APOLLO_CLK_TYPES_H__
/*! @} */