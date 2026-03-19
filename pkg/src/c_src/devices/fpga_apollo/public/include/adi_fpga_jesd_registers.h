/*!
 * @brief     JESD registers definition file.
 *
 * @copyright copyright(c) 2021 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_ADS10__
 * @{
 */

#ifndef __ADI_FPGA_JESD_REGISTERS__
#define __ADI_FPGA_JESD_REGISTERS__

/*============= I N C L U D E S ============*/


/*============= D E F I N E S ==============*/
#define VERSION_ID                                         0x0100
#define VERSION_ID2                                        0x0101

#define MISC_0											   0x010A
#define AD9528_VCXO_SELECT_MASK							   0x00000004
#define SMA_J3_SEL_MASK									   0x00000070
#define FPGA_TRIG_OUT_MASK                                 0x00000100
#define AD9528_VCXO_SELECT_OFST                            2

#define DATA_ADC_EN                                        0x0106
#define RX_KEEP_LINK_MASK                                  0x000F0000
#define SKIP_RX_LINK_INIT_MASK                             0x00000400
#define RX_EXT_TRIG_EN_MASK                                0x00000002
#define RX_FSRC_EN_MASK                                    0x01000000

#define GPIO_R_1										   0x0A00
#define DUT_GPIO_R_1_MASK								   0xffffffff

#define GPIO_R_2									       0x0A01
#define DUT_GPIO_R_2_MASK								   0x00000007

#define GPIO_W_1										   0x0A10
#define DUT_GPIO_W_1_MASK								   0xffffffff

#define GPIO_W_2									       0x0A11
#define DUT_GPIO_W_2_MASK								   0x00000007

#define GPIO_D_1									       0x0A12
#define DUT_GPIO_D_1_MASK   						       0xffffffff

#define GPIO_D_2									       0x0A13
#define DUT_GPIO_D_2_MASK   						       0x00000007

#define SEQ_CTRL_1                                         0x0A14
#define SEQ_GPIO_CHANGE_CNT_MASK                           0x0000FFFF

#define SEQ_CTRL_2                                         0x0A15
#define SEQ_FIRST_TRIG_CNT_MASK                            0x0000FFFF
#define SEQ_SECOND_TRIG_CNT_MASK                           0xFFFF0000

#define SEQ_CTRL_3                                         0x0A19
#define SEQ_START_MASK                                     0x00000001
#define SEQ_EN_MASK                                        0x00000002
#define TX_SEQ_NON_FSRC_DELAY_EN_MASK                      0x00000010
#define SEQ_TX_ACCUM_RST_CNT_MASK                          0xFFFF0000

#define SEQ_CTRL_4                                         0x0A1A
#define SEQ_EXT_TRIG_EN_MASK                               0x00000001
#define SEQ_EXT_TRIG_MASK                                  0x00000010
#define SMA_J2_SEL_MASK		                               0x00000100
#define SEQ_DEBUG_MASK		                               0x00001000
#define SEQ_RX_DELAY_CNT_MASK                              0xffff0000

#define FEATURE_FLAGS                                      0x0941

#define SYSREF_GLBLCLK_RATIO                               0x093F
#define SYSREF_GLBLCLK_RATIO_MASK                          0x0000FFFF

#define SYSREF_COUNT                                       0x0944
#define SYSREF_COUNT_EN_MASK                               0x00000001
#define SYSREF_GLBLCLK_RATIO_SET_MASK                      0x00000002
#define SYSREF_COUNT_START_MASK                            0x00000004
#define SYSREF_COUNT_RUNNING_MASK                          0x00000100
#define SYSREF_CALIB_COMPLETE_MASK                         0x00000200
#define SYSREF_CAPT_PHASE_SIGNED_MASK                      0xFFFF0000

#define JESD204B_TX_CONFIG                                 0x051D
#define JESD204B_TX_SUBCLASS_MASK                          0x00000001
#define TX_REPEAT_EN_MASK                                  0x0000001E
#define TX_FSRC_SKIP_VALID_DATA_MASK                       0x00000200
#define TX_FSRC_EN_MASK                                    0x00000100

#define JESD204B_TX_LINK0_CTRL                             0x0525
#define JESD204B_TX_LINK1_CTRL                             0x0526
#define JESD204B_TX_LINK2_CTRL                             0x0527
#define JESD204B_TX_LINK3_CTRL                             0x0528
#define NS_PARAM_MASK                                      0xFF000000

#define JESD204B_TX_LINK0_FSRC_RATIO_1                     0x05B0
#define JESD204B_TX_LINK0_FSRC_RATIO_2                     0x05B1
#define JESD204B_TX_LINK1_FSRC_RATIO_1                     0x05B2
#define JESD204B_TX_LINK1_FSRC_RATIO_2                     0x05B3
#define JESD204B_TX_LINK2_FSRC_RATIO_1                     0x05B4
#define JESD204B_TX_LINK2_FSRC_RATIO_2                     0x05B5
#define JESD204B_TX_LINK3_FSRC_RATIO_1                     0x05B6
#define JESD204B_TX_LINK3_FSRC_RATIO_2                     0x05B7

#define DATAPATH_STATUS                                    0x0948
#define DATAPATH_RUNNING_MASK                              0x00000001

#define LINE_RATE_RX_CTRL                                  0x010C

#define JESD204B_RX_CONFIG                                 0x011D
#define JESD204B_RX_SUBCLASS_MASK                          0x00000001
#define RX_REPEAT_EN_MASK                                  0x0000001E

#define JESD204B_RX_LINK_COUNT                             0x011E
#define JESD204B_RX_LINK_COUNT_MASK                        0x00000003

#define JESD204B_RX_PWR_CTRL                               0x011F
#define JESD204B_RX_LINK_PD_MASK                           0x0000000F

#define JESD204B_RX_L_SCR                                  0x0120
#define JESD204B_LINKX_RX_L_PARAM_MASK                     0x1F
#define JESD204B_LINKX_RX_SCR_PARAM_MASK                   0x80

#define JESD204B_RX_NP                                     0x0121

#define JESD204B_RX_M                                      0x0122
#define JESD204B_LINK0_RX_M_PARAM_MASK                     0xFF

#define JESD204B_RX_F                                      0x0123

#define JESD204B_RX_K                                      0x0124

#define JESD204C_RX_E                                      0x0203

#define JESD204C_RX_LINK_ERROR_CNT                         0x0210
#define JESD204C_RX_SH_ERR_CNT                             0x0000ffff
#define JESD204C_RX_EMB_ERR_CNT                            0xffff0000

#define JESD204C_RX_LINK_ERROR_CNT_RESET                   0x0214
#define JESD204C_RX_SH_ERR_CNT_RESET                       0x0000000f
#define JESD204C_RX_EMB_ERR_CNT_RESET                      0x000000f0

#define CTRL_CAPTURE                                       0x0140
#define CAPTURE_START_MASK                                 0x00000002
#define CAPTURE_STOP_MASK                                  0x00000008

#define GT_RX_CAPTURE_LEN                                  0x0143 /* capture_size[31:0] */
#define GT_RX_CAPTURE_LEN_H                                0x0144 /* capture_size[63:32] */
#define GT_RX_CAPTURE_ADDR                                 0x0145 /* capture_addr[31:0] */

#define GT_RX_PRBS_CTRL                                    0x0230
#define GT_RX_PRBS_SEL_MASK                                0x0000000F
#define GT_RX_PRBS_CNT_RESET_MASK                          0x00000010
#define GT_RX_PRBS_CNT_REQ_MASK                            0x00000020

#define GT_RX_PRBS_STATUS                                  0x240
#define gt_rx_prbs_cnt_valid_mask                          0x00000001

#define GT_RX_PRBS_LOCKED                                  0x241
#define gt_rx_prbs_locked_mask                             0x00ffffff

#define GT_RX_PRBS_CYCLE_CNT1                              0x250
#define gt_rx_prbs_cycle_cnt_31_0_mask                     0xffffffff

#define GT_RX_PRBS_CYCLE_CNT2                              0x280
#define gt_rx_prbs_cycle_cnt_63_32_mask                    0xffffffff

#define GT_RX_PRBS_ERR_CNT                                 0x2B0
#define gt_rx_prbs_err_cnt_mask                            0xffffffff

#define TX_POLARITY                                        0x500
#define POLARITY_INVERTED                                  0x00FFFFFF
#define POLARITY_NOT_INVERTED                              0x00000000

#define LINE_RATE_TX_CTRL                                  0x050C

#define JESD204B_TX_LINK_COUNT                             0x051E
#define JESD204B_TX_LINK_COUNT_MASK                        0x00000003

#define JESD204B_TX_PWR_CTRL                               0x051F
#define JESD204B_TX_LINK_PD_MASK                           0x0000000F

#define JESD204B_TX_L_SCR                                  0x0520
#define JESD204B_LINK0_TX_L_PARAM_MASK                     0x0000001F
#define JESD204B_LINK1_TX_L_PARAM_MASK                     0x00001F00
#define JESD204B_LINK2_TX_L_PARAM_MASK                     0x001F0000
#define JESD204B_LINK3_TX_L_PARAM_MASK                     0x1F000000

#define JESD204B_TX_NP                                     0x0521

#define JESD204B_TX_M                                      0x0522
#define JESD204B_LINK0_TX_M_PARAM_MASK                     0xFF

#define JESD204B_TX_F                                      0x0523

#define JESD204B_TX_K                                      0x0524

#define JESD204B_TX_LINK0_CTRL                             0x0525
#define JESD204B_TX_LINK1_CTRL                             0x0526
#define JESD204B_TX_LINK2_CTRL                             0x0527
#define JESD204B_TX_LINK3_CTRL                             0x0528

#define GT_TX_PATTERN_CTRL                                 0x053A
#define GT_TX_DATA_MODE_MASK                               0x00000002
#define TX_SINGLE_LINK_FULL_MEM_MASK                       0x00000300

#define GT_TX_PATTERN_LOAD_CTRL                            0x053B
#define GT_TX_PTN_LOAD_START_MASK                          0x00000001
#define GT_TX_PTN_LOAD_STOP_MASK                           0x00000004
#define GT_TX_PTN_LOAD_RUNNING_MASK                        0x00000008
#define GT_TX_PTN_LOAD_COMPLETE_MASK                       0x00000010
#define GT_TX_PTN_LOAD_SECTION_START_MASK                  0x0000FF00
#define GT_TX_PTN_LOAD_SECTION_END_MASK                    0x00FF0000
#define GT_TX_PTN_READ_START_MASK                          0x00000020
#define GT_TX_PTN_READ_STOP_MASK                           0x00000040

#define GT_TX_PATTERN_LEN                                  0x053D
#define GT_TX_PATTERN_LEN_H                                0x053E

#define GT_TX_PATTERN_PLAY_CTRL                            0x053F
#define GT_TX_PTN_PLAY_START_MASK                          0x00000001
#define GT_TX_PTN_PLAY_STOP_MASK                           0x00000004
#define GT_TX_PTN_LINK_MASK                                0x00000F00
#define GT_TX_PTN_LINK0_MASK                               0x00000100
#define GT_TX_PTN_LINK1_MASK                               0x00000200
#define GT_TX_PTN_LINK2_MASK                               0x00000400
#define GT_TX_PTN_LINK3_MASK                               0x00000800

#define CTRL_TRANSMIT                                      0x0540
#define TRANSMIT_SKIP_DATA_MASK                            0x00000001
#define TRANSMIT_START_MASK                                0x00000002
#define TRANSMIT_STOP_MASK                                 0x00000008
#define TRANSMIT_FSRC_STOP                                 0x00000010
#define TRANSMIT_FSRC_CHANGE_RATE                          0x00000020
#define TX_KEEP_LINK_MASK                                  0x00000F00
#define TX_FSRC_LINK_MASK                                  0x000F0000

#define CTRL_TRANSMIT_FIFO                                 0x0542
#define FX3_WRITE_MODE_MASK                                0x00000004

#define GT_TX_PATTERN_ADDR                                 0x0543
#define GT_TX_PATTERN_ADDR_H                               0x0544

#define JESD204B_TX_LINK_SYNC                              0x0546
#define JESD204B_TX_LINK_SYNC_STATUS_MASK                  0x00000001
#define JESD204B_TX_LINK_SYNC_COUNT_RESET_MASK             0x00000002
#define JESD204B_TX_LINK_SYNC_COUNT_MASK                   0xFFFF0000

#define GT_TX_DRV_CTRL                                     0x0550
#define GT_TX_DIFF_CTRL_MASK                               0x0000001F
#define GT_TX_DIFF_CTRL_REG_OFST                           0
#define GT_TX_PRE_CURSOR_MASK                              0x00001F00
#define GT_TX_PRE_CURSOR_REG_OFST                          8
#define GT_TX_POST_CURSOR_MASK                             0x001F0000
#define GT_TX_POST_CURSOR_REG_OFST                         16

#define GT_TX_PRBS_CTRL                                    0x0570
#define GT_TX_PRBS_SEL_MASK                                0x000F

#define TX_LINK_DATA_LEN                                   0x0588
#define TX_LINK_DATA_LEN2                                  0x0590
#define TX_LINK_DATA_ADDR                                  0x0598
#define TX_LINK_DATA_ADDR2                                 0x05A0

#define JESD204C_TX_E                                      0x0603

#define CLK_SRC_SEL										   0x0942
#define MGT_REFCLK_SEL_MASK								   0x00000001
#define SYSREF_SEL_MASK									   0x00030000
#define SYSREF_SEL_REG_OFST                                16
#define SYSREF_IO_BUFFER_DIRECTION_MASK					   0x00040000

#define DATA_PROTOCOL_SEL                                  0x0943
#define DATA_PROTOCOL_SEL_MASK                             0x0000FFFF
#define DATA_PROTOCOL_SEL_JESD204B                         0
#define DATA_PROTOCOL_SEL_JESD204C                         1

#define BIDIR_CTRL                                         0x0947
#define BIDIR_START_MASK                                   0x00000002
#define BIDIR_STOP_MASK                                    0x00000008

#define PRODUCT_NAME                                       0x09F0
#define BOARD_NAME                                         0x09F4
#define DESIGN_EXT1                                        0x09F8
#define DESIGN_EXT2                                        0x09FC

#define DATA_CAPTURE_STATUS_INT                            0x1040
#define DATA_CAPTURE_COMPLETE_MASK                         0x00000001
#define DATA_CAPTURE_RUNNING_MASK                          0x00000002
#define DATA_CAPTURE_STATE_MASK                            0x000003FC

#define DATA_TRANSMIT_STATUS_INT                           0x1041
#define DATA_TRANSMIT_COMPLETE_MASK                        0x00000001
#define DATA_TRANSMIT_RUNNING_MASK                         0x00000002
#define DATA_TRANSMIT_STATE_MASK                           0x000003FC

#define BIDIR_STATUS_INT                                   0x1042
#define BIDIR_RUNNING_MASK                                 0x00000002
#define BIDIR_STATE_MASK                                   0x000003FC

#define STATUS_MISC                                        0x014A
#define FIFO_READ_READY_MASK                               0x00000008

#define LINE_RATE_RX_CTRL                                  0x010C
#define LINK_0_RX_DIV_MASK                                 0x00000007
#define LINK_1_RX_DIV_MASK                                 0x00000700
#define LINK_2_RX_DIV_MASK                                 0x00070000
#define LINK_3_RX_DIV_MASK                                 0x07000000

#define LINE_RATE_TX_CTRL                                  0x050C
#define LINK_0_TX_DIV_MASK                                 0x00000007
#define LINK_1_TX_DIV_MASK                                 0x00000700
#define LINK_2_TX_DIV_MASK                                 0x00070000
#define LINK_3_TX_DIV_MASK                                 0x07000000

#define SPI_MASTER_CTRL                                    0x0901
#define FMCB_GPIO_SDO_EN                                   0x00001000

#define OUTPUT_DELAY                                       0x0949
#define DELAY_SEL_MASK                                     0x0000003F
#define DELAY_FB_ENABLE_MASK                               0x00000040


#define GT_RX_LANE_MAP_CTRL                                0x0126
#define RX_LANE_MAP_CTRL_MASK                              0x00000001

#define GT_RX_LANE_MAP_0                                   0x0127
#define RX_LANE_MAP_0_MASK                                 0x0000001F

#define GT_TX_LANE_MAP_CTRL                                0x0530
#define TX_LANE_MAP_CTRL_MASK                              0x00000001

#define GT_TX_LANE_MAP_0                                   0x0531
#define TX_LANE_MAP_0_MASK                                 0x0000001F

/*============= S T R U C T S ==============*/


/*============= E X P O R T S ==============*/

#endif /*__ADI_FPGA_JESD_REGISTERS__*/
