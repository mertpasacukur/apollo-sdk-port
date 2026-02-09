/*!
 * \brief     Source file implementing APIs for ADF4030's BSYNC Output functionality.
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * \addtogroup ADI_ADF4030_BSYNCOUT
 * @{
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_bsyncout.h"
#include "adi_adf4030_bf.h"

/*============= C O D E ====================*/


int32_t adi_adf4030_bsyncout_config_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsyncout_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(config->ref_input_freq_hz, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);
    ADI_ADF4030_VALIDATE_RANGE(config->o_div1, ADI_ADF4030_O_DIV_MIN, ADI_ADF4030_O_DIV_MAX);
    ADI_ADF4030_VALIDATE_RANGE(config->o_div2, ADI_ADF4030_O_DIV_MIN, ADI_ADF4030_O_DIV_MAX);

    err = adi_adf4030_bf___REG0053___ODIV1_set(adf4030, config->o_div1);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0054___ODIV2_set(adf4030, config->o_div2);
    ADI_ADF4030_CHECK_ERR_OK(err);


    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG003f___ODIV_SEL_0_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_0_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_0_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003f___BST_0_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG0041___ODIV_SEL_1_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_1_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_1_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0041___BST_1_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);

        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0043___ODIV_SEL_2_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_2_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_2_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0043___BST_2_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0045___ODIV_SEL_3_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_3_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_3_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0045___BST_3_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0047___ODIV_SEL_4_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_4_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_4_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0047___BST_4_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG0049___ODIV_SEL_5_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_5_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_5_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0049___BST_5_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG004b___ODIV_SEL_6_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_6_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_6_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004b___BST_6_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG004d___ODIV_SEL_7_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_7_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_7_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004d___BST_7_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG004f___ODIV_SEL_8_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_8_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_8_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG004f___BST_8_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG0051___ODIV_SEL_9_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].odiv_select);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_9_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].pd_tx_path);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_9_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0051___BST_9_set(adf4030, config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].channel_boost);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    config->bsync_channels[channel_id].channel_id = channel_id;

    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_bsyncout_freq_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, uint64_t vco_out_freq, uint64_t bsync_out_freq)
{
    int32_t err;
    uint32_t o_div;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(vco_out_freq, ADI_ADF4030_VCO_FREQ_MIN, ADI_ADF4030_VCO_FREQ_MAX);
    ADI_ADF4030_VALIDATE_RANGE(bsync_out_freq, ADI_ADF4030_BSYNC_FREQ_MIN, ADI_ADF4030_BSYNC_FREQ_MAX);

    if (vco_out_freq % bsync_out_freq) {
        return API_CMS_ERROR_INVALID_PARAM;
    }

    o_div = vco_out_freq / bsync_out_freq;
    ADI_ADF4030_VALIDATE_RANGE(o_div, ADI_ADF4030_O_DIV_MIN, ADI_ADF4030_O_DIV_MAX);

    err = adi_adf4030_bf___REG0053___ODIV1_set(adf4030, o_div);
    ADI_ADF4030_CHECK_ERR_OK(err);

    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG003f___ODIV_SEL_0_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_0_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_0_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG0041___ODIV_SEL_1_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_1_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_1_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0043___ODIV_SEL_2_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_2_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_2_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0045___ODIV_SEL_3_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_3_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_3_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0047___ODIV_SEL_4_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_4_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_4_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG0049___ODIV_SEL_5_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_5_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_5_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG004b___ODIV_SEL_6_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_6_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_6_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG004d___ODIV_SEL_7_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_7_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_7_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG004f___ODIV_SEL_8_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_8_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_8_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG0051___ODIV_SEL_9_set(adf4030, ADI_ADF4030_CHANNEL_ODIV1);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_9_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_9_set(adf4030, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }


    return API_CMS_ERROR_OK;
}


int32_t adi_adf4030_bsyncout_drive_set(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, adi_adf4030_bsync_drive_e bsync_drive)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(bsync_drive, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_RX, ADI_ADF4030_CHANNEL_BSYNC_DRIVE_TX);

    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_0_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_0_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_1_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_1_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_2_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_2_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_3_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_3_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_4_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_4_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG003b___PD_TX_PATH_5_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_5_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_6_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_6_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_7_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0012___EN_DRV_7_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_8_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_8_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG003c___PD_TX_PATH_9_set(adf4030, 0);
        ADI_ADF4030_CHECK_ERR_OK(err);
        err = adi_adf4030_bf___REG0013___EN_DRV_9_set(adf4030, bsync_drive);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4030_bsyncout_boost_enable(adi_adf4030_device_t *adf4030, adi_adf4030_channel_id_e channel_id, uint8_t enable)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_VALIDATE_RANGE(channel_id, ADI_ADF4030_CHANNEL_ID_0, ADI_ADF4030_CHANNEL_ID_9);
    ADI_ADF4030_VALIDATE_RANGE(enable, 0, 1);

    switch (channel_id) {
    case ADI_ADF4030_CHANNEL_ID_0:
        err = adi_adf4030_bf___REG003f___BST_0_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_1:
        err = adi_adf4030_bf___REG0041___BST_1_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_2:
        err = adi_adf4030_bf___REG0043___BST_2_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_3:
        err = adi_adf4030_bf___REG0045___BST_3_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_4:
        err = adi_adf4030_bf___REG0047___BST_4_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_5:
        err = adi_adf4030_bf___REG0049___BST_5_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_6:
        err = adi_adf4030_bf___REG004b___BST_6_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_7:
        err = adi_adf4030_bf___REG004d___BST_7_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_8:
        err = adi_adf4030_bf___REG004f___BST_8_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    case ADI_ADF4030_CHANNEL_ID_9:
        err = adi_adf4030_bf___REG0051___BST_9_set(adf4030, enable);
        ADI_ADF4030_CHECK_ERR_OK(err);
        break;

    default:
        ADI_ADF4030_CHECK_ERR_OK(API_CMS_ERROR_ERROR);
        break;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4030_bsyncout_config_get(adi_adf4030_device_t *adf4030, uint64_t ref_input_freq, adi_adf4030_bsyncout_config_t *config)
{
    int32_t err;

    ADI_ADF4030_PTR_CHECK(adf4030);
    ADI_ADF4030_PTR_CHECK(config);
    ADI_ADF4030_VALIDATE_RANGE(config->ref_input_freq_hz, ADI_ADF4030_REF_FREQ_MIN, ADI_ADF4030_REF_FREQ_MAX);

    config->ref_input_freq_hz = ref_input_freq;

    err = adi_adf4030_bf___REG0053___ODIV1_get(adf4030, &config->o_div1);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0054___ODIV2_get(adf4030, &config->o_div2);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].channel_id = ADI_ADF4030_CHANNEL_ID_0;
    err = adi_adf4030_bf___REG003f___ODIV_SEL_0_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_0_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_0_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003f___BST_0_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_0].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].channel_id = ADI_ADF4030_CHANNEL_ID_1;
    err = adi_adf4030_bf___REG0041___ODIV_SEL_1_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_1_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_1_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0041___BST_1_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_1].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].channel_id = ADI_ADF4030_CHANNEL_ID_2;
    err = adi_adf4030_bf___REG0043___ODIV_SEL_2_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_2_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_2_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0043___BST_2_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_2].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);


    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].channel_id = ADI_ADF4030_CHANNEL_ID_3;
    err = adi_adf4030_bf___REG0045___ODIV_SEL_3_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_3_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_3_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0045___BST_3_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_3].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].channel_id = ADI_ADF4030_CHANNEL_ID_4;
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0047___ODIV_SEL_4_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_4_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_4_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0047___BST_4_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_4].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].channel_id = ADI_ADF4030_CHANNEL_ID_5;
    err = adi_adf4030_bf___REG0049___ODIV_SEL_5_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003b___PD_TX_PATH_5_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_5_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0049___BST_5_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_5].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].channel_id = ADI_ADF4030_CHANNEL_ID_6;
    err = adi_adf4030_bf___REG004b___ODIV_SEL_6_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003c___PD_TX_PATH_6_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_6_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG004b___BST_6_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_6].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].channel_id = ADI_ADF4030_CHANNEL_ID_7;
    err = adi_adf4030_bf___REG004d___ODIV_SEL_7_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003c___PD_TX_PATH_7_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0012___EN_DRV_7_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG004d___BST_7_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_7].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].channel_id = ADI_ADF4030_CHANNEL_ID_8;
    err = adi_adf4030_bf___REG004f___ODIV_SEL_8_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003c___PD_TX_PATH_8_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0013___EN_DRV_8_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG004f___BST_8_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_8].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].channel_id = ADI_ADF4030_CHANNEL_ID_9;
    err = adi_adf4030_bf___REG0051___ODIV_SEL_9_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].odiv_select));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG003c___PD_TX_PATH_9_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].pd_tx_path);
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0013___EN_DRV_9_get(adf4030, (uint8_t *) &(config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].bsync_drive));
    ADI_ADF4030_CHECK_ERR_OK(err);
    err = adi_adf4030_bf___REG0051___BST_9_get(adf4030, &config->bsync_channels[ADI_ADF4030_CHANNEL_ID_9].channel_boost);
    ADI_ADF4030_CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}


/*! @} */
