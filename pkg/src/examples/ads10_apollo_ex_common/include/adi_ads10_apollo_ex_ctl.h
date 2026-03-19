/* 
 * \file
 * \brief     ADS10 Apollo Examples Console Control Macros
 *
 * This header provides a set of macros for controlling and documenting test steps
 * in ADS10 Apollo example applications. The macros enable standardized output to the
 * console for both automation and manual test flows, including prompts for user input,
 * measurement expectations, and test status reporting.
 *
 * \details
 * - Macros are conditionally defined based on the EXCTL_AUTOMATION preprocessor symbol.
 *   - If EXCTL_AUTOMATION is defined, output is formatted for automated test control.
 *   - Otherwise, output is formatted for interactive/manual use.
 * - Macros cover:
 *   - Clock configuration reporting (FPGA/Device reference clocks)
 *   - Transmit (TX) measurement and reference steps
 *   - Receive (RX) measurement and reference steps
 *   - Signal generator control
 *   - Test pass/fail/skip reporting
 *   - User pause/continue prompts
 *
 * \copyright Copyright(c) 2024 Analog Devices, Inc. All rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated Analog Devices software license agreement.
*/

#include <stdio.h>

#ifndef __ADI_ADS10_APOLLO_EX_CTL_H__
#define __ADI_ADS10_APOLLO_EX_CTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EXCTL_AUTOMATION

#define _EXCTL_START        printf("EXCTL:")
#define EXCTL_PAUSE(block)  if (block) {printf(":<CR>\n"); getchar();} else printf("\n")

#define _CLK_START                _EXCTL_START;  printf("CLK:")
#define EXCTL_FPGA_CLK(freq_mhz)  _CLK_START;    printf("FPGA<%f>", (double)(freq_mhz)); EXCTL_PAUSE(true);
#define EXCTL_DEV_CLK(freq_mhz)   _CLK_START;    printf("DEV<%f>", (double)(freq_mhz));  EXCTL_PAUSE(true);
#define EXCTL_DEV_PLL_CLK(freq_mhz) printf("DEV PLL REF CLK: %fMHz\n", (double)(freq_mhz));


#define _TX_START(channels)                                           _EXCTL_START;           printf("DAC<%x>:", channels)
#define EXCTL_TX_MEAS_FREQ(channels, freq_mhz, tolerance, block)      _TX_START(channels);    printf("FREQ<%f><%f>", (double)(freq_mhz), (double)(tolerance));          EXCTL_PAUSE(block)
#define EXCTL_TX_MEAS_LEVEL(channels, freq_mhz, level_db, tolerance, block)     _TX_START(channels);    printf("LEV<%f><%f><%f>", (double)(freq_mhz), (double)(level_db), (double)(tolerance));  EXCTL_PAUSE(block)
#define EXCTL_TX_REF(channels, freq_mhz, block)                                 _TX_START(channels);    printf("REF<%f>", (double)(freq_mhz));                                                   EXCTL_PAUSE(block)
#define EXCTL_TX_REFCLR(channels, block)                              _TX_START(channels);    printf("REFCLR");                                                     EXCTL_PAUSE(block)
#define EXCTL_TX_MEAS_NOSIG(channels, block)                          _TX_START(channels);    printf("NOSIG");                                                      EXCTL_PAUSE(block)

#define _RX_START(channels, fname)                                           _EXCTL_START;                  printf("ADC<%x><%s>:", channels, fname)
#define EXCTL_RX_MEAS_FREQ(channels, fname, freq_mhz, tolerance)      _RX_START(channels, fname);    printf("FREQ<%f><%f>", (double)(freq_mhz), (double)(tolerance));         EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_NSD(channels, fname, nsd, tolerance)            _RX_START(channels, fname);    printf("NSD<%f><%f>", (double)(nsd), (double)(tolerance));               EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_LEVEL(channels, fname, level_db, tolerance)     _RX_START(channels, fname);    printf("LEV<%f><%f>", (double)(level_db), (double)(tolerance));          EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_LEVEL_FREQ(channels, fname, level_db, level_tolerance, freq_mhz, freq_tolerance)  _RX_START(channels, fname);    printf("LEV_FREQ<%f><%f><%f><%f>", (double)(level_db), (double)(level_tolerance), (double)(freq_mhz), (double)(freq_tolerance));  EXCTL_PAUSE(false)
#define EXCTL_RX_REF(channels, fname)                                 _RX_START(channels, fname);    printf("REF");                                                             EXCTL_PAUSE(false)
#define EXCTL_RX_REF_FREQ(channels, fname, freq_mhz)       _RX_START(channels, fname);    printf("REF_FREQ<%f>", (double)(freq_mhz));                                           EXCTL_PAUSE(false)
#define EXCTL_RX_REFCLR(channels, fname)                              _RX_START(channels, fname);    printf("REFCLR");                                                          EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_SPUR(channels, fname, freq_mhz, tolerance)      _RX_START(channels, fname);    printf("SPUR<%f><%f>", (double)(freq_mhz), (double)(tolerance));           EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_NOSIG(channels, fname)                          _RX_START(channels, fname);    printf("NOSIG");                                                           EXCTL_PAUSE(false)

#define _SIGGEN_START                                      _EXCTL_START;           printf("SIGGEN:")
#define EXCTL_SIGGEN_ON(channels, block)                   _SIGGEN_START;          printf("ON<%x>", channels);                    EXCTL_PAUSE(block)
#define EXCTL_SIGGEN_FREQ(freq_mhz, block)                 _SIGGEN_START;          printf("FREQ<%f>", (double)(freq_mhz));          EXCTL_PAUSE(block)
#define EXCTL_SIGGEN_LEVEL(level_dbm, block)               _SIGGEN_START;          printf("LEVEL<%f>", (double)(level_dbm));        EXCTL_PAUSE(block)

#define EXCTL_PASS()                                       _EXCTL_START;           printf("PASS");                                EXCTL_PAUSE(0)
#define EXCTL_ERR(err)                                     _EXCTL_START;           printf("ERR<%d>", err);                        EXCTL_PAUSE(0)
#define EXCTL_FAIL(message)                                _EXCTL_START;           printf("FAIL<%s>", message);      EXCTL_PAUSE(0)
#define EXCTL_SKIP(message)                                _EXCTL_START;           printf("SKIP<%s>", message);      EXCTL_PAUSE(0)
#define EXCTL_END(err)                                     if (err == 0)  {EXCTL_PASS();} else {EXCTL_ERR(err);}

#else

#define EXCTL_PAUSE(block)                                                          if (block) {printf("\n<CR> to continue.\n"); getchar();} else printf("\n")

#define EXCTL_FPGA_CLK(freq_mhz)                                                    printf("FPGA REF CLK: %fMHz\n", (double)(freq_mhz));
#define EXCTL_DEV_CLK(freq_mhz)                                                     printf("DEV REF CLK: %fMHz\n", (double)(freq_mhz));
#define EXCTL_DEV_PLL_CLK(freq_mhz)                                                 printf("DEV PLL REF CLK: %fMHz\n", (double)(freq_mhz));

#define EXCTL_TX_MEAS_FREQ(channels, freq_mhz, tolerance, block)                    printf("Expect %fMHz +/-%f on DACs 0x%x", (double)(freq_mhz), (double)(tolerance), channels);   EXCTL_PAUSE(block)
#define EXCTL_TX_MEAS_LEVEL(channels, freq_mhz, level_db, tolerance, block)         printf("Expect %fdB +/-%f on DACs 0x%x", (double)(level_db), (double)(tolerance), channels);    EXCTL_PAUSE(block)
#define EXCTL_TX_REF(channels, freq_mhz, block)                                     printf("Record reference level on DACs 0x%x", channels);                                        EXCTL_PAUSE(block)
#define EXCTL_TX_REFCLR(channels, block)                                            ;
#define EXCTL_TX_MEAS_NOSIG(channels, block)                                        printf("Expect no output on DACs 0x%x", channels);                                          EXCTL_PAUSE(block)

#define _RX_STOP(channels, fname)                                                   printf(" on captures 0x%x, filename %s", channels, fname)
#define EXCTL_RX_MEAS_FREQ(channels, fname, freq_mhz, tolerance)                    printf("Expect %fMHz +/-%f", (double)(freq_mhz), (double)(tolerance));                                      _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_NSD(channels, fname, nsd, tolerance)                          printf("Expect %fN/Hz +/-%f", (double)(nsd), (double)(tolerance));                                          _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_LEVEL(channels, fname, level_db, tolerance)                   printf("Expect %fdB +/-%f", (double)(level_db), (double)(tolerance));                                       _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_LEVEL_FREQ(channels, fname, level_db, level_tolerance, freq_mhz, freq_tolerance)      printf("Expect %fdB +/-%f at %fMHz +/-%f", (double)(level_db), (double)(level_tolerance), (double)(freq_mhz), (double)(freq_tolerance));          _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_REF(channels, fname)                                               printf("Record reference level");                                                                           _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_REF_FREQ(channels, fname, freq_mhz)                                printf("Record reference level at %fMHz", (double)(freq_mhz));                                              _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_REFCLR(channels, fname)                                            ;
#define EXCTL_RX_MEAS_SPUR(channels, fname, freq_mhz, tolerance)                    printf("Expect spur at %fMHz +/-%f", (double)(freq_mhz), (double)(tolerance));                              _RX_STOP(channels, fname); EXCTL_PAUSE(false)
#define EXCTL_RX_MEAS_NOSIG(channels, fname)                                        printf("Expect no signal");                                                                                 _RX_STOP(channels, fname); EXCTL_PAUSE(false)

#define EXCTL_SIGGEN_ON(channels, block)                                            ;
#define EXCTL_SIGGEN_FREQ(freq_mhz, block)                                          printf("Set signal generator frequency to %fMHz", (double)(freq_mhz));     EXCTL_PAUSE(block)
#define EXCTL_SIGGEN_LEVEL(level_dbm, block)                                        printf("Set signal generator level to %fdBm", (double)(level_dbm));        EXCTL_PAUSE(block)

#define EXCTL_PASS()                                                                printf("Example passed");                                EXCTL_PAUSE(0)
#define EXCTL_ERR(err)                                                              printf("Example failed with error code %d", err);        EXCTL_PAUSE(0)
#define EXCTL_FAIL(message)                                                         printf("Example failed with message '%s'", message);     EXCTL_PAUSE(0)
#define EXCTL_SKIP(message)                                                         printf("Example skipped with message '%s'", message);    EXCTL_PAUSE(0)
#define EXCTL_END(err)                                                              if (err == 0)  {EXCTL_PASS();} else {EXCTL_ERR(err);} printf("Goodbye\n");

#endif

#ifdef __cplusplus
}
#endif
#endif /* __ADI_ADS10_APOLLO_EX_CTL_H__ */