#if !defined(VERSAL_PLATFORM)
/*!
 * \brief     PMBus Numeric Data Format Conversion header file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */


#ifndef __ADI_PMBUS_FORMAT_CONV_H__
#define __ADI_PMBUS_FORMAT_CONV_H__

/*============= I N C L U D E S ============*/

#include <stdio.h>
#include <stdint.h>
#include <math.h>


/*============= N O T E S ==============*/

/*
	LINEAR11 Format: LINEAR_5s_11s

		|<-  High Byte  ->|<-  Low Byte ->|
		___________ _______________________
		|7|6|5|4|3| |2|1|0|7|6|5|4|3|2|1|0|
		|<-  N  ->| |<-        Y        ->|

	X = Y·2^N
	where:
		X is the “real world” value;
		Y is an 11 bit, two’s complement integer (mantissa); and
		N is a 5 bit, two’s complement integer (exponent).



	LINEAR16 Format: LINEAR_16u

		|<- VOUT_MODE ->|		|<- High Byte ->|<-  Low Byte ->|
		_________________		_________________________________
		|7|6|5|4|3|2|1|0|		|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
		|Mode |<-  N  ->|		|<-             Y             ->|

	Voltage = Y·2^N
	where:
		Voltage is the parameter of interest;
		Y is an 16 bit, unsigned integer (mantissa); 
		N is a 5 bit, two’s complement integer (exponent) that is hardwired per device; and
		Mode is hardwired to 000b to select Linear mode.
*/

/*============= E X P O R T S ==============*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLIENT_IGNORE


/**
 * \brief       Convert a 'real world' floating point value to PMBus's LINEAR16 Numeric Format.
 * 				Used for setting or reading output voltage related PMBUS Commands like VOUT_COMMAND, READ_VOUT, etc.
 * 
 * \param[in]   vout_mode   		The VOUT_MODE value from which exponent will be calculated.
 * \param[in]   vout_float  		The value, of type float, which need to be converted to LINEAR16.
 * \param[out]  vout_l16  			The LINEAR16 conversion for 'vout_float' based on hardware 'exponent'.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * 
 * \return      0    API Completed Successfully.
 */
int32_t adi_pmbus_format_conv_float_to_l16(uint8_t vout_mode, float vout_float[], uint16_t vout_l16[], uint32_t num_page_channels);


/**
 * \brief       Convert a PMBus LINEAR16 Format to a 'real world' floating point value.
 * 				Used for setting or reading output voltage related PMBUS Commands like VOUT_COMMAND, READ_VOUT, etc.
 * 
 * \param[in]   vout_mode   		The VOUT_MODE value from which exponent will be calculated.
 * \param[in]   vout_l16  			The LINEAR16 conversion based on hardware 'exponent', which need to be converted to float for 'vout_float'.
 * \param[out]  vout_float  		The value, of type float.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * 
 * \return      0    API Completed Successfully.
 */
int32_t adi_pmbus_format_conv_l16_to_float(uint8_t vout_mode, uint16_t vout_l16[], float vout_float[], uint32_t num_page_channels);

/**
 * \brief       Convert a 'real world' floating point value to PMBus's LINEAR11 Numeric Format.
 * 				Used for setting or reading Ton/Toff time and related PMBUS Commands like TON_DELAY, TON_RISE, TOFF_DELAY, etc.
 * 				Also used to set and read under and over voltage/current, fault and warning limits.
 * 
 * \param[in]   float_input  		The value, of type float, which need to be converted to LINEAR11.
 * \param[out]  l11_value  			The LINEAR11 conversion for 'float_input'.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * 
 * \return      0    API Completed Successfully.
 */
int32_t adi_pmbus_format_conv_float_to_l11(float float_input[], uint16_t l11_value[], uint32_t num_page_channels);

/**
 * \brief       Convert a PMBus LINEAR11 Format to a 'real world' floating point value.
 * 				Used to read measured telemetry related PMBUS Commands like READ_VIN, READ_VOUT, READ_IOUT, READ_TEMPERATURE_1, etc.
 * 
 * \param[in]   l11_input  			The value, of type The LINEAR11, which need to be converted to float.
 * \param[out]  float_value  		The floating point value conversion for the LINEAR11, 'l11_input'.
 * \param[in]   num_page_channels   Number of pages/channels whose output voltage needs to be set.
 * 
 * \return      0    API Completed Successfully.
 */
int32_t adi_pmbus_format_conv_l11_to_float(uint16_t l11_input[], float float_value[], uint32_t num_page_channels);


#endif /* CLIENT_IGNORE*/

#ifdef __cplusplus
}
#endif
#endif /*__ADI_PMBUS_FORMAT_CONV_H__*/

#endif /* !defined(VERSAL_PLATFORM) */
