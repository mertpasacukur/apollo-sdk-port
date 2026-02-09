/*!
 * \brief     Basic FPGA Apollo functions
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= D E F I N E S ==============*/
#define SET_BIT(num, bit_index)     ((num) | 1 << (bit_index))
#define CLEAR_BIT(num, bit_index)   ((num) & ~(1 << (bit_index)))

/*============= I N C L U D E S ============*/
#include "adi_fpga_apollo_gpio.h"


static uint32_t get_bf_info(uint32_t addr, uint32_t bf_info);

/*============= C O D E ====================*/

int32_t adi_fpga_apollo_gpio_output_set(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t gpio_state)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t reg_addr = 0;
    uint32_t regRd = 0;
    uint32_t regWr = 0;
    uint8_t bit_index = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK(gpio_index > (ADI_FPGA_GPIO_NUM - 1));
    ADI_CMS_INVALID_PARAM_CHECK((gpio_state > 1));

    bit_index = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? (gpio_index % ADI_FPGA_CMOS_GPIO_NUM) : (gpio_index % 32);
    // Read GPIO Direction register to get current set direction.
    reg_addr = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? REG_DUT_SYNCB_GPIO_D_ADDR : (((gpio_index / 32) == 0) ? REG_GPIO_D_1_ADDR : REG_GPIO_D_2_ADDR);
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO at gpio_index as Output and other GPIOs as is.
    regWr = SET_BIT(regRd, bit_index);
    err = fpga->hal_info.reg_write(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);


    // Read present GPIO Write register.
    reg_addr = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? REG_DUT_SYNCB_GPIO_W_ADDR : (((gpio_index / 32) == 0) ? REG_GPIO_W_1_ADDR : REG_GPIO_W_2_ADDR);
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO State of gpio_index and other GPIOs as is.
    if (gpio_state == 1)
        regWr = SET_BIT(regRd, bit_index);
    else
        regWr = CLEAR_BIT(regRd, bit_index);

    err = fpga->hal_info.reg_write(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_gpio_output_word_set(adi_fpga_apollo_device_t *fpga, uint64_t gpio_mask, uint64_t gpio_state)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t regRd = 0;
    uint32_t regWr = 0;
    uint32_t mask = 0;
    uint32_t state = 0;
    uint8_t r;
    uint32_t reg_d[] = {REG_GPIO_D_1_ADDR, REG_GPIO_D_2_ADDR};
    uint32_t reg_w[] = {REG_GPIO_W_1_ADDR, REG_GPIO_W_2_ADDR};

    ADI_CMS_NULL_PTR_CHECK(fpga);

    if (fpga->state_info.design_id ==  ADI_FPGA_APOLLO_DESIGN_VCU128) {
        gpio_mask = gpio_mask >> 15;
        gpio_state = gpio_state >> 15;
    }

    for (r = 0; r < 2; r++) {
        // Read GPIO Direction register to get current set direction.
        mask = (uint32_t) (gpio_mask >> (r * 32));
        if (mask == 0)
            break;

        state = (uint32_t) (gpio_state >> (r * 32));
        state = state & mask;
        err = fpga->hal_info.reg_read(reg_d[r], &regRd);
        ADI_CMS_ERROR_RETURN(err);

        // Set FPGA GPIO at gpio_index as Output and other GPIOs as is.
        regWr = regRd | gpio_mask;
        err = fpga->hal_info.reg_write(reg_d[r], regWr);
        ADI_CMS_ERROR_RETURN(err);


        // Read present GPIO Write register.
        err = fpga->hal_info.reg_read(reg_w[r], &regRd);
        ADI_CMS_ERROR_RETURN(err);

        // Set FPGA GPIO State of gpio_index and other GPIOs as is.
        regWr = regRd & (~mask);
        regWr |= state;
        err = fpga->hal_info.reg_write(reg_w[r], regWr);
        ADI_CMS_ERROR_RETURN(err);
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_gpio_input_get(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t *gpio_state)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t reg_addr = 0;
    uint32_t regRd = 0;
    uint32_t regWr = 0;
    uint8_t bit_index = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(gpio_state);
    ADI_CMS_INVALID_PARAM_CHECK(gpio_index > (ADI_FPGA_GPIO_NUM - 1));

    bit_index = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? (gpio_index % ADI_FPGA_CMOS_GPIO_NUM) : (gpio_index % 32);
    // Read GPIO Direction register to get current set direction.
    reg_addr = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? REG_DUT_SYNCB_GPIO_D_ADDR : (((gpio_index / 32) == 0) ? REG_GPIO_D_1_ADDR : REG_GPIO_D_2_ADDR);
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO at gpio_index as Input and other GPIOs as is.
    regWr = CLEAR_BIT(regRd, bit_index);
    err = fpga->hal_info.reg_write(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);

    // Read GPIO Input register to get current GPIO state.
    reg_addr = (gpio_index > (ADI_FPGA_CMOS_GPIO_NUM - 1)) ? REG_DUT_SYNCB_GPIO_R_ADDR : (((gpio_index / 32) == 0) ? REG_GPIO_R_1_ADDR : REG_GPIO_R_2_ADDR);
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    *gpio_state = ((regRd >> bit_index) & 0x01);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_syncinb_ab_sel(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_syncinb_src_sel_e syncinb_scr_sel, adi_fpga_apollo_syncinb_link_sel_e syncinb_link_sel)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t bf_info = get_bf_info(BF_DUT_SYNCINB_AB_SEL_INFO);
    uint8_t offset = (uint8_t)(bf_info >> 0);
    uint32_t regRd = 0;
    uint32_t regWr = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK((syncinb_scr_sel > 1));
    ADI_CMS_INVALID_PARAM_CHECK((syncinb_link_sel > 1));

    err = fpga->hal_info.reg_read(REG_DUT_SYNCB_CTRL_ADDR, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    if(syncinb_scr_sel == 1) {
        regWr = SET_BIT(regRd, (offset + syncinb_link_sel));
    } else {
        regWr = CLEAR_BIT(regRd, (offset + syncinb_link_sel));
    }

    err = fpga->hal_info.reg_write(REG_DUT_SYNCB_CTRL_ADDR, regWr);
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.reg_read(REG_DUT_SYNCB_CTRL_ADDR, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}


int32_t adi_fpga_apollo_syncoutb_ab_sel(adi_fpga_apollo_device_t *fpga, adi_fpga_apollo_syncoutb_src_sel_e syncoutb_scr_sel, adi_fpga_apollo_syncoutb_link_sel_e syncoutb_link_sel)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t bf_info = get_bf_info(BF_DUT_SYNCOUTB_AB_SEL_INFO);
    uint8_t offset = (uint8_t)(bf_info >> 0);
    uint32_t regRd = 0;
    uint32_t regWr = 0;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_INVALID_PARAM_CHECK((syncoutb_scr_sel > 1));
    ADI_CMS_INVALID_PARAM_CHECK((syncoutb_link_sel > 1));

    err = fpga->hal_info.reg_read(REG_DUT_SYNCB_CTRL_ADDR, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    if(syncoutb_scr_sel == 1) {
        regWr = SET_BIT(regRd, (offset + syncoutb_link_sel));
    } else {
	regWr = CLEAR_BIT(regRd, (offset + syncoutb_link_sel));
    }

    err = fpga->hal_info.reg_write(REG_DUT_SYNCB_CTRL_ADDR, regWr);
    ADI_CMS_ERROR_RETURN(err);

    err = fpga->hal_info.reg_read(REG_DUT_SYNCB_CTRL_ADDR, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_fpga_apollo_gpio_aux_input_get(adi_fpga_apollo_device_t *fpga, uint8_t gpio_index, uint8_t *gpio_state)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t reg_addr = 0;
    uint32_t regRd = 0;
    uint32_t regWr = 0;
    uint8_t bit_index = gpio_index;

    ADI_CMS_NULL_PTR_CHECK(fpga);
    ADI_CMS_NULL_PTR_CHECK(gpio_state);
    ADI_CMS_INVALID_PARAM_CHECK(gpio_index > (ADI_FPGA_AUX_GPIO_NUM - 1));

    // Read GPIO Direction register to get current set direction.
    reg_addr = REG_AUX_GPIO_D_ADDR;
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    // Set FPGA GPIO at gpio_index as Input and other GPIOs as is.
    regWr = CLEAR_BIT(regRd, bit_index);
    err = fpga->hal_info.reg_write(reg_addr, regWr);
    ADI_CMS_ERROR_RETURN(err);

    // Read GPIO Input register to get current GPIO state.
    reg_addr = REG_AUX_GPIO_R_ADDR;
    err = fpga->hal_info.reg_read(reg_addr, &regRd);
    ADI_CMS_ERROR_RETURN(err);

    *gpio_state = ((regRd >> bit_index) & 0x01);

    return API_CMS_ERROR_OK;
}

static uint32_t get_bf_info(uint32_t addr, uint32_t bf_info)
{
    return bf_info;
}
