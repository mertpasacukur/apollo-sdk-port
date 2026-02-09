
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

#ifndef __JESD_REGISTERS__
#define __JESD_REGISTERS__

/*============= I N C L U D E S ============*/


/*============= D E F I N E S ==============*/
#define DATA_ADC_EN                                        0x0106
#define RX_KEEP_LINK_MASK                                  0x000F0000
#define SKIP_RX_LINK_INIT_MASK                             0x00000400
#define RX_EXT_TRIG_EN_MASK                                0x00000002

#define LINE_RATE_RX_CTRL                                  0x010C

#define JESD204B_RX_CONFIG                                 0x011D
#define JESD204B_RX_SUBCLASS_MASK                          0x00000001

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

#define CTRL_CAPTURE                                       0x0140
#define CAPTURE_START_MASK                                 0x00000002
#define CAPTURE_STOP_MASK                                  0x00000008

#define GT_RX_CAPTURE_LEN                                  0x0143 /* capture_size[31:0] */
#define GT_RX_CAPTURE_LEN_H                                0x0144 /* capture_size[63:32] */

#define LINE_RATE_TX_CTRL                                  0x050C

#define JESD204B_TX_CONFIG                                 0x051D
#define JESD204B_TX_SUBCLASS_MASK                          0x00000001

#define JESD204B_TX_LINK_COUNT                             0x051E

#define JESD204B_TX_PWR_CTRL                               0x051F
#define JESD204B_TX_LINK_PD_MASK                           0x0000000F

#define JESD204B_TX_L_SCR                                  0x0520

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
#define TX_KEEP_LINK_MASK                                  0x00000F00

#define CTRL_TRANSMIT_FIFO                                 0x0542
#define FX3_WRITE_MODE_MASK                                0x00000004

#define GT_TX_PATTERN_ADDR                                 0x0543
#define GT_TX_PATTERN_ADDR_H                               0x0544

#define JESD204B_TX_LINK_SYNC                              0x0546
#define JESD204B_TX_LINK_SYNC_STATUS_MASK                  0x00000001
#define JESD204B_TX_LINK_SYNC_COUNT_RESET_MASK             0x00000002
#define JESD204B_TX_LINK_SYNC_COUNT_MASK                   0xFFFF0000

#define GT_TX_DRV_CTRL                                     0x0550

#define GT_TX_PRBS_CTRL                                    0x0570

#define JESD204C_TX_E                                      0x0603

#define DATA_PROTOCOL_SEL                                  0x0943
#define DATA_PROTOCOL_SEL_MASK                             0x0000FFFF
#define DATA_PROTOCOL_SEL_JESD204B                         0
#define DATA_PROTOCOL_SEL_JESD204C                         1

#define BIDIR_CTRL                                         0x947
#define BIDIR_START_MASK                                   0x00000002
#define BIDIR_STOP_MASK                                    0x00000008

#define DATA_CAPTURE_STATUS_INT                            0x014A
#define DATA_CAPTURE_COMPLETE_MASK                         0x00000008
#define DATA_CAPTURE_RUNNING_MASK                          0x00000002
#define DATA_CAPTURE_STATE_MASK                            0x000003FC

#define DATA_TRANSMIT_STATUS_INT                           0x1041
#define DATA_TRANSMIT_COMPLETE_MASK                        0x00000001
#define DATA_TRANSMIT_RUNNING_MASK                         0x00000002
#define DATA_TRANSMIT_STATE_MASK                           0x000003FC

#define BIDIR_STATUS_INT                                   0x1042
#define BIDIR_RUNNING_MASK                                 0x00000002
#define BIDIR_STATE_MASK                                   0x000003FC

/*============= S T R U C T S ==============*/


/*============= E X P O R T S ==============*/

#endif /*__JESD_REGISTERS__*/
