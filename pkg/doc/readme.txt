May 12, 2025

API Version: v0.5.0
FW version: 20250227.3.19 (*new*)

Device profile: v9.1.0

ADS10 FPGA versions:
    10/20/22 (sw fsrc)
    10/05/22 (hw fsrc, hw jesd transport)
    09/20/22 (loopback),
    02/17/23 (hw fsrc, sc1)
    09/20/23 (hw fsrc, sysref seq w/ ext trig)
    11/20/23 (raw image, rx/tx sw transport, rx/tx sw fsrc)
    07/11/24 (raw image, rx/tx sw transport, rx/tx sw fsrc)
    09/25/24 RAW (raw image, rx/tx sw transport, rx/tx sw fsrc)
    08/09/24 MCS (mcs image, rx/tx hw transport, rx/tx hw fsrc)
    10/28/24 RAW (raw image, rx/tx sw transport, rx/tx sw fsrc)
    10/24/24 MCS (mcs image, rx/tx hw transport, rx/tx hw fsrc)

Release package contains the following:
- Apollo device API source code
- ADS10 FPGA API source code
- Supporting devices' API source code
- API Example Code and device profiles
- Apollo FW binaries for X-grade and B-grade Apollo Silicons


Example Code
- The example code supports B0 silicon (A0 no longer supported from v0.3.19)
- The debug_b0.mak file contains configuration info for compiling
- To build an example, run 'make` in the selected folder

    make

  Compile Macros:
    ADI_EN_STDERR           - Prints error code, file and line # to stderr if defined
    ADI_MIN_STACK_ALLOC     - User provides buffer used by certain APIs to reduce allocating mem on stack
                              See adi_apollo_hal_buffer_set()
    ADS10, VCU128           - Platform targeted


- In folder examples/ads10_apollo_ex_main:
    - Top level main.c that runs common device startup sequence (e.g. firmware and profile loading)
    - Individual example files to demonstrate specific features
    - Example to run and profile is passed via command prompt


Usage: apollo_main EXAMPLE_TYPE DEVICE_PROFILE [-devclk=DEV_CLK_SRC] [-fpgaclk=FPGA_CLK_SRC] [CMD_SPECIFIC_ARGS]

EXAMPLE_TYPE summary:
    dp_load                    id00_uc06 | id00_uc06_T | <any profile>
    fullchip                   id00_uc06 | id00_uc06_F | id00_uc06j1_F | id00_uc08a | id00_uc08a1 | id00_uc06_204B
    fullchip_8t8r              id98_uc05
    fullchip_fsrc_dr           id00_uc06_F | id00_uc06j1_F | id00_uc08a | id00_uc08a1 [-i]
    fullchip_fsrc_sc1_ext_trig id00_uc06_F_sc1 | id00_uc08a_sc1 | id00_uc08a1_sc1
    fullchip_hop               id00_uc06 [-i]
    fullchip_sparse_cfir       id00_uc06_C [-i]
    fullchip_pfilt             id00_uc06 | id98_uc05 [-i]
    fullchip_sc1_dl            id00_uc06sc1 | id00_uc13_sc1 | id00_uc14_sc1 | id00_uc15_sc1 | id00_uc08sc1_f  [-jrx_adj]
    fullchip_mcs_sc1_dl        id00_uc08sc1_f | id00_uc13_sc1 | id00_uc14_sc1 | id00_uc15_sc1 | id99_uc02c_sc1  [-jrx_adj]
    fullchip_sr_dr             id00_uc06 | id00_uc08_f [-i]
    lb0_bmem_delay_hop         id00_uc06 [-i]
    loopback0                  id00_uc06 | id00_uc08a [-i]
    loopback1_2                id00_uc06lb | id98_uc05lb
    jesd_loopback              id00_uc06lb
    rx_adc_cc                  id00_uc06
    rx_adc_deep                id00_uc06 | id01_uc05
    rx_adc_bmem                id00_uc06 | id98_uc05
    rx_adc_fd                  id00_uc06 [-i]
    rx_adc_ms                  id00_uc06
    rx_adc_nz                  id00_uc06
    rx_adc_mux2                id00_uc06 | id98_uc05
    rx_adc_pave                id00_uc06, id98_uc05
    rx_adc_pfilt               id00_uc06_P | id98_uc05
    rx_bmem_cfir               id00_uc06_C
    rx_bmem_ddc                id00_uc06
    rx_jesd                    id00_uc06_F | id00_uc08a | id00_uc08a1  {[-b] for BMEM AWG}
    rx_bmem_pfilt              id00_uc06_P
    rx_bmem_delay              id00_uc06xA2
    rx_sniffer                 id00_uc06 [-i]
    tx_jesd_cfir               id00_uc06_C [-i]
    tx_jesd                    id00_uc06_F | id00_uc08a | id00_uc08a1
    tx_nco                     id00_uc06_T | id00_uc06a_T [-i]
    tx_nco_ffh                 id00_uc06_T
    tx_nco_mod                 id00_uc06_T
    tx_nco_pfilt               id00_uc06_PT | id00_uc06a_PT
    tx_jesd_file               id00_uc06 | id00_uc13 {-a | -f i_file q_file [cnco]}
    mcs_cal                    id00_uc13_sc1 | id99_uc02c_sc1
    prbs                       id00_uc06a_T | id99_uc00 | id99_uc02
    gpio_toggle                id00_uc06
    fmcb_aux                   id00_uc06
    bsync_tof                  id00_uc13_sc1 | id99_uc02c_sc1
    jrx_eye_sweep              id00_uc06a_T | id99_uc00 | id99_uc02

DEVICE_PROFILE summary:
    Dev Profile         Fclk (MHz)  FPGA (MHz)  DR (MHz)  LR (Gbps)  FSRC        PFILT    CFIR         Note
    ===========         ==========  ==========  ========  ========   ====        =====    ====         =============
    id00_uc06           20000.0     156.25      1250      10.3125    no          bypass
    id00_uc06_C         20000.0     156.25      1250      10.3125    no          no       non-sparse
    id00_uc06_F         20000.0     156.25      983.04    10.3125    15625/12288 bypass
    id00_uc06_F_sc1     20000.0     156.25      983.04    10.3125    15625/12288 bypass                FSRC w/ SC1, trig sync
    id00_uc06lb         20000.0     156.25      1250      10.3125    no          bypass                Muxing for loopback
    id00_uc06j1_F       20000.0     156.25      1237.5    10.3125    100/99      bypass
    id00_uc06sc1        20000.0     156.25      1250      10.3125    no          bypass                Subclass 1 testing
    id00_uc06sc1_lb     20000.0     161.1328125 1250      10.3125    no          bypass                Subclass 1 testing. Requires loopback fpga image
    id00_uc06_P         20000.0     156.25      1250      10.3125    no          real n/2
    id00_uc06_PT        20000.0     n/a         n/a       n/a        no          real n/2              NCO test mode
    id00_uc06_T         20000.0     n/a         n/a       n/a        no          bypass                NCO test mode
    id00_uc06xA2        20000.0     156.25      1250      10.3125    no          bypass                ADC 0/1, DAC 0/1 through same datapath
    id00_uc06a          10000.0     156.25      1250      10.3125    no          bypass
    id00_uc06a_PT       10000.0     n/a         n/a       n/a        no          real n/2              NCO test mode
    id00_uc06a_T        10000.0     n/a         n/a       n/a        no          bypass                NCO test mode
    id00_uc06_204B      20000.0     312.5       1250      12.500     no          bypass    no          4T4R, JESD204B
    id00_uc08_f         20000.0     312.5       2500      20.625     no          bypass
    id00_uc08sc1_f      20000.0     312.5       2500      20.625     no          bypass                Subclass 1
    id00_uc08a          20000.0     312.5       1966.08   20.625     15625/12288 bypass
    id00_uc08a_sc1      20000.0     312.5       1966.08   20.625     15625/12288 bypass                FSRC w/ SC1, trig sync
    id00_uc08a1         20000.0     312.5       1497.6    20.625     3125/1872   bypass
    id00_uc08a1_sc1     20000.0     312.5       1497.6    20.625     3125/1872   bypass                FSRC w/ SC1, trig sync
    id00_uc13           20000.0     312.5       5000      20.625     no          bypass                Np=12, 4G IBW
    id00_uc13_sc1       20000.0     312.5       5000      20.625     no          bypass                Np=12, 4G IBW, SC1
    id00_uc14_sc1       19660.8     307.2       2457.6    20.2752    no          bypass                Np=16, SC1
    id00_uc15_sc1       19200.0     300.0       4800      19.800     no          bypass                Np=12, L=12, SC1
    id01_uc05           14000.0     218.75      3500      10.3125    no          bypass                Np=12, 14/28
    id98_uc05           8000.0      250.0       125.0     10.000     no          bypass                8T8R, JESD204B
    id98_uc05lb         8000.0      250.0       125.0     10.000     no          bypass                8T8R, JESD204B, loopback muxes
    id01_uc66           20000.0     156.25      625       10.3125    no          bypass                Lane_Adapt=1
    id99_uc00           10000.0     312.5       2500      20.625     no          bypass                PRBS
    id99_uc02           20000.0     416.667     20000.0   27.5       no          bypass                PRBS, Full BW, 1x1x mode, L=12, Np=16
    id99_uc02c_sc1      19660.8     409.6       19660.8   27.0336    no          bypass                1x1x mode, L=12, Np=16, SC1
    id81_uc28           8000.0      250.0       250       16.5       no          bypass                8T8R, JESD204C
    id81_uc28_sc1       8000.0      250.0       250       16.5       no          bypass                8T8R, JESD204C SC1
    id99_uc09           20000.0     416.667     20000.0   27.5       no          bypass                NP=8
    id00_uc08           20000.0     250         2500      16.5    (3125/1872):(3125/2496)  bypass      Np=16, JESD204C, tot_dcm != link_dcm


DEV_CLK_SRC summary:
    ext_center          Use external sig gen to clock Apollo -- CLK_C (default)
    ext_dual            Use external sig gen to clock Apollo -- CLK_A + CLK_B
    adf4382             Use ADF4382 to clock Apollo

FPGA_CLK_SRC summary:
    ext            Use external sig gen to clock the ADS10 (default)
    fmc            Use HMC7044 to clock the ADS10


- Example application folders (./examples/):
    - ads10_apollo_ex_common                    Code common to all examples, utilities
    - ads10_apollo_ex_main                      Main application for device startup sequence plus example functionality per file
    - ads10_apollo_ex_power_modules             Power Modules Example: Voltage and Current Measurement.
    - configureVUP                              Example that programs specified FPGA binary image. The FPGA img should be on target platform (uzed)

- In folder examples/ads10_apollo_ex_common
    - Boilerplate code used by examples
    - Apollo and FPGA HAL configuration
    - FW load, device profile load, data configuration
    - FPGA JESD configuration
    - Wrappers for 'inspect' functions for displaying device states
    - Internal clocking configuration
    - Vector generation function
    - Common configuration functions for auxilary devices like ADF4382, ADF4030, HMC7044 and FPGA
    - MCS calibration and Bsync alignment functions

- Apollo Startup API: adi_apollo_startup_execute()

The adi_apollo_startup_execute() API is a high level function that executes the Apollo startup sequence in a single call.
Alternatively, it may be called iteratively where the sequence step(s) are passed as a parameter mask.

The startup function is platform agnostic by use of a firmware image provider (adi_apollo_fw_provider_t).
The firmware image provider is an interface the defines function pointers for open, close and get.
These functions are implemented by the user application and called from adi_apollo_startup_execute() to retrieve
Apollo firmware binary byte arrays. The open and close methods are optional. A typical use case is an
application running Linux where the FW provider reads the specified ADI FW binary file and returns a byte array.
All file I/O and memory management is handled by the user. The ADI examples demonstrate this use case.

Example Code References:

File                    Function                                    Description
---------------------   -----------------------------------------   ---------------------------------------------------------------------------------
adi_ads10_apollo_ex.c   adi_ads10_apollo_ex_configure_startup()     Creates a user defined FW provider.
                                                                    Assign open/close/get function pointers to device context

ads10_fw_provider.c     ads10_fw_provider_create()                  Create ADS10 platform specific FW provider
ads10_fw_provider.c     ads10_fw_provider_open()                    Implementations for ADS10 (Linux) based platform FW provider functions
                        ads10_fw_provider_close()
                        ads10_fw_provider_get()

adi_ads10_apollo_ex.c   adi_ads10_apollo_ex_startup()               Calls adi_apollo_startup_execute(device, profile, ADI_APOLLO_STARTUP_SEQ_DEFAULT)

adi_apollo_startup.c    adi_apollo_startup_execute()                Startup sequence API code
