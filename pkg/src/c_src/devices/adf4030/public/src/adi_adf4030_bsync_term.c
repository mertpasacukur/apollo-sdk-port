/*!
 * \brief     Source file implementing APIs for ADF4030's BSYNC Termination functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADF4030_BSYNC_TERM
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_bsync_term.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_bsync_term_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_term_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);

    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG0040___AC_COUPLED_0_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___CON_RL_DRV_0_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___CON_RL_RCV_0_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___FLOAT_ON_DRV_0_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___FLOAT_ON_RCV_0_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG0042___AC_COUPLED_1_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___CON_RL_DRV_1_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___CON_RL_RCV_1_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___FLOAT_ON_DRV_1_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___FLOAT_ON_RCV_1_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0044___AC_COUPLED_2_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___CON_RL_DRV_2_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___CON_RL_RCV_2_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___FLOAT_ON_DRV_2_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___FLOAT_ON_RCV_2_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0046___AC_COUPLED_3_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___CON_RL_DRV_3_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___CON_RL_RCV_3_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___FLOAT_ON_DRV_3_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___FLOAT_ON_RCV_3_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0048___AC_COUPLED_4_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___CON_RL_DRV_4_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___CON_RL_RCV_4_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___FLOAT_ON_DRV_4_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___FLOAT_ON_RCV_4_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG004a___AC_COUPLED_5_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___CON_RL_DRV_5_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___CON_RL_RCV_5_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___FLOAT_ON_DRV_5_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___FLOAT_ON_RCV_5_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG004c___AC_COUPLED_6_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___CON_RL_DRV_6_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___CON_RL_RCV_6_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___FLOAT_ON_DRV_6_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___FLOAT_ON_RCV_6_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG004e___AC_COUPLED_7_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___CON_RL_DRV_7_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___CON_RL_RCV_7_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___FLOAT_ON_DRV_7_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___FLOAT_ON_RCV_7_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG0050___AC_COUPLED_8_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___CON_RL_DRV_8_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___CON_RL_RCV_8_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___FLOAT_ON_DRV_8_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___FLOAT_ON_RCV_8_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG0052___AC_COUPLED_9_set(adf4030, config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___CON_RL_DRV_9_set(adf4030, config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___CON_RL_RCV_9_set(adf4030, config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___FLOAT_ON_DRV_9_set(adf4030, config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___FLOAT_ON_RCV_9_set(adf4030, config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_bsync_term_get(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_term_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);

    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG0040___AC_COUPLED_0_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___CON_RL_DRV_0_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___CON_RL_RCV_0_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___FLOAT_ON_DRV_0_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0040___FLOAT_ON_RCV_0_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG0042___AC_COUPLED_1_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___CON_RL_DRV_1_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___CON_RL_RCV_1_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___FLOAT_ON_DRV_1_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0042___FLOAT_ON_RCV_1_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0044___AC_COUPLED_2_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___CON_RL_DRV_2_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___CON_RL_RCV_2_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___FLOAT_ON_DRV_2_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0044___FLOAT_ON_RCV_2_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0046___AC_COUPLED_3_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___CON_RL_DRV_3_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___CON_RL_RCV_3_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___FLOAT_ON_DRV_3_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0046___FLOAT_ON_RCV_3_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0048___AC_COUPLED_4_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___CON_RL_DRV_4_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___CON_RL_RCV_4_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___FLOAT_ON_DRV_4_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0048___FLOAT_ON_RCV_4_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG004a___AC_COUPLED_5_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___CON_RL_DRV_5_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___CON_RL_RCV_5_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___FLOAT_ON_DRV_5_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004a___FLOAT_ON_RCV_5_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG004c___AC_COUPLED_6_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___CON_RL_DRV_6_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___CON_RL_RCV_6_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___FLOAT_ON_DRV_6_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004c___FLOAT_ON_RCV_6_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG004e___AC_COUPLED_7_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___CON_RL_DRV_7_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___CON_RL_RCV_7_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___FLOAT_ON_DRV_7_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004e___FLOAT_ON_RCV_7_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG0050___AC_COUPLED_8_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___CON_RL_DRV_8_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___CON_RL_RCV_8_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___FLOAT_ON_DRV_8_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0050___FLOAT_ON_RCV_8_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG0052___AC_COUPLED_9_get(adf4030, &config->ac_coupled);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___CON_RL_DRV_9_get(adf4030, &config->link_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___CON_RL_RCV_9_get(adf4030, &config->link_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___FLOAT_ON_DRV_9_get(adf4030, &config->float_tx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0052___FLOAT_ON_RCV_9_get(adf4030, &config->float_rx);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    return API_CMS_ERROR_OK;
}


/*! @} */
