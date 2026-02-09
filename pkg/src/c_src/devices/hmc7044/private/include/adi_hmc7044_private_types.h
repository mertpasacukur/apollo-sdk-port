/*!
 * \brief     HMC7044 Private Types
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#ifndef __ADI_HMC7044_PRIVATE_TYPES_H__
#define __ADI_HMC7044_PRIVATE_TYPES_H__

#include "adi_cms_api_common.h"
#include "adi_hmc7044_types.h"

#define HMC7044_NOF_OP_CH                   14
#define HMC7044_CH_DIV_MAX                  4095
#define HMC7044_SLIP_DELAY_MAX              4095
#define HMC7044_SPI_RESET_PERIOD_US         10
#define HMC7044_DIV_RESET_PERIOD_US         5000
#define HMC7044_NOF_CLK_IN                  4
#define HMC7044_NOF_OSC_IN                  1
#define HMC7044_NOF_GPIO_MAX                4
#define HMC7044_PLL2_R_DIV_MAX              4095
#define HMC7044_HW_RESET_PERIOD_US          10
#define HMC7044_REF_CLK_FREQ_HZ_MIN         150
#define HMC7044_REF_CLK_FREQ_HZ_MAX         800000000ull
#define HMC7044_EXT_VCO_CLK_FREQ_HZ_MIN     400000000ull
#define HMC7044_EXT_VCO_CLK_FREQ_HZ_MAX     6000000000ull
#define HMC7044_VCXO_CLK_FREQ_HZ_MIN        10000000ull
#define HMC7044_VCXO_CLK_FREQ_HZ_MAX        500000000ull
#define HMC7044_PD1_CLK_FREQ_HZ_MIN         150
#define HMC7044_PD1_CLK_FREQ_HZ_MAX         50000000ull
#define HMC7044_PD2_CLK_FREQ_HZ_MIN         150
#define HMC7044_PD2_CLK_FREQ_HZ_MAX         250000000ull
#define HMC7044_LCM_CLK_FREQ_HZ_MIN         150
#define HMC7044_LCM_CLK_FREQ_HZ_MAX         123000000ull
#define HMC7044_VCO_CLK_FREQ_HZ_MIN         2650000000ull
#define HMC7044_VCO_CLK_FREQ_HZ_MAX         3550000000ull
#define HMC7044_PLL1REF_CLK_FREQ_HZ_MIN     30000000ull
#define HMC7044_PLL1REF_CLK_FREQ_HZ_MAX     40000000ull
#define HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MIN  10000000ull
#define HMC7044_PLL2REF_CLK_DB_FREQ_HZ_MAX  125000000ull

typedef enum {
    HMC7044_OP_CH_0   = 0x0001, /*  CLKOUT0  */
    HMC7044_OP_CH_1   = 0x0002, /* SCLKOUT1  */
    HMC7044_OP_CH_2   = 0x0004, /*  CLKOUT2  */
    HMC7044_OP_CH_3   = 0x0008, /* SCLKOUT3  */
    HMC7044_OP_CH_4   = 0x0010, /*  CLKOUT4  */
    HMC7044_OP_CH_5   = 0x0020, /* SCLKOUT5  */
    HMC7044_OP_CH_6   = 0x0040, /*  CLKOUT6  */
    HMC7044_OP_CH_7   = 0x0080, /* SCLKOUT7  */
    HMC7044_OP_CH_8   = 0x0100, /*  CLKOUT8  */
    HMC7044_OP_CH_9   = 0x0200, /* SCLKOUT9  */
    HMC7044_OP_CH_10  = 0x0400, /*  CLKOUT10 */
    HMC7044_OP_CH_11  = 0x0800, /* SCLKOUT11 */
    HMC7044_OP_CH_12  = 0x1000, /*  CLKOUT12 */
    HMC7044_OP_CH_13  = 0x2000, /* SCLKOUT13 */
    HMC7044_OP_CH_ALL = 0x3FFF
}adi_hmc7044_op_ch_e;

typedef enum
{
    HMC7044_CH0_CH1_EN   = 0x01,
    HMC7044_CH2_CH3_EN   = 0x02,
    HMC7044_CH4_CH5_EN   = 0x04,
    HMC7044_CH6_CH7_EN   = 0x08,
    HMC7044_CH8_CH9_EN   = 0x10,
    HMC7044_CH10_CH11_EN = 0x20,
    HMC7044_CH12_CH13_EN = 0x40,
    HMC7044_CH_ALL_EN    = 0x7F
}adi_hmc7044_ch_out_e;

typedef enum {
    HMC7044_CLK_IN_NONE = 0x0,
    HMC7044_CLK_IN_0    = 0x1,
    HMC7044_CLK_IN_1    = 0x2,
    HMC7044_CLK_IN_2    = 0x4,
    HMC7044_CLK_IN_3    = 0x8,
    HMC7044_CLK_IN_ALL  = 0XF
}adi_hmc7044_clk_in_e;

typedef enum
{
    HMC7044_LOS_TIMER_NONE       = 0,
    HMC7044_LOS_TIMER_CYCLES_2   = 1,
    HMC7044_LOS_TIMER_CYCLES_4   = 2,
    HMC7044_LOS_TIMER_CYCLES_8   = 3,
    HMC7044_LOS_TIMER_CYCLES_16  = 4,
    HMC7044_LOS_TIMER_CYCLES_32  = 5,
    HMC7044_LOS_TIMER_CYCLES_64  = 6,
    HMC7044_LOS_TIMER_CYCLES_128 = 7
}adi_hmc7044_los_timer_e;

typedef enum
{
    HMC7044_MANUAL_MODE_CLK_0 = 0,
    HMC7044_MANUAL_MODE_CLK_1 = 1,
    HMC7044_MANUAL_MODE_CLK_2 = 2,
    HMC7044_MANUAL_MODE_CLK_3 = 3
}adi_hmc7044_manual_clk_switching_e;

typedef enum
{
    HMC7044_PLL1_LOCK_ST = 1,          /*BIT 0 Indicates PLL1 Lock Status*/
    HMC7044_PLL2_LOCK_ST = 2,          /*BIT 1 Indicates PLL1 Lock Status*/
    HMC7044_PLL1_AND_PLL2_LOCK_ST = 4, /*BIT 2 Indicates PLL1 AND PLL2 Lock Status*/
}adi_hmc7044_pll_lock_status_mask_e;

typedef enum
{
    HMC7004_HO_EA_RESET_DIVIDERS = 0,
    HMC7044_HO_EA_DO_NOTHING_1   = 1,
    HMC7044_HO_EA_DO_NOTHING_2   = 2,
    HMC7044_HO_EA_DAC_ASSIT      = 3,
    HMC7044_HO_EC_LOS_GONE_0     = 0,
    HMC7044_HO_EC_ZERO_PHASE_ERR = 1,
    HMC7044_HO_EC_LOS_GONE_2     = 2,
    HMC7044_HO_EC_IMMEDIATE      = 3
}adi_hmc7044_holdover_exit_e;

typedef enum {
    HMC7044_OSCOUT_DIV_1 = 0,
    HMC7044_OSCOUT_DIV_2 = 1,
    HMC7044_OSCOUT_DIV_4 = 2,
    HMC7044_OSCOUT_DIV_8 = 3,
}adi_hmc7044_oscout_div_e;

typedef enum {
    HMC7044_OP_SIG_CH_DIV = 0x0,
    HMC7044_OP_SIG_ANALOG_DELAY = 0x1,
    HMC7044_OP_SIG_CH_PAIR = 0x2,
    HMC7044_OP_SIG_IP_VC0 = 0x3,
    HMC7044_OP_SIG_INVALID = 0x4
}adi_hmc7044_op_source_e;

typedef enum
{
    HMC7044_VCO_DISABLED         = 0,
    HMC7044_VCO_INTERNAL_3GHZ    = 1,
    HMC7044_VCO_INTERNAL_2GHZ    = 2,
    HMC7044_VCO_EXTERNAL         = 3,
    HMC7044_VCO_SEL_INVALID      = 4
}adi_hmc7044_vco_sel_e;

typedef enum
{
    HMC7044_GPI_PLL1_HOLDOVER             = 1,
    HMC7044_GPI_PLL1_REFERENCE_BIT1       = 2,
    HMC7044_GPI_PLL1_REFERENCE_BIT0       = 3,
    HMC7044_GPI_CHIP_SLEEP_MODE           = 4,
    HMC7044_GPI_ISSUE_MUTE                = 5,
    HMC7044_GPI_SELECT_VCO_TYPE           = 6,
    HMC7044_GPI_PLL2_HIGH_PERFORM_AND_VCO = 7,
    HMC7044_GPI_PLUSE_GENERATE_REQ        = 8,
    HMC7044_GPI_RESEED_REQ                = 9,
    HMC7044_GPI_RESTART_REQ               = 10,
    HMC7044_GPI_CHIP_FANOUT_MODE          = 11,
    HMC7044_GPI_SLIP_REQ                  = 13
}adi_hmc7044_gpi_op_config_e;

typedef enum
{
    HMC7044_GPO_ALARM_SIG                      = 0,
    HMC7044_GPO_SPI_SDATA                      = 1,
    HMC7044_GPO_LOS_CLKIN3_IN                  = 2,
    HMC7044_GPO_LOS_CLKIN2_IN                  = 3,
    HMC7044_GPO_LOS_CLKIN1_IN                  = 4,
    HMC7044_GPO_LOS_CLKIN0_IN                  = 5,
    HMC7044_GPO_PLL1_HOLDOVER_SIG_EN           = 6,
    HMC7044_GPO_PLL1_LOCK_DETECT_SIG           = 7,
    HMC7044_GPO_ACQ_PLL1_LOCK_SIG              = 8,
    HMC7044_GPO_PLL1_NEAR_LOCK_ACQ_STATUS      = 9,
    HMC7044_GPO_PLL2_LOCK_DETECT_SIG           = 10,
    HMC7044_GPO_SYSREF_NO_SYNC_ON_RESET        = 11,
    HMC7044_GPO_CLK_OUT_PHASE_STATUS           = 12,
    HMC7044_GPO_PLL1_PLL2_LOCK_DETECT_LOCKED   = 13,
    HMC7044_GPO_SYNC_REQ_STATUS_SIG            = 14,
    HMC7044_GPO_PLL1_ACTIVE_CLKIN_0            = 15,
    HMC7044_GPO_PLL1_ACTIVE_CLKIN_1            = 16,
    HMC7044_GPO_PLL1_HOLDOVER_ADC_RANGE_STATUS = 17,
    HMC7044_GPO_PLL1_HOLDOVER_ADC_STATUS       = 18,
    HMC7044_GPO_PLL1_VCXO_STATUS               = 19,
    HMC7044_GPO_PLL1_ACTIVE_CLKINx_STATUS      = 20,
    HMC7044_GPO_PLL1_FSM_BIT0_STATUS           = 21,
    HMC7044_GPO_PLL1_FSM_BIT1_STATUS           = 22,
    HMC7044_GPO_PLL1_FSM_BIT2_STATUS           = 23,
    HMC7044_GPO_HOLDOVER_EXIT_BIT0_PHASE       = 24,
    HMC7044_GPO_HOLDOVER_EXIT_BIT1_PHASE       = 25,
    HMC7044_GPO_CH_OUTS_FSM_BUSY               = 26,
    HMC7044_GPO_SYSREF_FSM_BIT0_STATUS         = 27,
    HMC7044_GPO_SYSREF_FSM_BIT1_STATUS         = 28,
    HMC7044_GPO_SYSREF_FSM_BIT2_STATUS         = 29,
    HMC7044_GPO_SYSREF_FSM_BIT3_STATUS         = 30,
    HMC7044_GPO_FORCE_LOGIC_1                  = 31,
    HMC7044_GPO_FORCE_LOGIC_0                  = 32,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT0_VAL = 39,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT1_VAL = 40,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT2_VAL = 41,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_AVG_BIT3_VAL = 42,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT0_VAL = 43,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT1_VAL = 44,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT2_VAL = 45,
    HMC7044_GPO_PLL1_HOLDOVER_DAC_CUR_BIT3_VAL = 46,
    HMC7044_GPO_HOLDOVER_COM_STATUS            = 61,
    HMC7044_GPO_PULSE_GEN_REQ_STATUS           = 62
}adi_hmc7044_gpo_op_config_e;

typedef enum
{
    HMC7044_SYSREF_LEVEl_SENS      = 0,
    HMC7044_SYSREF_1_PULSE         = 1,
    HMC7044_SYSREF_2_PULSE         = 2,
    HMC7044_SYSREF_4_PULSE         = 3,
    HMC7044_SYSREF_8_PULSE         = 4,
    HMC7044_SYSREF_16_PULSE        = 5,
    HMC7044_SYSREF_ALSO_16_PULSE   = 6,
    HMC7044_SYSREF_CONTINUOUS_MODE = 7
}adi_hmc7044_sysref_mode_config_e;

typedef struct {
    adi_cms_signal_type_e mode;                 /*!< Output Driver Mode */
    adi_cms_signal_impedance_type_e impedance;  /*!< Output Driver Impedance */
    uint8_t dynamic_driver_en;                  /*!< Dynamic Driver Mode Enable */
    uint8_t force_mute_en;                      /*!< Force Mute Enable */
}adi_hmc7044_op_driver_config_t;

typedef struct
{
    uint8_t pll1_lock;
    uint8_t pll1_lock_aquisition;
    uint8_t pll1_lock_detect;
    uint8_t pll1_holdover_status;
    uint8_t pll1_clkInx_status;
    uint8_t sync_request;
    uint8_t pll1_pll2_lock_detect;
    uint8_t clkoutputs_phase_status;
    uint8_t sysref_sync_status;
    uint8_t pll2_lock_detect;
}adi_hmc7044_alarm_mask_config_t;

typedef struct
{
    uint8_t exit_action;
    uint8_t exit_criteria;
    uint8_t holdover_dac;
    uint8_t adc_tracking;
    uint8_t quick_mode;
    uint8_t holdover_bw;
}adi_hmc_pll1_holdover_config_t;

typedef enum {
    IPBUFFER_INTERNAL_100_OHM_EN = 0x1,
    IPBUFFER_AC_COUPLED_MODE_EN  = 0x2,
    IPBUFFER_LVPECL_MODE_EN = 0x4,
    IPBUFFER_HIGH_Z_MODE_EN = 0x8,
    IPBUFFER_CONFIG_MAX = 0xF
}adi_hmc7044_ip_buffer_settings_e;

typedef struct {
    uint8_t ref_priority[4];
    uint64_t ref_clk_freq_hz;
    uint64_t fvcxo_clk_freq_hz;
    uint16_t output_ch;
    uint64_t output_freq_hz[HMC7044_NOF_OP_CH];
} adi_hmc7044_clk_config_t;

#ifndef CLIENT_IGNORE

typedef int32_t(*adi_hmc7044_hw_open_t)(void *user_data);
typedef int32_t(*adi_hmc7044_hw_close_t)(void *user_data);

typedef struct {
    void *                        user_data;
    adi_cms_spi_write_t           spi_write;            /*!< Function Pointer to HAL SPI write function */
    adi_cms_spi_read_t            spi_read;             /*!< Function Pointer to HAL SPI read function */
    adi_cms_delay_us_t            delay_us;             /*!< Function Pointer to HAL delay function */
    adi_hmc7044_hw_open_t         hw_open;              /*!< Function Pointer to HAL initialization function */
    adi_hmc7044_hw_close_t        hw_close;             /*!< Function Pointer to HAL De-initialization function */
    adi_hmc7044_reset_pin_ctrl_t  reset_pin_ctrl;       /*!< Function Pointer to HAL RESETB Pin Control Function */
} adi_hmc7044_private_hal_t;

typedef struct {
    uint8_t  dev_prod_id;                       /*!< Product ID */
} adi_hmc7044_info_t;

typedef struct {
    adi_hmc7044_private_hal_t  hal_info;        /*!< HAL information */
    adi_hmc7044_info_t dev_info;                /*!< DEV information */
} adi_hmc7044_private_device_t;

#endif // !CLIENT_IGNORE

#endif // !__ADI_HMC7044_PRIVATE_TYPES_H__
