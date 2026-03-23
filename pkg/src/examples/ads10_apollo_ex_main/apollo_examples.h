#if !defined(VERSAL_PLATFORM)

/*!
 * \brief     Apollo code examples
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */


#ifndef __APOLLO_EXAMPLES_H__
#define __APOLLO_EXAMPLES_H__

#include "adi_apollo.h"
#include "adi_fpga_apollo_types.h"
#include "adi_ads10_apollo_ex_types.h"

int32_t dp_load(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip_8t8r(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip_fsrc_dr(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip_sr_dr(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip_fsrc_sc1_ext_trig(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t fullchip_hop(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t fullchip_pfilt(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t fullchip_sc1_dl(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t fullchip_mcs_sc1_dl(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t loopback0(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t loopback1_2(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t tx_nco(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t tx_nco_ffh(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t tx_nco_pfilt(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_bmem_pfilt(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_cc(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_bmem(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_fd(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_smon(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_mux2(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_pave(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_pfilt(adi_apollo_device_t* device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_bmem_cfir(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t tx_jesd_cfir(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t tx_jesd(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_nz(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_adc_ms(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_bmem_ddc(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t rx_adc_deep(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t tx_jesd_file(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t tx_nco_mod(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t rx_sniffer(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t rx_bmem_delay(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t lb0_bmem_delay_hop(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t mcs_cal(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t jesd_loopback(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fullchip_sparse_cfir(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t prbs(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t gpio_toggle(adi_apollo_device_t* device, adi_fpga_apollo_device_t* fpga_device, adi_apollo_top_t* profile, int argc, char* argv[], int argc_ofst);
int32_t fmcb_aux(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t bsync_tof(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t jrx_eye_sweep(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);
int32_t power_readback(adi_apollo_device_t *device, adi_fpga_apollo_device_t *fpga_device, adi_apollo_top_t *profile, int argc, char *argv[], int argc_ofst);

#endif /* __APOLLO_EXAMPLES_H__ */
#endif /* !defined(VERSAL_PLATFORM) */
