/*!
 * \brief     Basic FPGA Apollo types
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_FPGA_APOLLO_CORE_TYPES_H__
#define __ADI_FPGA_APOLLO_CORE_TYPES_H__

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif
#include "adi_cms_api_common.h"


#define NUM_JTX_LANES              24
#define FPGA_MAX_CAPTURE_SIZE      65536  // 4GiB/64KiB
#define FPGA_MAX_NAME_LENGTH	   16
#define FPGA_MULTI_MODE_MEM_SIZE   0x100000 // 1MB
#define FPGA_STD_MEM_SIZE          0x80000000 // 2GB

/*!
 * \brief PRBS Pattern select
 */
typedef enum {
	PRBS_OFF = 0,
	PRBS_7   = 1,
	PRBS_9   = 2,
	PRBS_15  = 3,
	PRBS_23  = 4,
	PRBS_31  = 5,
} adi_apollo_fpga_prbs_e;

/*!
 * \brief System output indicator function (SMA J3 on ADS10)
 */
typedef enum {
	ADI_FPGA_APOLLO_RX_SYS_READY_INDICATOR = 0,		/*!< RX System Ready Indicator */
	ADI_FPGA_APOLLO_TX_PAT_START_INDICATOR = 1		/*!< TX Start of pattern indicator. 0 to 1 transition. */
} adi_fpga_apollo_sys_indicator_e;


/*!
 * \brief ADS10 SYSREF source selection
 */
typedef enum {
	ADI_FPGA_APOLLO_SYSREF_SRC_AD9528 = 0,		/*!< Use ADS10 on-board AD9528 to generate SYSREF */
	ADI_FPGA_APOLLO_SYSREF_SRC_FMC = 1,			/*!< Use SYSREF from FMC connector */
	ADI_FPGA_APOLLO_SYSREF_SRC_EXT = 2,			/*!< Use external input for SYSREF (sma J5 on ADS10) */
} adi_fpga_apollo_sysref_source_e;

/*!
 * \brief FPGA Design
 */
typedef enum {
	ADI_FPGA_APOLLO_DESIGN_STD_EVAL     = 0,
	ADI_FPGA_APOLLO_DESIGN_RAW_EVAL     = 1,
	ADI_FPGA_APOLLO_DESIGN_MM_REF       = 2,
	ADI_FPGA_APOLLO_DESIGN_MCS          = 3,
	ADI_FPGA_APOLLO_DESIGN_VCU128		= 4
} adi_fpga_design_identifier_e;

/*!
 * \brief  Defines FPGA JESD Parameters
 */
typedef struct {
	uint8_t valid;							    /*!< 1 if struct has been populated by profile */
	uint8_t jesd_l;                             /*!< No of lanes */
	uint8_t jesd_f;                             /*!< No of octets in a frame */
	uint8_t jesd_m;                             /*!< No of converters */
	uint8_t jesd_s;                             /*!< No of samples */
	uint8_t jesd_hd;                            /*!< High Density */
	uint16_t jesd_k;                            /*!< No of frames for a multi-frame */
	uint8_t jesd_e;								/*!< No of multiblocks in EMB */
	uint8_t jesd_n;                             /*!< Converter resolution */
    uint8_t jesd_np;                            /*!< Bit packing sample */
	uint8_t jesd_cf;                            /*!< Parameter CF */
	uint8_t jesd_cs;                            /*!< Parameter CS */
	uint8_t jesd_subclass;                      /*!< Subclass */
	uint8_t jesd_scr;                           /*!< Scramble enable */
	uint8_t jesd_jesdv;                         /*!< Version (0:204B, 1:204C, 2:204H) */
	uint8_t jesd_dual_link;                     /*!< Dual or single link */
	uint8_t jesd_mode_s_sel;                    /*!< JESD mode S value */
	uint8_t jesd_link_pd;                       /*!< JESD link power down mask: 1 = link powered down */
    uint8_t jesd_ns;                            /*!< No of samples per conv clock (for FSRC) */
	uint8_t base_lane_id;					    /*!< Starting Lane ID */
}adi_fpga_jesd_param_t;

/*!
 * \brief  
 */
typedef struct {
	uint32_t n;			    /*! N value for fsrc */
	uint32_t m;			    /*! M value for fsrc */
	uint32_t link_xdrc;		/*! Decimation ratio for a given link */	
	uint32_t tot_xdrc;		/*! Total decimation ratio */
	bool fsrc;			    /*! FSRC enabled */
	bool sr;			    /*! Sample Repeat Enabled */
} adi_fpga_sr_fsrc_param_t;

/*!
 * \brief  Defines FPGA Feature Flags.
 */
typedef struct {
	bool raw_data;			/*! Raw Data Flag, indicates data output conforms to raw data standard format. */
	bool spi_clk_adj;		/*! Adjustable SPI Clock Flag, indicates adjustable SPI clock frequency is supported. */
	bool dl_demo;			/*! Deterministic Latency Demo Flag, indicates DL Demo circuit is supported. */
	bool rx_hw_tl;			/*! RX Hardware TPL(Transport Layer) Feature Flag. If True, FPGA handle Rx TPL else SW.*/
	bool tx_hw_tl;			/*! TX Hardware TPL(Transport Layer) Feature Flag. If True, FPGA handle Tx TPL else SW.*/
	bool rx_hw_fsrc;		/*! RX Hardware FSRC(Fractional Sample Rate Conversion) Feature Flag. */
	bool tx_hw_fsrc;		/*! TX Hardware FSRC(Fractional Sample Rate Conversion) Feature Flag. */
	bool rx_capture_delay;   /*! RX Sequencer Capture Delay Feature Flag. */
} adi_fpga_feature_flag_t;


#endif // !__ADI_FPGA_APOLLO_CORE_TYPES_H__
