/*!
 * \brief     PMBus Numeric Data Format Conversion source file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_pmbus_format_conv.h"

/*==================== S T A T I C  F U N C T I O N S ====================*/

/**
 * \brief       Convert a 'real world' floating point value to PMBus's LINEAR11 Numeric Format.
 * 
 * \param[in]   input_val   The value, of type float, which need to be converted to LINEAR11.
 * 
 * \return      The LINEAR11 conversion for 'input_val'.
 */
static uint16_t float_to_l11(float input_val);

/**
 * \brief       Convert a PMBus LINEAR11 Format to floating point value.
 * 
 * \param[in]   input_val   The value, of type int, which need to be converted to float.
 * 
 * \return      The floating point value conversion for 'input_val'.
 */
static float l11_to_float(uint16_t input_val);

/**
 * \brief       Convert a 'real world' floating point value to PMBus's LINEAR16 Numeric Format.
 * 
 * \param[in]   input_val   The value, of type float, which need to be converted to LINEAR16.
 * \param[in]   vout_mode   The VOUT_MODE value from which exponent will be calculated.
 * 
 * \return      The LINEAR16 conversion for 'input_val' based on hardware 'exponent'.
 */
static uint16_t float_to_l16(float input_val, uint8_t vout_mode);

/**
 * \brief       Convert a PMBus LINEAR16 Format to floating point value.
 * 
 * \param[in]   input_val   The value, of type int, which need to be converted to float.
 * \param[in]   vout_mode   THe VOUT_MODE value from which exponent will be calculated.
 * 
 * \return      The floating point value conversion for 'input_val' based on hardware 'exponent'.
 */
static float l16_to_float(uint16_t input_val, uint8_t vout_mode);

/*==================== C O D E ====================*/

int32_t adi_pmbus_format_conv_float_to_l16(uint8_t vout_mode, float vout_float[], uint16_t vout_l16[], uint32_t num_page_channels)
{
    uint8_t i = 0;
    for (i = 0; i < num_page_channels; ++i) {
        vout_l16[i] = float_to_l16(vout_float[i], vout_mode);
    }
    return 0;
}

int32_t adi_pmbus_format_conv_l16_to_float(uint8_t vout_mode, uint16_t vout_l16[], float vout_float[], uint32_t num_page_channels)
{
    uint8_t i = 0;
    for (i = 0; i < num_page_channels; ++i) {
        vout_float[i] = l16_to_float(vout_l16[i], vout_mode);
    }
    return 0;
}

int32_t adi_pmbus_format_conv_float_to_l11(float float_input[], uint16_t l11_value[], uint32_t num_page_channels)
{
    uint8_t i = 0;
    for (i = 0; i < num_page_channels; ++i) {
        l11_value[i] = float_to_l11(float_input[i]);
    }
    return 0;
}

int32_t adi_pmbus_format_conv_l11_to_float(uint16_t l11_input[], float float_value[], uint32_t num_page_channels)
{
    uint8_t i = 0;
    for (i = 0; i < num_page_channels; ++i) {
        float_value[i] = l11_to_float(l11_input[i]);
    }
    return 0;
}

static uint16_t float_to_l11(float input_val)
{
    int8_t exponent = 0;
    int32_t mantissa = 0;
    uint16_t uExponent = 0;
    uint16_t uMantissa = 0;

    /*
    A N-bit, two's complement, numeral system can only represent integers in the range −2^(N−1) to 2^(N−1)−1.
    So, For 5 bit exponent, 2's complement range: -16 to 15.
        And 11 bit mantissa, 2's complement range: -1024 to 1023.

    For L11,
    input_value = mantissa * 2^exponent. 
    So for every exponent within it's range, check for mantissa that falls with it's range.       
    */
    for (exponent = -16; exponent <= 15; ++exponent)
    {
        mantissa = (int32_t) (input_val / pow(2.0, (float)exponent));
        // printf("expo: %i : 0x%04X \tman: %i : 0x%04X.\n", exponent, exponent, mantissa, mantissa);
        if ((mantissa >= -1024) && (mantissa <= 1023))
            break;
    }

    uExponent = (uint16_t) (exponent << 11);
    uMantissa = (uint16_t) (mantissa & 0x07FF);
    

    return (uExponent | uMantissa);
}

static float l11_to_float(uint16_t input_val)
{
    // Extract exponent as Most Significant 5 bits.
    int8_t exponent = (int8_t) (input_val >> 11);

    // Extract mantissa as Least Significant 11 bits.
    int16_t mantissa = input_val & 0x07FF;
    
    // sign extend exponent from 5 to 8 bits
    if( exponent > 0x0F ) 
        exponent |= 0xE0;
    // sign extend mantissa from 11 to 16 bits
    if( mantissa > 0x03FF ) 
        mantissa |= 0xF800;

    return ((float) mantissa * pow(2, (float)exponent));
}

static uint16_t float_to_l16(float input_val, uint8_t vout_mode)
{
    int8_t exponent = 0;

    // For negative values, output is 0.
    if(input_val < 0)
        return 0;
    
    exponent = ((int8_t)vout_mode) & 0x1F;

    // Sign extend exponent from 5 to 8 bits
    if (exponent > 0x0F) 
        exponent |= 0xE0;
    
    return (uint16_t)(round(input_val / pow(2.0, (float)exponent)));
}

static float l16_to_float(uint16_t input_val, uint8_t vout_mode)
{
    int8_t exponent = ((int8_t)vout_mode) & 0x1F;
    
    // Sign extend exponent from 5 to 8 bits
    if (exponent > 0x0F) 
        exponent |= 0xE0;

    return ((float) input_val) * pow(2, (float)exponent);
}


/*******************************************************************************************/

/*! @} */
