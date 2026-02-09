# ADI Apollo Release Notes
All notable changes to this project will be documented in this file

### [0.5.0] [sdk49] (2025-05-12)

### Features
* Apollo startup and initialization sequence as an API
* Apollo SW1/SW3/SW5 software trim support
* Support for independent JESD modes for FPGA SerDes JTx and JRx with FPGA JRx further supporting different JESD modes on either sides
### FW version
* FW version: B0 (X and B grade): 20250227.3.19 (*new*)

### Known Issues
* BMEM capture returns all 0s if HSCI protocol is used during streaming write and read transaction
* FMCA-RevB and FMCB-RevB requires `adi_apollo_clk_mcs_dyn_sync_sequence_run()` towards the end of Apollo startup sequence
* Captured data from `fullchip_mcs_sc1_dl` example might not be clean
* All deprecated APIs will be removed from future releases

### New Apollo APIs
* STARTUP: *Apollo Startup sequence*
  * `adi_apollo_startup_execute()` - Performs Apollo startup sequence like FW and profile load, Tx-Rx Datapath config, etc
* CLK_MCS: *Apollo System Clock Syncs*
  * `adi_apollo_clk_mcs_dynamic_sync()` - Performs a dynamic sync operation synchronizing digital root clk and all converters
  * `adi_apollo_clk_mcs_dyn_sync_rxtxlinks_sequence_run()` - Run JTx and JRx SerDes Link Dynamic Sync Sequence with digital root clks masked
* HAL: *Apollo HAL APIs*
  * `adi_apollo_hal_buffer_set()` - Assigns user allocated memory for use by HAL APIs. Currently only supported for HSCI streaming reads and writes
  * `adi_apollo_hal_buffer_get()` - Gets pointer to user allocated memory that is used by API. Currently only supported for HSCI streaming reads and writes

### Modified APIs
* CLK_MCS:
  * `adi_apollo_clk_mcs_sync_hw_align_set()`: Masks off SerDes Rx-Tx clocks from HW OneShot sync, aligning internal and external Sysref
* DEVICE:
  * `adi_apollo_device_reset()`: Perform Tx-Rx Datapath reset before Apollo device reset to mitigate sudden supply current changes from creating supply faults
* HAL_REGIO_HSCI:
  * `stream_write()` and `stream_read()`: Can now use user allocated memory for streaming write and read transaction, defined by `ADI_MIN_STACK_ALLOC` macro and made available to device struct by `adi_apollo_hal_buffer_set()` and `adi_apollo_hal_buffer_get()`.
  * If user allocated buffer is not defined, a local buffer of default size will be allocated within API

### Bug Fixes
* CFIR:
  * `adi_apollo_cfir_inspect()`: Fixed CFIR Enabled status being misrepresent as CFIR Bypassed

### FW Changes
* [20250227.3.19] - 2025-04-18
  ### Added
  - Clock conditioning warmboot
  ### Fixed
  - Temperature sensors report wrong temperatures
  - Firmware cannot write to prod_id, chip_type, chip_grade, vendorID EC registers
  - APOLLO_DAC_ENABLE_STATUS scratch register reports the wrong status
  - TDC precondition was executed more than once during MCS measurement
  - Long MCS lock time for 8S systems
  - Reverted JTx force on changes to lower down JTx power consumption
  - JRX: 204B foreground calibration timeout with scrambled framed data in quarter rate mode
  ### Changed
  - ADC: Updated 8S configuration defaults
  - DAC: Disabled cross-point correction for digital clock path in hardware initialization to improve DLL performance
  - Update Mcs_PossibleHwOneShotSync to return error codes and improve error handling
  - Use ADC input signal type in profile to initialize ADC library
  - Check if sw_trim in device profile matches part variant

### Example code
* Modified Common APIs
  * Apollo startup is now performed using the API and is not part of common application code

* Modified Examples
  * All examples now use follows a slightly modified execution flow where, Apollo is first initialized and brought up using the startup API.
  * For each individual example, following the startup, all required digital feature blocks are enabled and configured based on the application. At the end of this a Clock Conditioning call is to be made
  * Also, redundant calls for OneShot Sync are also removed and replaced by application required sync mechanisms
  * All examples are also formatted to use similar error checking and reporting macros
  * The capture code has been updated in terms of how the data is being returned to the user from the FPGA memory.
  * In previous releases, capture would return a 1D array with the data for all converters that had to be decoded to get the desired data. With the implemented changes capture now returns a nested array so that each converter's data is isolated in a separate array. The structure is organized as: `data[link_idx][vc_idx][samples]`



---



### [0.4.58] [sdk48] (2025-02-19)

### Features
* Serdes JRx BGCal APIs
* FPGA binary loading API

### FW version
* FW version: B0 (X and B grade): 20250120.5.352 (*new*)

### New Apollo APIs
* SERDES: *SERDES JRx Init and BG Calibrations*
  * `adi_apollo_serdes_jrx_init_cal()` - Performs SerDes JRx Init (FG) calibration with select SerDes Pack and Config option and logs cal status upon failure
  * `adi_apollo_serdes_jrx_bgcal_freeze()` - Freeze (Halt) the Apollo JRx SerDes background calibration
  * `adi_apollo_serdes_jrx_bgcal_unfreeze()` - Unfreeze (Start/Restarts) the Apollo JRx SerDes background calibration
  * `adi_apollo_serdes_jrx_bgcal_state_get()` - Get SerDes JRx background calibration status
### New Platform APIs
* FPGA Image Loader: *APIs for loading FPGA binary*
  * `adi_fpga_image_loader_configure()` - Loads the platform FPGA image file
  * `adi_fpga_image_loader_dir_get()` - Returns the target dir location for storing FPGA images
### Modified APIs
* All APIs (like ADC, CNCO, CFIR, PFILT, etc.) where block selection is a function argument, now uses a custom datatype `adi_apollo_blk_sel_t` instead of block specific enum or `uint8_t` or `uint16_t`
* Also within above APIs, the block selector will be applied with a mask based on device config (i.e. 4T4R vs 8T8R) to avoid invalid reg access
* ADC:
  * `adi_apollo_adc_cal()` - API now checks if the device being used, is fused with ADC Cal Data to make corresponding selection for ADC Init Cal Config (`adi_apollo_init_cal_cfg_e`) option
  * `adi_apollo_adc_init_cal()` - API now logs detailed ADC cal status for each channel, like error codes and masks, upon init calibration failure
* MCS_CAL:
  * `adi_apollo_mcs_cal_init_run()` - API now logs detailed MCS cal status, like error codes and masks, upon init calibration failure
* SERDES:
  * `adi_apollo_serdes_jrx_cal()` - API now wraps the new `adi_apollo_serdes_jrx_init_cal()` API with SerDes Init Cal Config enabled for both SerDes Packs


### Bug Fixes
* CLK_MCS:
  * `adi_apollo_clk_mcs_sync_trig_map()`: Fixed incorrectly selected base address for bf_set()
  * `adi_apollo_clk_mcs_input_power_status_get()`: Fixed incorrectly selected base address for bf_set()
* ADF4382:
  * `adi_adf4382_rfout_output_set()`: Fixed read-modify-write based bf_set() for double-buffered bitfields
  * `adi_adf4382_cpctrl_cp_i_set(), adi_adf4382_cpctrl_cptest_enable_set(),` <br>
    `adi_adf4382_cpctrl_bleed_polarity_set() and adi_adf4382_cpctrl_bleed_word_set()`: Fixed settings to double-buffered bit fields by writing to Reg 0x10


### FW Changes
* [20250120.5.352] - 2025-02-12
  ### Added
  - ADC: flash tilt calibration and factory gain trim
  - A scratch register to store core module calibration data fuse status in NVM
  ### Fixed
  - SerdesRX pack 1 calibration does not run when only pack 1 calibration is enabled
  - Serdes Rx does not report error code to API when foreground calibration fails
  - Firmware bootup fails with clock PLL during PVT
  ### Changed
  - ADC: updated the full-scale adjust range from [-3.0dB, +3.0dB] to [-6.0dB to +2.0dB]
  - ADC: enabled independent foreground calibration configurations for 8S channels
  - ADC: enabled the ability to warm boot calibration data independently for each 8S channel
  - ADC: each 8S channel will independently support all parameter set and get commands
  - ADC: added per-channel configuration for input protections
  - ADC: enabled Input Power input protection for 8S
  - Enabled DAC/ADC/JTx by default
  - Used DAC/ADC/JTx disable scratch registers to replace dac_enable, adc_enable and JTx lane_enables in device profile

### Example code
* Modified Common APIs
  * Removed `adi_apollo_cfg_adc_init_cal_cfg_set(.., .., ADI_APOLLO_INIT_CAL_DISABLED_WARMBOOT_FROM_NVM)` from startup
  * Added Serdes BG Cal as selection option to `adi_ads10_apollo_ex_cals_run()` API and clean up function implementation

* Modified Examples
  * All examples now use `adi_ads10_apollo_ex_cals_run()` API for running various Apollo Calibrations to facilitate code readability
  * Updated `MakeFile` and `debug_b0.mak` file in all examples to include ImageLoader related src code paths
  * Added SerDes BG cal to required examples
  * General code clean-up by using error-checking `#MACROs` instead of conditional print statements
  * ads10_apollo_ex_main
    * `bsync_tof.c` - Added SYSREF Output through channel BSYNC_1 for external reference
    * `jesd_loopback.c` - Fixed example description based on applied configuration
    * `jrx_eye_sweep.c and prbs.c` - Added `adi_ads10_apollo_ex_run_serdes_track_cal_get()` to showcase it's usage
    * `rx_adc_pave.c` - Added consideration for opposite sides (A/B) ADC polarity for 4T4R vs 8T8R device
    * `tx_jesd_file.c` - Fix proper use of `adi_fpga_apollo_core_tx_links_init()` for FPGA JTx link bring-up

* New Examples
  * `configureVUP` - Example to load a platform FPGA image



---




### [0.4.40] [sdk47] (2024-12-23)

### Features
* ADC dither mode (FW)
* Add Subclass 1 example using on-board clocking scheme with SYSREF generation and alignment
* Add support for internal clocking on FMCB Eval boards

### FW version
* FW version: B0 (X and B grade): 20241216.2.324 (*new*)

### Modified APIs
* CFIR:
  * `adi_apollo_cfir_coeff_pgm()` - Modified API to use paged SPI writes (vs indirect), significantly reducing the number of SPI transactions

### Bug Fixes
* CLK_MCS:
  * `adi_apollo_clk_mcs_trig_reset_serdes_enable()`: Fixed incorrect bitfield setting
* CNCO:
  * `adi_apollo_cnco_inspect()` - Fixed the active CNCO profile not restoring back to original before returning
* FNCO:
  * `adi_apollo_fnco_inspect()` - Fixed the active FNCO profile not restoring back to original before returning
* TX:
  * `adi_apollo_tx_fduc_configure()` - Fixed not setting the FNCO phase offset

### FW Changes
* [20241216.2.324] - 2024-12-16
  ### Added
    - ADC: Added dither mode configuration.
    - New error code for firmware stack overflow detection.
    - New error codes for internal RPC queue processing.
  ### Fixed
  - The get_init_completion mailbox command to signalize the correct status based on which core ran the calibration.
  ### Changed
  - The system will refuse a new initial calibration request if any core is already running a calibration.

### Example code
* New Examples
  * ads10_apollo_ex_main
    * `fullchip_mcs_sc1_dl_.c` - Subclass 1 deterministic latency using on-board clock for SYSREF generation and alignment

* Modified Examples
  * ads10_apollo_ex_main
    * `bsync_tof.c and mcs_cal.c` - Added support for ADF4030 VCO freq calculation instead of hardcoded settings
    * `fullchip_fsrc_sc1_ext_trig.c and fullchip_sc1_dl.c` - Updated default JRx phase adjust values



---



### [0.4.22] [sdk46] (2024-12-02)

### Features
* ADC Over-Range Samples and Threshold Set/Get APIs
* FPGA APIs for 2x H/W Sample Repeat Enable
* Initialization functions for ADL6331 and ADL6332
* Support for FMCB in `ads10_apollo_ex_main` examples

### FW version
* FW version: B0 (X and B grade): 20241111.2.300 (*new*)

### New APIs
* ADC: *Input Status and Over-Range Samples and Threshold*
  * `adi_apollo_adc_input_status_get()` - Gets ADC input protection status for monitored events
  * `adi_apollo_adc_ovr_threshold_set()` - Configures the ADC input level which will engage Over Range Protection
  * `adi_apollo_adc_ovr_threshold_get()` - Gets ADC's input level threshold which would engage Over-Range Protection
  * `adi_apollo_adc_ovr_samples_set()` - Configures the number of samples at or above the Over-Range Threshold which would engage protection
  * `adi_apollo_adc_ovr_samples_get()` - Gets the number of samples at or above the Over-Range Threshold which would engage protection
* CFG: *SERDES JRx Bridging Cal*
  * `adi_apollo_cfg_serdes_rx_bridging_cal_cfg_set()` - Configures the number of time bridging cal run for Rx SERDES (JRx) after init cal

### Modified APIs
* ARM: *Changed Memory Bank Division*
  * Inside API `adi_apollo_arm_fwload_pre_config()`, memory division changed for both ARM cores
    * Core 0: Changed from `ADI_APOLLO_DIV_IRAM_352K_DRAM_224K` to `ADI_APOLLO_DIV_IRAM_384K_DRAM_320K`
    * Core 1: Changed from `ADI_APOLLO_DIV_IRAM_288K_DRAM_288K` to `ADI_APOLLO_DIV_IRAM_352K_DRAM_352K`
  * For Enum `adi_apollo_cpu_memory_bank_div_sel_e`, related integral constants have also changed
  * Inside API `adi_apollo_arm_fwload_post_config()`, core 0 start is now also enabled
* DAC: *API implementation changed*
  Implementation for following APIs have updated to use mailbox commands for configuration instead of register access
    * `adi_apollo_dac_clk_trim_set()` and `adi_apollo_dac_clk_trim_get()`
    * `adi_apollo_dac_data_enable()`
* TX: *Sequence change in Tx Datapath enables*
  * API `adi_apollo_tx_configure()` does not configure ALL DAC Scrambler and Enable them. This is now moved to top level API `adi_apollo_cfg_data_path()`
* CFG: *Changed Datapath Config Sequence*
  * API `adi_apollo_cfg_data_path()` now configures DAC Scrambler and Enables them after setting up entire Tx Datapath using `adi_apollo_tx_configure()`
  * Not ALL DACs are configured and only those enabled in device profile parameter `dac_enable` will be set
  * Similarly, not ALL Tx and Rx Data FIFOs are enabled and only those in device profile are

### FW Changes
* [20241111.2.300] - 2024-11-19
### Added
  * ADC: ADC input status indicator "sticky bits"
  * ADC: Over range detection programmability
  * JRX: Auto-tune feature where SERDES JRx configuration parameters adjusts (or tunes) depending on the lane rate
  * JRX: Background calibration capability
### Changed
  * ADC: Lowered the default sensitivity for over range detection to 100 samples
  * ADC: Increased the maximum dwell time range (4T4R: 16 to 2^28-1 sample clock cycles, 8T8R: 8 to 2^27-1 sample clock cycles)
  * ADC: Raised the upper threshold limit to +2dBFS from the previous 0dBFS
  * JRX: Optimized foreground calibration time
  * JRX: Moved from Core-1 to Core-0
  * Core-0 IRAM and DRAM sizes
### Fixed
  * ADC: an issue when setting the minimum dwell time below the hardware limits
  * ADC/DAC: errors are not returned to users
  * ADC (8S): some channels on A side report foreground calibration errors for certain "adc_enable" configurations
### Known Issues
  * Apollo Silicons that aren't fused with NVM calibration data can get calibration errors if ADC init cal is performed with `_WARMBOOT_FROM_NVM` flag

### Example code
* New Examples
  * ads10_apollo_ex_main
    * `fullchip_sr_dr.c` - Apollo Rx/Tx Sample Repeat Dynamic Reconfig (DR)
    * `rx_adc_mux2.c` - Apollo Rx Mux2 Reconfig to demonstrate that the relative phase between ADC channels is maintained while alternating CNCO selections

* Removed Examples
  * ads10_apollo_ex_main
  * `rx_adc_sr_dr.c` and `tx_jesd_sr_dr.c`. Instead refer to fullchip version of example

* Modified Examples
  * ads10_apollo_ex_main
    * `main.c` - Added support for initializing Tx/Rx VGA used in FMCB Eval Boards. To use set `is_fmcb_eval = True`
    * `fullchip_fsrc_sc1_ext_trig.c` - Added jrx phase adjustment for consistent DL across profiles and FSRC change rate call in support of new FPGA image
    * `rx_adc_fd.c` - Updated fast detect dwell cycles to match FW changes
    * `tx_jesd.c`, `fullchip.c` and `fullchip_fsrc_dr.c` - Due to changes in the FPGA image for Tx HW FSRC, adi_apollo_jrx_rm_fifo_reset() is required to ensure apollo jrx link stability and improve DAC outputs

* Example Known Issues
  * `fullchip_hop` - FNCO hop, auto increment triggered by tmaster not working for DAC-B0 output. DAC-A0 works up to hop 16
  * `rx_adc_nz` - Device profile id00_uc06 now uses random slice mode by default, requires sequential profile
  * `rx_adc_cc` - Only works with FPGA images that support JRx hw transport
  * Examples where select BMEM APIs are used, with HSCI as active protocol, results in read or write transaction errors. 
  This is due to size limitations in HSCI HAL APIs used for streaming reads or writes


### FPGA
* Added
  * `adi_fpga_apollo_core_sysref_setup()` Configure FPGA SYSREF Counter settings.
  * `adi_fpga_apollo_hw_fsrc_sr_enable_set()` and `adi_fpga_apollo_hw_fsrc_sr_enable_get()` Set/Get HW Sample repeat 2x enable setting
* Removed
  * `adi_fpga_apollo_hw_fsrc_tx_pgm()`


---


### [0.4.0] [sdk45.1] (2024-10-21)

### Features
* Support for loading FW binaries for Apollo B-grade devices

### FW version
* FW version: B0 (X and B grade): 20241008.1.281 (*new*)

### FW Changes
* Added
  * Support for B-grade devices with production keys.


---


### [0.3.39] [sdk45] (2024-10-07)

### Features
* JRx phase adjust calculation for aligning Tx outputs with SC1
* Update Device Profile to v9.1.0 - DAC scrambling on by default
* ADC BG Cal Gating Group APIs
* FMCB Evaluation Board SPI driver to support attach devices (e.g. ADF4382, HMC7044, ADL6331 and ADL6332)
* ADF4030 SYSREF generation and BSYNC alignment
* Update example code sequence to decouple rx/tx links
* Initial support for VCU128 platform (see vcu128 startup guide)
* Example code support for new ADS10 FPGA images

### FW version
* FW version: B0: 20240806.6.275 (*new*)

### New APIs
* JRx: Phase Adjustment across links
  * `adi_apollo_jrx_phase_adjust_get()` - Get LMFC phase adjustment in conv_clk cycles
  * `adi_apollo_jrx_phase_adjust_calc()` - Calculates the jrx phase adjustment

* ADC: Support for BG Calibration gating
  * `adi_apollo_adc_bg_cal_grp_gate_set()` - Sets ADC cal gating on groups of calibrations preventing them from operating during background calibration.
  * `adi_apollo_adc_bg_cal_grp_gate_get()` - Gets ADC cal gating applied on groups of background calibration.

* UTILS:
  * `adi_api_utils_lcm()` - Least common multiple
  * `adi_api_utils_lcm_64()` - Least common multiple (64-bit)
  * `ADI_UTILS_MIN` and `ADI_UTILS_MAX` macros

### Example code
* New Examples
  * ads10_apollo_ex_main
    * `bsync_tof.c` Bsync Time-of-Flight measurements and alignment for ADF4030 to Apollo and FPGA
    * `fmcb_aux.c` Show SPI configuration for auxiliary devices (ADF4382, HMC7044, ADL6331 and ADL6332) on FMCB
    * `gpio_toggle.c` Configuring Apollo and corresponding, connected FPGA GPIOS as INPUTs and OUTPUTs (and vice-versa)
    * `jrx_eye_sweep.c` Performs multiple JRx vertical eye sweeps per lane and append data to corresponding file
    * `prbs.c` Performs JRx and JTx PRBS tests
    * `rx_adc.c` Performs Apollo Rx JESD Capture. If -b CLI flag is used, inject BMEM tone and does BMEM AWG Capture

* Removed Examples
   * `ads10_apollo_ex_gpio()` instead use ads10_apollo_ex_main\gpio_toggle.c
   * `ads10_apollo_ex_jrx_eye_sweep()` instead use ads10_apollo_ex_main\jrx_eye_sweep.c
   * `ads10_apollo_ex_prbs()` instead use ads10_apollo_ex_main\prbs.c
   * `ads10_apollo_ex_rx_adc()` instead use ads10_apollo_ex_main\rx_adc.c
   * `ads10_apollo_ex_tx_nco()` instead use ads10_apollo_ex_main\tx_nco.c
   * ads10_apollo_ex_main
      * `rx_bmem.c` instead use rx_adc.c with -b CLI arg
      * `tx_vec_file.c` instead use tx_jesd_file.c
      * `rx_adc_fsrc_dr.c`
      * `rx_adc_fsrc.c`
      * `tx_jesd_fsrc_dr.c`

* Modified Examples
  * ads10_apollo_ex_main
    * use `adi_fpga_apollo_bidir_init()`, `adi_fpga_apollo_rx_links_init()`, and `adi_fpga_apollo_tx_links_init()` to bring up jesd links
  * EXCTL macros used to support automated QA testing of example code

* Example Known Issues
  * `fullchip_hop` FNCO hop, auto increment triggered by tmaster not woorking for DAC-B0 output. DAC-A0 works up to hop 16.
  * `rx_adc_nz` id00_uc06 now uses random slice mode by default, requires sequential profile
  * `rx_adc_cc` Only works with FPGA images that support JRx hw transport

* Misc
  * `adi_vector_generate_square()` - Square wave vec gen function
  * Fixed Doxygen formatting typos

### Bug Fixes
* `adi_apollo_jrx_link_inspect()` Fixed incorrect readback of jrx `ns_minus1` parameter


### FPGA
* Added
  * `adi_fpga_apollo_core_bidir_init()` - Initialize Tx/Rx links simultaneously
  * `adi_fpga_apollo_core_rx_links_init()` - Initialize Rx links
  * `adi_fpga_apollo_core_tx_links_init()` - Initialize Tx links
  * `adi_fpga_apollo_core_spi2_sdo_alt_enable_set()` - Enables SPI2 SDO alternate GPIO (for FMCB HMC7044 read backs)
  * `adi_fpga_apollo_core_max_mem_size_check()` - Check if the requested memory size is within the FPGA memory limits.
  * `adi_fpga_apollo_gpio_aux_input_get()` - Sets the FPGA AUX GPIO's direction as INPUT and reads the GPIO pin's current state.
* Fixed
  * Fixed SW transport processing bug with FPGA raw image for L=12/S=6 JESD configurations

### FW Changes
* [20240806.6.275] - 2024-10-01
* Added
  * ADC: Calibration Group Gating.
* Changed
  * ADC: Increased Over Range detection sensitivity.
  * ADC: Improved ADC input protections.
  * ADC: 8S performance improvements.
  * Upgrade device profile to v9.1.0.
  * Disable device profile minor version check.
* Fixed
  * ADC: Resolved issue where +12 dBFS inputs could halt calibrations.
  * DAC: legacy part issue for Apollo Trim set mailbox commands
  * DAC: DDR spur cal initialisation issue.
  * Setting the sysref Rx Resistor termination to 100ohm default.
  * JTx lane Xbar setting issue in dual-link.
  * ADC interrupt mapping issue for 8R parts.
  * Moved the sysref_early_delay_ctrl out of clock mode check.

### Device Profile
* Device Profile 9.1.0

---

### [0.3.19] [sdk44] (2024-08-21)

### Features
* Update Device Profile to v9
* Remove Apollo A0 Support
* Update Apollo HSCI procedure for manual and auto link-up
* ADC Fast Mode Switch APIs
* JTx and JRx Lane Rate Adapt APIs
* Apollo BMEM APIs for CDDC and FDDC
* GPIO control for Sniffer
* DAC Standby Control Lock State APIs
* Generic Error check macros
* Loopback LB1 and LB2 APIs
* FPGA APIs for JTx and JRx line rate division

### FW version
* FW version: B0: 20240806.2.253 (*new*)

### New APIs
* BMEM *Support for HSDIN, CDDC and FDDC*
    * `adi_apollo_bmem_hsdin_awg_config()`
    * `adi_apollo_bmem_hsdin_awg_start()`
    * `adi_apollo_bmem_hsdin_awg_stop()`
    * `adi_apollo_bmem_hsdin_awg_sram_set()`
    * `adi_apollo_bmem_hsdin_awg_sample_write()`
    * `adi_apollo_bmem_hsdin_awg_sample_shared_write()`
    * `adi_apollo_bmem_hsdin_capture_config()`
    * `adi_apollo_bmem_hsdin_capture_run()`
    * `adi_apollo_bmem_hsdin_capture_get()`
    * `adi_apollo_bmem_hsdin_delay_sample_set()`
    * `adi_apollo_bmem_hsdin_delay_hop_set()`
    * `adi_apollo_bmem_hsdin_delay_sample_config()`
    * `adi_apollo_bmem_hsdin_delay_hop_config()`
    * `adi_apollo_bmem_hsdin_delay_start()`
    * `adi_apollo_bmem_cddc_delay_sample_set()`
    * `adi_apollo_bmem_cddc_delay_hop_set()`
    * `adi_apollo_bmem_cddc_delay_sample_config()`
    * `adi_apollo_bmem_cddc_delay_hop_config()`
    * `adi_apollo_bmem_cddc_delay_start()`
    * `adi_apollo_bmem_fddc_delay_sample_set()`
    * `adi_apollo_bmem_fddc_delay_hop_set()`
    * `adi_apollo_bmem_fddc_delay_sample_config()`
    * `adi_apollo_bmem_fddc_delay_hop_config()`
    * `adi_apollo_bmem_fddc_delay_start()`
* DAC *Standby Control Lock*
    * `adi_apollo_dac_standby_lock_set()`
    * `adi_apollo_dac_standby_lock_get()`
* CLK_MCS *Multichip Sync*
    * `adi_apollo_clk_mcs_trig_sync_enable_get()`
* APOLLO_HAL: *HSCI Auto Link-up*
    * `adi_apollo_hal_alink_tbl_get()`
* JRX: *JRX Lane Rate Adapt*
    * `adi_apollo_jrx_lr_adapt_set()`
    * `adi_apollo_jrx_lr_adapt_get()`
* JTX: *JTX Lane Rate Adapt*
    * `adi_apollo_jtx_lr_adapt_set()`
    * `adi_apollo_jtx_lr_adapt_get()`
* SNIFFER *FFT Sniffer*
    * `adi_apollo_sniffer_fft_enable_set()`
    * `adi_apollo_sniffer_fft_hold_set()`
    * `adi_apollo_sniffer_fft_done_get()`
    * `adi_apollo_sniffer_fft_data_get()`
* LOOPBACK *Loopback*
    * `adi_apollo_loopback_lb1_enable_set()`
    * `adi_apollo_loopback_lb1_cduc_enable_set()`
    * `adi_apollo_loopback_lb1_blend_set()`
    * `adi_apollo_loopback_lb2_enable_set()`
    * `adi_apollo_loopback_lb2_fduc_enable_set()`

### Changed APIs
* ADC *Renamed for clarity*
    * Renamed `adi_apollo_adc_slice_mode_switch_enable_set()` to `adi_apollo_adc_mode_switch_enable_set()`
    * Renamed `adi_apollo_adc_slice_mode_switch_prepare()` to `adi_apollo_adc_mode_switch_prepare()`
    * Renamed `adi_apollo_adc_slice_mode_switch_execute()` to `adi_apollo_adc_mode_switch_execute()`
    * Renamed `adi_apollo_adc_slice_mode_switch_restore()` to `adi_apollo_adc_mode_switch_restore()`
* APOLLO_HAL *HSCI Auto Link-up*
    * Added parameters `hscim_mosi_clk_inv` and `hscim_miso_clk_inv` to `adi_apollo_hal_auto_linkup()`
* SERDES *Align with API naming conventions*
    * Renamed `adi_ads10_serdes_jrx_spo_set_dir()` to `adi_apollo_serdes_jrx_spo_dir_set()`
* SYSCLK_COND *Align with API naming conventions*
    * Renamed `adi_apollo_sysclk_bg_cond_cal_start()` to `adi_apollo_sysclk_cond_bg_cal_start()`
    * Renamed `adi_apollo_sysclk_bg_cond_cal_stop()` to `adi_apollo_sysclk_cond_bg_cal_stop()`
    * Renamed `adi_apollo_sysclk_bg_cond_cal_resume()` to `adi_apollo_sysclk_cond_bg_cal_resume()`
* TX *Changed parameter*
    * Changed parameter for `adi_apollo_tx_inv_sinc_configure()`. Parameter `idx` now refers to enum `adi_apollo_cduc_path_idx_e` and not `adi_apollo_cduc_idx_e`

### Deprecated APIs
* BMEM *Following APIs only targeted the HSDIN block, see function docstring for replacement*
    * Deprecated `adi_apollo_bmem_sample_delay_set()`
    * Deprecated `adi_apollo_bmem_hop_delay_set()`
    * Deprecated `adi_apollo_bmem_sample_delay_config()`
    * Deprecated `adi_apollo_bmem_hop_delay_config()`
    * Deprecated `adi_apollo_bmem_delay_start()`
    * Deprecated `adi_apollo_bmem_awg_data16_config()`
    * Deprecated `adi_apollo_bmem_awg_data16_2x_config()`
    * Deprecated `adi_apollo_bmem_capture_config()`
    * Deprecated `adi_apollo_bmem_capture_normal_set()`
    * Deprecated `adi_apollo_bmem_capture_normal_run()`
    * Deprecated `adi_apollo_bmem_awg_config()`
    * Deprecated `adi_apollo_bmem_awg_normal_set()`
    * Deprecated `adi_apollo_bmem_awg_normal_slow()`
    * Deprecated `adi_apollo_bmem_awg_normal_start()`
    * Deprecated `adi_apollo_bmem_awg_normal_stop()`
    * Deprecated `adi_apollo_bmem_awg_normal_sram_set()`
    * Deprecated `adi_apollo_bmem_sram_get()`

### Removed APIs
* ARM *Functions only applicable for Apollo A0*
    * Removed: `adi_apollo_arm_profile_write_pre_config()`
* SYSCLK_COND *Functions only applicable for Apollo A0*
    * Removed `adi_apollo_sysclk_cond_caldata_get()`
    * Removed `adi_apollo_sysclk_cond_caldata_set()`
    * Removed `adi_apollo_sysclk_cond_calconfig_get()`
    * Removed `adi_apollo_sysclk_cond_calconfig_set()`
    * Removed `adi_apollo_sysclk_cond_on_demand()`
    * Removed `adi_apollo_sysclk_cond_caldata_validate()`
    * Removed `adi_apollo_sysclk_cond_centering_done()`

### Example code
* New Examples
  * ads10_apollo_ex_main
    * `fullchip.c`  Fullchip configuration for FSRC and Non-FSRC profiles
    * `rx_bmem.c`   BMEM AWG configuration
    * `rx_adc_ms.c` ADC Fast Mode Switch configuration using FW, SPI and GPIO control
    * `tx_jesd.c`   JESD TX configuration for FSRC and Non-FSRC profiles
    * `loopback1_2` Loopback 1 and loopback 2 configuration

* Removed Examples
  * `ads10_apollo_ex_fullchip()` instead use `ads10_apollo_ex_main\fullchip.c`
  * `ads10_apollo_ex_rx_bmem()` instead use `ads10_apollo_ex_main\rx_bmem.c`
  * `ads10_apollo_ex_slow_switch()` instead use `ads10_apollo_ex_main\rx_adc_ms.c`
  * `ads10_apollo_ex_tx_jesd()` instead use `ads10_apollo_ex_main\tx_jesd.c`
  * ads10_apollo_ex_main
    * `fullchip_fsrc.c` instead use `fullchip.c`
    * `rx_bmem_fsrc.c` instead use `rx_bmem.c`
    * `tx_jesd_fsrc.c` instead use `tx_jesd.c`

* Modified Examples
  * Removed Apollo A0 specific configuration
  * Replaced static vector generation and loading functions with common example vec functions
  * ads10_apollo_ex_main
    * `lb0_bmem_delay_hop.c` Replace deprecated BMEM APIs with new
    * `rx_adc_bmem.c` Replace deprecated BMEM APIs with new
    * `rx_adc_nz.c` Replace deprecated BMEM APIs with new
    * `rx_bmem_cfir.c` Replace deprecated BMEM APIs with new
    * `rx_bmem_ddc.c` Replace deprecated BMEM APIs with new
    * `rx_bmem_delay.c` Replace deprecated BMEM APIs with new
    * `rx_bmem_pfilt.c` Replace deprecated BMEM APIs with new
    * `mcs_cal` Updated `adi_apollo_mcs_cal_config_t` struct
    * `rx_sniffer.c` Updated exmaple for GPIO based control

* Fixed Examples
  * `tx_jesd_cfir` Filter coeffs now applied for chans A1/B1. Changed id00_uc06_F profile.

* Example Known Issues
  * `rx_adc_nz` id00_uc06 now uses random slice mode by default, requires sequential profile
  * `rx_adc_cc` Only works with FPGA images that supprt JRx hw transport
  * `tx_vec_file` core_not_running_wait times out, example continues and links still come up, transmit passes


* Misc
  * Compile with `ADI_EN_STDERR` to enable stack trace to program stderr


### Bug Fixes
  * Device Profile 9
    * Tx path SUMMER supports 8T8R
    * Tx path INVSINC supports 8T8R

### FPGA APIs
* New APIs
  * FPGA_APOLLO_CLK *APIs for JTX/JRX line rate division*
    * `adi_fpga_apollo_clk_line_rate_div_set()`
    * `adi_fpga_apollo_clk_line_rate_div_get()`
    * `adi_fpga_apollo_clk_line_rate_div_calc()`
    * `adi_fpga_apollo_clk_line_rate_div_pgm()`

### FW Changes
* Added
  * Support both ADC slice mode fast switch (with GPIO) and slow switch (with software interrupt) in 4T4R parts.
  * DAC: ability to set/get clock duty cycle and phase control options through ParameterSet() and ParameterGet().
  * DAC: ability to change control of DAC standby through ParameterSet().
  * DAC: ability to read control path for DAC standby status through ParameterGet().
  * ADC: Support for 14G.
  * ADC: Support for fast detect status readback.
* Changed
  * Upgrade device profile to v9.0.0.
  * Initialize init_cal_cfg scratch registers only when they are not set by the user before loading the firmware.
  * ADC: improved foreground calibration with fused calibration data.
  * ADC: improved 20G analog trim values.
* Fixed
  * Fixed a pack index related bug for requesting JRx calibration status.
  * Added report ParameterSet() and ParameterGet() error code to API.
  * Added missing Run() for ADC/DAC/JRx FSM commands.
  * Fixed Stack pointer issue for tiny enclave enabled parts.

### Device Profile
* Device Profile 9
  * Adds new device configuration fields
  * Adds support for Multi Chip Sync (MCS)
  * Fixes Tx summer mux and INVSINC for 8T8R devices
  * DP8 profiles not directly compatible with DP9. Requires migration.


---

### [0.2.55] [sdk43] (2024-06-14)

### Features
* New Jrx link/rate-match-fifo IRQs and status APIs
* Fast detect flag status from FW
* Example code updates
  * fullchip_fsrc_sr - use manual dynamic sync, IRQs
  * fullchip_hop - 8t8r support
  * CFIR sparse mode
* FW Updates

### FW version
* FW version: A0: 20230406.5.563
* FW version: B0: 20240531.1.215 (*new*)

### New APIs
* `adi_apollo_adc_fast_detect_status_get()` Read fast detect status from FW
* `adi_apollo_jrx_j204c_irq_enable_set()` Enable JRx 204C IRQs
* `adi_apollo_jrx_j204c_irq_enable_get()` Get enable status for JRx 204C IRQs
* `adi_apollo_jrx_j204c_irq_get()` Get status of JRx 204C IRQs
* `adi_apollo_jrx_j204c_irq_clear()` Clear JRx 204C IRQ sticky bits

### Example code
* New Examples
  * `fullchip_sparse_cfir.c` CFIR sparse filter mode example
* Updated Examples
  * `fullchip_fsrc_dr.c` Modified to use manual reconfig trig (repleaces internal trigger). Uses new Jrx IRQ and RMFIFO IRQ apis to monitor link/fifo status.
  * `fullchip_hop.c` Add support for 8T8R devices.
* Removed Examples
  * `tx_vec_deep.c` No longer supported.
  * `tx_jesd_fsrc_sc1_ext_trig.c` No longer supported. See `fullchip_fsrc_sc1_ext_trig.c` example.
* New Common/Utility APIs
  * `adi_ads10_apollo_ex_clk_power_cal()` Displays clk power level, if ADF4382 calibrate until power level is good.
* Misc
  * For ADF4382 clock sources, add calibration for proper power level
  * Fixed missing enable of A1/B1 bmem in `rx_bmem_pfilt.c` example
  * Fixed missing fpga fsrc enable for HW based fsrc fpga images

### Removed/Changed APIs
* Deprecated
  * `adi_apollo_jrx_clear_fec_errors()` Use `adi_apollo_jrx_j204c_irq_clear()`
  * `adi_apollo_jrx_fec_errors` Use `adi_apollo_jrx_j204c_irq_get()`

### Bug Fixes
  * `adi_apollo_arm_ram_boot_error_check()` now reads the correct system ram boot status and not mailbox status

### New FPGA APIs
* `adi_fpga_apollo_fsrc_n_m_ratio_set()` Set FSRC values given n/m parameters (n >= m)
* `adi_fpga_apollo_core_jrx_link_error_count_get()` Get the number of Sh and Emb errors for a specific link
* `adi_fpga_apollo_core_jrx_link_error_cnt_reset()` Resets Rx Sh and Rx Emb lock error count on all active links

### FW Changes
* Implemented both the old and new equations for TMUs. (Done for readback accuracy, will increase temp values at high temperatures relative to old firmware)
* Disabled data path FDUC/CDUC0/CDUC1 (both east side and west side ) overflow interrupt. 
* Support fast detect status report.
* Apply JTx PHY firmware workaround to fix JTx aging issue. (May increase power consumption relative to earlier versions of FW.

### Known Issues
* Examples
  * These examples not tested. Recommend using fullchip versions
    * tx_dac_trim
    * rx_adc_fsrc
    * rx_adc_sr_dr
    * rx_bmem_fsrc
  * rx_adc_cc not tested. Plan to remove from future release.
  * FPGA HW fsrc images don't support jesd m=16 (id99_uc02_8G). Results in capture errors.

---

### [0.2.36] (2024-04-16)

### Features
* PFILT operating modes for real-ntap, half complex and full matrix. (Supports 8T8R devices)
* ADC PFILT averaging (supports 8T8R dvices)
* ADF4030 device APIs
* GPIO hopping support APIs
* Fast detect for muliple channels
* ADC status API (e.g. tracking cal state,slice modes)
* Expanded example code for device hopping. Added Rx/Tx CNCO and Rx/Tx FNCO w/ various trigger options
* FPGA support for RAW image and SW transport layer

### FW version
* FW version: A0: 20230406.5.563
* FW version: B0: 20240403.1.197 (*new*)

### New APIs
* BMEM
  * `int32_t adi_apollo_bmem_awg_data16_2x_config` Configures BMEM to AWG with 16-bit sample data for 8T8R device
* Hopping
  * `adi_apollo_gpio_hop_profile_configure()` Set up GPIO profile selection for user-defined gpio configuration (no quick config)
  * `adi_apollo_gpio_hop_profile_calc()` GPIO profile select bit calculation
  * `adi_apollo_gpio_hop_profile_qc_calc()` GPIO profile select bit calculation for given quick config
  * `adi_apollo_gpio_hop_block_configure()`  Set up GPIO block selection for user-defined gpio configuration (no quick config)
  * `adi_apollo_gpio_hop_block_calc()` GPIO block select bit calculation
  * `adi_apollo_gpio_hop_block_qc_calc()` GPIO block select bit calculation for given quick config
  * `adi_apollo_gpio_hop_side_configure()` Set up GPIO side selection for user-defined gpio configuration (no quick config)
  * `adi_apollo_gpio_hop_side_calc()` GPIO side select bit calculation
  * `adi_apollo_gpio_hop_side_qc_calc()` GPIO side select bit calculation for given quick config
  * `adi_apollo_gpio_hop_slice_configure()` Set up GPIO slice selection for user-defined gpio configuration (no quick config)
  * `adi_apollo_gpio_hop_slice_calc()` GPIO slice select bit calculation
  * `adi_apollo_gpio_hop_slice_qc_calc()` GPIO slice select bit calculation for given quick config
  * `adi_apollo_gpio_hop_terminal_configure()` Set up GPIO terminal selection for user-defined gpio configuration (no quick config)
  * `adi_apollo_gpio_hop_terminal_calc()` GPIO terminal select bit calculation
  * `adi_apollo_gpio_hop_terminal_qc_calc()` GPIO terminal select bit calculation for given quick config
  * `adi_apollo_gpio_hop_block_select_set()` GPIO/SPI select for block profile hopping
  * `adi_apollo_gpio_hop_block_set()` Block set for profile hopping (block_select must be set to SPI)
  * `adi_apollo_gpio_hop_slice_select_set()` GPIO/SPI select for slice profile hopping. If SPI, overrides GPIO select for slice, side, and terminal.
  * `adi_apollo_gpio_hop_fnco_enable_set()` Enable hopping for selected FNCOs (slice_select must be set to SPI)
  * `adi_apollo_gpio_hop_cnco_enable_set()` Enable hopping for selected CNCOs (slice_select must be set to SPI)
  * `adi_apollo_gpio_hop_cfir_enable_set()` Enable hopping for selected CFIR datapaths (slice_select must be set to SPI)
  * `adi_apollo_gpio_hop_pfilt_enable_set()` Enable hopping for selected PFILTs (slice_select must be set to SPI)
  * `adi_apollo_gpio_hop_dynamic_config_enable_set()` Enable hopping for selected reconfig blocks (slice_select must be set to SPI)
  * `adi_apollo_gpio_hop_bmem_delay_enable_set()` Enable hopping for selected BMEMs (delay mode only) (slice_select must be set to SPI)
* PFILT
  * `adi_apollo_pfilt_coeff_ntap_set()` - Set PFILT coeffs for Real-N, Real-N/2 or Real-N/4 modes
  * `adi_apollo_pfilt_coeff_half_complex_set()` Set PFILT coeffs for half-complex mode
  * `adi_apollo_pfilt_coeff_full_matrix_set()` Set PFILT coeffs for full-matrix mode
  * `adi_apollo_pfilt_half_complex_delay_set()` Set PFILT delay fir half-complex mode
  * `adi_apollo_pfilt_coeff_transfer()` Select filter bank to transfer to active mode
  * `adi_apollo_pfilt_ave_mode_set()` Set the Rx PFILT ADC averaging mode
  * `adi_apollo_pfilt_data_type_set()` Set the PFILT data type for real or complex processing
* RXMUX
  * `adi_apollo_rxmux_sample_xbar_set()` Configure the Rx sample crossbar
* TRIGTS
  * `adi_apollo_trigts_mst_mute_mask_set()` Selects which mute mask to use for muting after specified count number
  * `adi_apollo_trigts_mst_mute_mask_count_set()` Sets the number of pulses to enable trigger master before muting
* ADC
  * `adi_apollo_adc_status_get()` Get ADC status data

### New FPGA APIs
* `adi_fpga_apollo_gpio_output_word_set()` Sets the FPGA GPIO directiosn as OUTPUT and drives the GPIO pins to given state.

### Example code
* New Examples
  * `fullchip_pfilt.c` Demonstrates various PFILT configurations for Rx and Tx paths. Includes real-Ntaps, half-complex and full matrix.
  * `jesd_loopback.c` Demonstrates Apollo JESD loopback
  * `rx_adc_bmem.c` Raw capture ADC samples into BMEM. No JESD or datapath.
  * `rx_adc_pave.c` Demonstrates PFILT ADC averaging mode
* New Common/Utility APIs
  * `adi_ads10_apollo_ex_inspect_adc_all()` Print the state of all ADC channels
  * `adi_ads10_apollo_extras_create_ramp()` Generate ramp data
* Misc
  * `fuillchip_hop.c` Now demonstrates CNCO an FNCO hopping with triggering modes
  * Added id99_uc02 to `ads10_apollo_ex_fullchip` example (20G, 1x1x)
  * Use FPGA feature flags to determine capability. Removed date coded method.

### Removed/Changed APIs
* Fast Detect
  * `adi_apollo_fast_detect_enable_set()` was removed. Use `adi_apollo_adc_fast_detect_pgm()`
  * `adi_apollo_fast_detect_upper_threshold_set()` was removed. Use `adi_apollo_adc_fast_detect_pgm()`
  * `adi_apollo_fast_detect_lower_threshold_set()` was removed. Use `adi_apollo_adc_fast_detect_pgm()`
  * `adi_apollo_fast_detect_dwell_cycle_set()` was removed. Use `adi_apollo_adc_fast_detect_pgm()`
* Loopback
  * `adi_apollo_lb0_*()` APIs have been renamed to `adi_apollo_loopback_lb0_*()` for improved clarity with other loopback features.
* PFILT
  * `adi_apollo_pfilt_pgm()` has been replaced by`adi_apollo_pfilt_mode_pgm()`
* Enums
  * `ADI_APOLLO_FUNC_ADC_FD0` through `ADI_APOLLO_FUNC_ADC_FD7` have changed to `ADI_APOLLO_FUNC_ADC_FD_A0 through ADI_APOLLO_FUNC_ADC_FD_B3` for clarity.
* Headers
  * `adi_apollo_lb0.h` is replaced by `adi_apollo_loopback.h`

### Bug fixes
* Fix [in]/[out] doxy comment in mcs cal


### [0.2.18] (2024-03-14)

### Features
* MCS calibration
* Loopback 0 through BMEM with Delay
* Sample repeat vector/capture 
* TX-CNCO hopping (direct regmap, triggered)
* PFILT and CFIR hopping (direct regmap/gpio, triggered)
* Set swing/pre-emphasis/post-emphasis levels for multiple serdes lane

### FW version
* FW version: A0: 20230406.5.563
* FW version: B0: 20240226.3.3  (*new*)

### New APIs
* BMEM
  * `adi_apollo_bmem_sample_delay_set` Sets BMEM sample delay
  * `adi_apollo_bmem_hop_delay_set()` Sets BMEM delay for the 4 hopping profiles
  * `adi_apollo_bmem_sample_delay_config()` Configures BMEM to delay
  * `adi_apollo_bmem_hop_delay_config()` Configures BMEM to delay with hopping mode
  * `adi_apollo_bmem_delay_start()` Starts BMEM sample delay (after one-shot)
* LB0
  * `adi_apollo_lb0_bmem_enable_set()` Enable/disable loopback from BMEM.
* CFIR
  * `adi_apollo_cfir_profile_sel_mode_set()` Set the CFIR profile selection and hopping mode
  * `adi_apollo_cfir_next_hop_num_set()` Set the next CFIR profile selection or hop
* PFILT
  * `adi_apollo_pfilt_profile_sel_mode_set()` Set the PFILT profile selection and hopping mode
  * `adi_apollo_pfilt_next_hop_num_set()` Set the next PFILT profile selection or hop
* TRIGTS
  * `adi_apollo_trigts_pfilt_trig_mst_config()` Configures PFILT trigger master's offset and period
  * `adi_apollo_trigts_cfir_trig_mst_config()` Configures CFIR trigger master's offset and period
  * `adi_apollo_trigts_counter_get()` Return the timestamp counter value
  * `adi_apollo_trigts_ts_reset_mode_set()` Set the timestamp counter reset mode (via SPI or SYSREF)
  * `adi_apollo_trigts_ts_reset()` Reset the timestamp counter
  * `adi_apollo_trigts_reset_done_clear()` Clears the timestamp reset done status sticky bit
  * `adi_apollo_trigts_reset_done_get()` Returns the reset-done status following a timestamp reset request
* JTX
  * `adi_apollo_jtx_multi_lane_swing_emphasis_set()` Set swing/pre emphasis/post emphasis levels for multiple serdes lane
  * `adi_apollo_jtx_multi_lane_swing_emphasis_get()` Get swing/pre emphasis/post emphasis levels for multiple serdes lane
* MCS_CAL
  * `adi_apollo_mcs_cal_config_set()` Load configuration data required for MCS Calibration.
  * `adi_apollo_mcs_cal_init_run()` Execute an Initial MCS Calibration.
  * `adi_apollo_mcs_cal_init_status_get()` Get status of a MCS Initial Calibration run
  * `adi_apollo_mcs_cal_parameter_set()` Set select parameters related to MCS Calibration.
  * `adi_apollo_mcs_cal_tracking_decimation_set()` Set decimation for MCS Tracking Calibration to do TDC measurement
  * `adi_apollo_mcs_cal_tracking_enable()` Enable/Disable MCS Tracking Calibration
  * `adi_apollo_mcs_cal_tracking_initialize_set()` Initialize MCS Tracking Calibration. This initializes mcs tracking fw with user-defined values.
  * `adi_apollo_mcs_cal_fg_tracking_run()` Execute MCS Foreground Tracking Calibration for fast SysRef alignment.
  * `adi_apollo_mcs_cal_bg_tracking_run()` Execute MCS Background Tracking Calibration for maintaining SysRef alignment.
  * `adi_apollo_mcs_cal_bg_tracking_freeze()` Halts(freezes) MCS Background Tracking Calibration. TDC measurement and phase correction routine will be halted.
  * `adi_apollo_mcs_cal_bg_tracking_unfreeze()` Restarts(unfreezes) MCS Background Tracking Calibration. TDC measurement and phase correction routine will start again.
  * `adi_apollo_mcs_cal_bg_tracking_abort()` Stops(aborts) MCS Background Tracking Calibration. TDC measurement and phase correction routine will stop.
  * `adi_apollo_mcs_cal_force_fg_tracking_run()` Force a MCS Foreground Tracking Calibration for fast SysRef alignment.
  * `adi_apollo_mcs_cal_force_bg_tracking_run()` Force a ONE TIME MCS Background Tracking Calibration for maintaining SysRef alignment.
  * `adi_apollo_mcs_cal_coarse_jump_set()` Performs a coarse bleed current adjustment on ADF4382.
  * `adi_apollo_mcs_cal_tracking_status_get()` Get status of MCS Tracking Calibration.

### New FPGA APIs
* `adi_fpga_apollo_transmit_write()` Write vector to FPGA memory for transmit

### New HMC7044 APIs 
* `adi_hmc7044_device_gpi_config_set()` Configure GPI
* `adi_hmc7044_device_gpi_mode_set()` Configure GPI Mode
* `adi_hmc7044_device_gpi_enable_set()` Configure GPI Enable State

### Example code
* Added `fullchip_hop.c` Demonstrates various hopping options for CFIR and PFILT. Timestamp counter resets
* Added `mcs_cal.c` Configure and run Multi Chip Sync Calibration on Apollo B0
* Added `rx_adc_sr_dr.c` Apollo Rx sample repeat data path test with SPI triggered FDDC/CDDC change
* Added `tx_jesd_sr_dr` Apollo Tx data path test with SPI triggered SR ratio change
* Added `rx_bmem_delay.c` Apollo BMEM delay test
* Added `tx_nco_ffh.c` Apollo Tx data path Fast-Frequency Hopping
* Added `ads10_apollo_ex_bgcal_clk_cond` Demonstrates starting/pausing/resuming background clock condition cal
` 

### Misc Changes
* The default CNCO hop mode in device profiles is now `ADI_APOLLO_NCO_CHAN_SEL_DIRECT_REGMAP`

### Bug fixes
* Fixed FDUCs not enabled properly for device profile id00_uc13.

### [0.1.76] (2024-02-05)

### Features
* Initial support for Apollo 8T8R 
* Fast Detect feature APIs
* FFT Sniffer feature APIs
* Background Clock Conditioning support

### FW version
* FW version: A0: 20230406.5.563
* FW version: B0: 20240129.1.166 (*new*)

### New APIs
* Fast Detect:
  * `adi_apollo_adc_fast_detect_enable_set()` Enable/disable fast detect
  * `adi_apollo_adc_fast_detect_upper_threshold_set()` Set upper threshold for fast detect
  * `adi_apollo_adc_fast_detect_lower_threshold_set()` Set lower threshold for fast detect
  * `adi_apollo_adc_fast_detect_dwell_cycle_set()` Set dwell cycles for fast detect
  * `adi_apollo_adc_fast_detect_inspect()` Inspect fast detect settings
* FFT Sniffer
  * `adi_apollo_sniffer_enable_set()` Enable/disable sniffer
  * `adi_apollo_sniffer_adc_mux_set()` Set ADC mux for sniffer
  * `adi_apollo_sniffer_init()` Program FFT Sniffer
  * `adi_apollo_sniffer_data_get()`
* Clock Conditioning
  * `adi_apollo_sysclk_bg_cond_cal_start()` Execute a background clock conditioning calibration
  * `adi_apollo_sysclk_bg_cond_cal_stop()` Stop background clock conditioning calibration
  * `adi_apollo_sysclk_bg_cond_cal_resume()` Resume background clock conditioning calibration

### New FPGA APIs
* `adi_apollo_sniffer_enable_set()` Setup FFT Sniffer (before oneshot sync) or disable
* `adi_apollo_sniffer_pgm()` Get Sniffer FFT data

### Changed APIs
* Changed parameters for `adi_apollo_txmux_summer_block_set()`
* Removed structure `adi_apollo_summer_pgm_t`. No longer relevant.
* Removed redundant fields in inspect structures `adi_apollo_fnco_inspect_t` and `adi_apollo_cnco_inspect_t`

### Changed FPGA APIs
* N/A

### Misc Changes
* Device profile version patch 1

### Example Code
* Added new example file `fullchip_8t8r` in `ads10_apollo_ex_main` (8T8R device)
* Added new example file `rx_adc_fd` in  `ads10_apollo_ex_main` (Fast Detect)
* Added new example file `rx_sniffer` in `ads10_apollo_ex_main` (FFT Sniffer)
* Display FPGA ref clock for JESD204B configurations

### Bug fixes
* Fixed JESD204B type indicator field in `adi_apollo_jtx_link_inspect()` and `adi_apollo_jrx_link_inspect()`

### New Profiles
* id99_uc02_8G (8T8R, JESD204B, 8G/8G)
* id81_uc07 (1T8R, JESD204B, 8G/8G)
* id81_uc10 (8R, JESD204B, 8G)



### [0.1.55] (2023-12-22)

### Features
* GPIO interrupt APIs for overrange detection
* JRx serdes eye monitor APIs
* FPGA FSRC capture API refactor, support for SW FSRC
* FMCx customer board power measurement APIs.
* Profiles id00_UC13/14/15 added to fullchip subclass1 example
* A2 silicon FW update for Fs/N fix

### FW version
* FW version: A0: 20230406.5.563 (*new*)
* FW version: B0: 20231215.1.4 (*new*)

### New APIs
* `adi_apollo_arm_fw_irq_out_get()` Get FW IRQ OUT status 
* `adi_apollo_arm_fw_irq_out_clear()` Clear FW IRQ sticky bits
* `adi_apollo_cduc_irq_enable_set()` Enables/disables coarse DUC IRQ generation
* `adi_apollo_dformat_overflow_status_get()` Get DFORMAT (Rx) overflow status
* `adi_apollo_dformat_overflow_status_clear()` Clear DFORMAT (Rx) overflow status
* `adi_apollo_fduc_irq_enable_set()` Enable FDUC overflows to trigger an interrupt

* `adi_apollo_serdes_jrx_horiz_eye_sweep()` Perform a Horizontal Eye Sweep for select JRx Serdes lane
* `adi_apollo_serdes_jrx_horiz_eye_sweep_resp_get()` Get Response of a Horizontal Eye Monitor Test for select JRx serdes lane
* `adi_apollo_serdes_jrx_vert_eye_sweep()` Perform a Vertical Eye Sweep for select JRx Serdes lane
* `adi_apollo_serdes_jrx_vert_eye_sweep_resp_get()` Get Response of a Vertical Eye Monitor Test for select JRx serdes lane

* `adi_api_utils_ratio_composition()` Utility function for determining FSRC ratio from device regs

### New FPGA APIs
* `adi_fpga_apollo_capture_transfer_setup()` Malloc memory for capture buffer. Return information relating to the transfer of capture samples.
* `adi_fpga_apollo_capture_cap_buffer_get()` Get Capture Buffer
* `adi_fpga_apollo_capture_cleanup_transfer()` Free memory allocated for capture transfer
* `adi_fpga_apollo_capture_set_fsrc_vals()` Set FSRC values

### Changed FPGA APIs
* `adi_fpga_apollo_capture_get()` param list modified
* `adi_fpga_apollo_fsrc_invalid_samples_remove()` Removed
* `adi_fpga_apollo_virtual_conv_memory_allocate()` Removed
* `adi_fpga_apollo_write_virtual_conv_data_to_file()` Removed
* `adi_fpga_apollo_virtual_conv_memory_deallocate()` Removed

### Changed APIs
* N/A

### Example Code
* New: `ads10_apollo_ex_jrx_eye_sweep` JRx serdes eye sweep
* New: `ads10_apollo_ex_power_modules` 
* Update: `fullchip_sc1_dl` now includes UC13 (4GIBW), UC14/UC15 profiles.

### Bug fixes
* N/A

### Misc
* Added `adi_ltc2977_core_device_id_get()` Checks that the chip at device address is an LTC2977
* Added `adi_ltm4681_core_reset()` Forces device to turn off both channels, load the operating memory from internal EEPROM, clear faults and then perform a soft-start of PWM channels, if enabled.
* Added `adi_ltm4681_core_fault_status_get()` Get fault status registers
* Added `adi_ltm4681_core_fault_status_clear()` Clear fault status registers
* Added `adi_ltc2980_core_fault_status_get()` Get 2-byte fault status value
* Added `adi_ltc2980_core_fault_status_clear()` Clear fault status
* Added `adi_ltc2980_core_device_id_get()` Checks that the chip at device address is an LTC2980
* Added `adi_ltc2980_core_mfr_config_get()` Reads ADC measured output voltage for selected pages/channels

### Known Issues
* The function to check the manufacturer device id for LTC2980A always returns true (adi_ltc2980_core_device_id_get). This is used in the example to check if the device at address 0x5C is LTC2980 or LTC2977, so even if the LTC2977 is connected it attempts to read LTC2980B and C, causing an error. This error occurs after reading the LTC2980A, which is the same as the LTC2977, so the actual voltages reported are still correct.

### [0.1.47] (2023-11-30)

### Features
* JESD204B sync enable
* Fullchip FSRC, FDDC, FDUC w/ SC1. APIs and example code.

### FW version
* FW version: A0: 20230406.4.560
* FW version: B0: 20231017.7.129 (*new*)

### New APIs
* `adi_apollo_gpio_sync_pad_lvds_enable()`  Enable(1) or Disable(0) Apollo LVDS pads sync mode
* `adi_apollo_gpio_jesd_204b_configure()` Configure GPIO pins for JESD 204B 
* `adi_apollo_fsrc_ratio_set()` Configures an FSRC block given n/m parameters
* `adi_apollo_clk_mcs_man_reconfig_sync()` Manual reconfiguration trigger by SPI (similar to ext trig)
* `adi_apollo_jtx_force_invalids_set()` Force sending invalid samples as part of FSRC reconfig flow
* `apollo_mailbox_power_up_jtx()` Power up JTx *Internal API*
* `adi_api_utils_div_floor_u64()` 64-bit floor function w/ division
* `adi_api_utils_div_ceil_u64()` 64-bit ceil function w/ division

### New FPGA APIs
* `adi_fpga_apollo_hw_fsrc_tx_enable_set()` Tx HW FSRC enable set
* `adi_fpga_apollo_hw_fsrc_tx_enable_get()` Tx HW FSRC enable get
* `adi_fpga_apollo_hw_fsrc_ratio_set()` Set the HW FSRC ratio given the n/m ratio
* `adi_fpga_apollo_core_jtx_link_status_get()` Get FPGA JTx link up status

### Changed APIs
* N/A

### Example Code
* `fullchip_fsrc_sc1_ext_trig` 
  * Major refactor to support muliple profiles with per-chan configuration.
  * Support for FSRC, FDDC, FDUC reconfig w/ SC1 

### Bug fixes
* N/A

### Misc
* Added profile `id02_uc10` for JESD204B


### [0.1.43] (2023-11-13)
### Features
* ADC slice mode switching APIs
* Tx Rx FSRC w/ SC1, ext trigger
* CNCO modulus API

### FW version
* FW version: A0: 20230406.4.560
* FW version: B0: 20231017.4.107 (*new*)

### New APIs
* `adi_apollo_adc_slice_mode_switch_enable_set()` Enable or Disable the ADC Slice mode switching feature.
* `adi_apollo_adc_slice_mode_switch_prepare()` Perform setup sequence for the Slow ADC Slice Mode Switch for select ADCs.
* `adi_apollo_adc_slice_mode_switch_execute()` Execute Slow ADC Slice Mode Switch for select ADCs.
* `adi_apollo_adc_slice_mode_switch_restore()` Perform restore/cleanup sequence for the Slow ADC Slice Mode Switch for select ADCs.
* `adi_apollo_arm_err_codes_get()` Get FW error codes
* `adi_apollo_reconfig_trig_evt_cnt_get()` Gets the number of internal/external reconfig event count
* `adi_apollo_cnco_mod_set` Sets coarse NCO hop modulus params
* `adi_api_utils_ratio_decomposition` Convert a ratio M/N to (X+A/B)/(2^bit_size)


### Changed APIs
* `adi_apollo_clk_mcs_trig_phase_get()` Gets the trigger to sysref phase.

### Example Code
* `fullchip_fsrc_sc1_ext_trig` example refactored for enhanced support Rx and Tx.
* `tx_nco_mod` example for modulus NCO programming
* Run CC for B0 Si only to enable JTx links where needed (e.g. prbs, bmem examples)

### Bug fixes
* Fixed incorrect sample repeat setting in DFormat/JTx
* Fixed id00_uc06_F_sc1 profile where invalid_enable was set false for rx_fsrc. Correct value is true for FSRC.

### Misc
* JRx sample repeat configured from profile load



### [0.1.34] (2023-10-25)
### Features
* Rx FSRC w/ SC1
* TMU read API performance improvement
* Warm-Boot ADC calibration coeff dump and reload APIs. Supports random and sequential modes
* MemDump example code/utility for saving state of registers and cal data for debug

### FW version
* FW version: A0: 20230406.4.560
* FW version: B0: 20231017.0.91 (*new*)

### New APIs
* `adi_apollo_device_tmu_enable()` Enables all TMU channels for read back. Factored out of `adi_apollo_device_tmu_get()` for speed improvements
* `adi_apollo_fnco_main_pgm()` Configures FNCO main parameters
* `adi_apollo_fsrc_rate_set()` Sets main FSRC rate parameters. Used when changing FSRC ratios post startup.
* `adi_apollo_fsrc_mode_1x_enable_set()` Sets FSRC to 1x mode
* `adi_apollo_jrx_rm_fifo_reset()` Resets the JRx (DAC) rate match fifo
* `adi_apollo_utils_side_from_adc_select_get()` Utility for determining side from an ADC bit-wise selection
* `adi_apollo_utils_side_from_dac_select_get()` Utility for determining side from a DAC bit-wise selection

### Changed APIs
* `adi_apollo_cfg_adc_cal_data_set()` Added mode param for seq or random data
* `adi_apollo_cfg_adc_cal_data_get()` Added mode param for seq or random data
* `adi_apollo_cfg_adc_cal_data_len_get()` Added mode param for seq or random data
* `adi_apollo_rx_configure()` Adds JTx param for configuring FSRC params in DFORMAT block
* `adi_apollo_tx_configure()` Adds JRx param for configuring FSRC params in JRx JESD block
* `adi_apollo_tx_inv_sinc_configure()` Enable param type changed from bool to unit8_t

### Example Code
* `ads10_apollo_ex_caldata_dump_reload` example code/utility for dumping and reloading ADC cal coefficients, option for establishing desired temperature
* `ads10_apollo_ex_warmboot_nvm_dump` example code/utility for dumping ADC cal coefficients
* `ads10_apollo_ex_memdump` example code/utility for saving off state of Apollo registers and ADC calibration data for post analysis

### Bug fixes
* Fixed possible time sensitive FW startup error where primary core not identified in time for FW boot
* Fixed incorrect invsinc regmap address for A1/B0 when accessing individually
* Fixed DFORMAT invalid enable setting for FSRC use cases
* 
### ADS10 FPGA API
* New APIs to support SYSREF sequencer ext trigger
  * `adi_fpga_apollo_core_capture_start_x()`, `adi_fpga_apollo_core_capture_start_x2()` spits capture for FSRC reconfig and SYSREF aligned captures
  * `adi_fpga_apollo_core_sysref_seq_ext_trig_enable_set`, `adi_fpga_apollo_core_sysref_seq_ext_trig_enable_get()` Set/get SYSREF seq ext trigger enable

### Known Issues
* Some Rx FSRC ratios w/ SC1 may produce sample skew. This is being investigated and should be resolved in following release
* Tx FSRC w/ SC1 is nearing completion and should be resolved in following release

### Misc
* Profile `id00_uc08_20GLR_sc1_zeroIF_HWFSRC` was added as another test for Rx FSRC w/ SC1


### [0.1.19] (2023-08-14)
### Features
* New JTx serdes settings APIs
* Clock input power detector API
* Trigger pin(4) mapping to Rx/Tx API
* Makefile configurations for A0 and B0 devices
### FW version
* FW version: A0: 20230406.4.560 
* FW version: B0: 20230531.12.67 (new)
### A0 vs B0 Differences
* JTx and JRx link enables must occur before fist oneshot-sync or clock conditioning call.
* For B0, `JRx SYSREF Phase lock` is always 0 in subclass 0. Even if links are up. In subclass 1, the bit will be 0 or 1.
* PFILT responses may differ from A0. B0 fixes some PFILT issues and responses are more accurate.

### New APIs
* `adi_apollo_adc_init_cal` Execute an ADC init (foreground) calibration with config option
* `adi_apollo_clk_mcs_sync_trig_map` Maps a trigger pin to Rx and Tx digital
* `adi_apollo_clk_mcs_input_power_status_get` Get the status of the clock input power detector
* `adi_apollo_jtx_lane_pre_emphasis_set`, `adi_apollo_jtx_lane_pre_emphasis_get` Set/Get output Drive Swing level for jtx serdes lanes
* `adi_apollo_jtx_lane_drive_swing_set`,`adi_apollo_jtx_lane_drive_swing_get` Set/Get pre-Emphasis level for jtx serdes lanes
* `adi_apollo_jtx_lane_post_emphasis_set`, `adi_apollo_jtx_lane_post_emphasis_get` Set/Get post-Emphasis level for jtx serdes lanes
* `adi_apollo_dformat_configure` Configures Rx DFORMAT block from profile. Used in datapath configuration
### Example Code
* Removed explict setting of DFORMAT block in example code. No longer required, now configured from device profile
* Added debug_a0.mak and debug_b0.mak configurations. For example, to build example for B0: `make CONFIG=debug_b0`

### Bug fixes

### Misc
* Add return info to adi_apollo_clk_mcs.h declarations
* Remove unused serdes cal info from device context (adi_apollo_device_t)
* DFORMAT block now configured from device profile with RX data path
* Remove use of double constant type
* Remove extraneous printfs
### Known Issues
* Intermittent ADC init cal timeouts on CE
* DAC trim not setting A1 and B1 correctly
* Intermittent example code FSRC tone out may be corrupted, need rm fifo reset after FPGA bdir start
* GPIO pin #34 not functional


### [0.1.15] (2023-07-30)

### Features
* Example code B0 related updates
### FW version
FW version: A0: 20230406.4.560 
FW version: B0: 20230531.10.60

### New APIs
`adi_apollo_arm_profile_crc_valid_get` Get the CRC checksum validation status after device profile load
`adi_apollo_clk_mcs_sysref_internal_term_en_set` Set the SYSREF internal termination resistor enable

### [0.1.11] (2023-07-28)

### Features
* Support for B0 four additional GPIOs

### FW version
FW version: A0: 20230406.4.560 
FW version: B0: 20230531.10.60 (new)

### APIs Changed
`adi_apollo_clk_mcs_adc_path_offsets_set` removed

### Bug Fixes
* Fixed overwriting CC data during profile load for B0
* JTx PRBS for B0


### [0.1.6] (2023-07-20)
### Features

* Apollo version B0 initial support
* Code base supports A0 and B0 through build configurations
* ADS10 HSCI Master configuration for auto link-up 
* Apollo device APIs for setting HSCI manual and auto link-up configurations
* B0 specific device API for enabling Tx datapath scramblers and raptor data output
* Loopback0 API support

### FW version
FW version: A0: 20230406.4.560 (changed from 0.0.250)
FW version: B0: 20230531.8.54
### New APIs
* `adi_apollo_dac_scrambler_enable_set` Set enable/disable state of tx datapath scrambler to avoid substrate noise coupling
* `adi_apollo_dac_scrambler_enable_get` Get enable state of tx datapath scrambler
* `adi_apollo_hal_auto_linkup` HSCI autolink up
* `adi_apollo_lb0_rx_*` Loopback0 APIs
* `adi_apollo_utils_side_from_adc_select_get` Utility for obtaining device side from ADC selector
* `adi_apollo_utils_side_from_dac_select_get` Utility for obtaining device side from DAC selector

###  Changed APIs
* `adi_apollo_device_api_revision_get` major, minor, patch params now uint16_t (was uint8_t)

### Example Code
* Choose interleaved or separate IQ files when writing capture data
* Added loopback0 example code
* `adi_ads10_apollo_ex_fpga_capture` Now has single or interleaved option when writing capture data to files

### Bug Fixes
* Incorrect regmap base mapping for HSDOUT and raptor based in DAC index.


### [0.0.250] (2023-07-05)
### Features
  * Example code vector loading from file, Np12 support

### FW version
* 20230406.4.560 (changed from v0.0.248)

### Example Code
* `tx_vec_file.c` example to ads10_apollo_ex_main
* `adi_ads10_apollo_ex_vec_load_from_file()` Iteratively load vector from files (large file loading)
* Support for passing custom cmd line options to individual example modules

### Bug Fixes
    * Fixed compile warnings in hmc7044, adi6331 and adl6332 devices

### [0.0.248] (2023-06-30)
### Features
  * Example code for FSRC w/ SC1, HW SYSREF alignment (20Gsps, 10.3125Gbps)
  * ADS10 platform APIs added

### FW version
* 20230406.3.551 (not change from v0.0.246)

### New APIs
  * `adi_apollo_clk_mcs_sysref_count_set` to specify num SYSREFs to ignore before alignment
  * `adi_apollo_jrx_link_stat_e` enum to replace magic numbers

### Example Code
  * `fullchip_fsrc_sc1_ext_trig` FSRC w/ SC1, simultaneous Rx/Tx functional example
  * `tx_jesd_fsrc_sc1_ext_trig` FSRC w/ SC1, Tx only functional example
  * `adi_ads10_apollo_ex_reg_dump` for dumping Apollo regs to file
  * New use-case `id00_uc06_F_sc1` 20/10.3125, 15625/12288, SC1

### Known Issues
  * New use case `id00_uc06_F_sc1` has incorrect checksum
  * FSRC w/ SC1 examples contain some direct bitfield writes. These will be replaced with API calls in future release.
  * On the ADS10 FPGA platform:
    * FPGA image can occasionally get stuck in bdir state at startup
    * Must reload image or power cycle prior to show DL


### [0.0.246] (2023-06-16)
### Features
  * Device profile #8
  * TyE encrypted FW loading
  * Dual Clock APIs
  * Np12 JESD
  * Example code for id00_uc13 profile (Np12, 4G IBW)
  * Example code foe id01_uc05 profile (Np12, 14/28 clocking)
  * Add version and subclass to Jrx/Jtx inspect
  * Example code profiles are checksum accurate

### FW version
    * 20230406.3.551
    * Device profile 8
    * Serdes Rx tracking calibration period from 60s to 10s 

### Apollo APIs
    * Device Profile
      * `adi_apollo_top_t` device profile struct to support 8T8R devices
    * ARM
      * `adi_apollo_arm_tye_firmware_buf_write()`
      * `adi_apollo_arm_tye_bypassed_get()`
      * `adi_apollo_arm_tye_bypassed_get()`
    * TXMISC
      *`adi_apollo_txmisc_inspect()`

### APIs Changed
    * MCS 
      * `adi_apollo_clk_mcs_sync_hw_align_set()`
      * `adi_apollo_clk_mcs_sync_hw_align_get()`
      * `adi_apollo_clk_mcs_sync_only_set()`
      * `adi_apollo_clk_mcs_sync_only_get()`
      * `adi_apollo_clk_mcs_trig_reset_dsp_enable()`
      * `adi_apollo_clk_mcs_trig_reset_serdes_enable()`
      * `adi_apollo_clk_mcs_trig_reset_disable()`
      * `adi_apollo_clk_mcs_trig_phase_get()`
      * `adi_apollo_clk_mcs_dyn_sync_sequence_run()`

### Apollo Example Code
    * `ads10_apollo_ex_main` main example module
      * `rx_adc_deep` for large rx capture size
      * `tx_vec_deep` for large tx vector size
      * `dp_load` loads a profile and exits
    * `ads10_apollo_ex_fullchip` simultaneous Rx/Tx
      * add use cases id01_uc05 (Np12 14/28) and id00_uc13 (Np12, 4G IBW)

### Bug Fixes
    * adi_apollo_serdes_prbs_generator_enable: Calculate offset based on lane.

### Known Issues
    * tx_jesd_cfir may not show 1475 tone

### [0.0.216] (2023-03-21)
### Features
  * Adds MCS subclass 1 APIs and example code. HW sync (one-shot-sync)
  * Artemis API updates
  * ADS10 new Apollo FPGA APIs for SYSREF select, trig out, JESD control

  * Apollo APIs
    * MCS
      * `adi_apollo_clk_mcs_sysref_en_set()`
      * `adi_apollo_clk_mcs_sysref_en_get()`
      * `adi_apollo_clk_mcs_internal_sysref_per_set()`
      * `adi_apollo_clk_mcs_internal_sysref_per_get()`
      * `adi_apollo_clk_mcs_subclass_set()`
      * `adi_apollo_clk_mcs_subclass_get()`
      * `adi_apollo_clk_mcs_sysref_phase_get()`
    * JRx
        * `adi_apollo_jrx_phase_adjust_set()`
        * `adi_apollo_jrx_lane_xbar_set()`
        * `adi_apollo_jrx_lanes_xbar_set()`
        * `adi_apollo_jrx_phase_diff_get()`
        * `adi_apollo_jrx_subclass_set()`
    * JTx
       * `adi_apollo_jtx_phase_adjust_set()`
       * `adi_apollo_jtx_lane_xbar_set()`
       * `adi_apollo_jtx_lanes_xbar_set()`
       * `adi_apollo_jtx_subclass_set()`
       * `adi_apollo_jtx_lane_force_pd_set()`
    * GPIO
      * `adi_apollo_gpio_cmos_st_enable()`
      * `adi_apollo_gpio_syncin_pad_termination_enable()`

  * Apollo Example Code
    * `ads10_apollo_ex_main` main example module
      * `rx_mcs_dl` example for Rx subclass 1. Sends signal to ADC through DAC.
      * `tx_mcs_dl` example for Tx subclass 1. Create edge pattern and drive through DAC.
    * `ads10_apollo_ex_common`
      * `adi_ads10_apollo_ex_vec_create_usr_transmit_buffer()` allows passing user defined vector gen func.

  * ADS10 FPGA
    * `adi_fpga_apollo_core_sys_indicator_set()`
    * `adi_fpga_apollo_core_sys_indicator_get()`
    * `adi_fpga_apollo_core_sysref_src_set()`
    * `adi_fpga_apollo_core_sysref_src_get()`
    * `adi_fpga_apollo_core_transmit_skip_data_en()`

  * ADF4382
    * `adi_adf4382_ldctrl_status_get()`
    * `adi_adf4382_ldctrl_lock_wait()`

  * Misc Changes
    * Added more fields to JRx/JTx inspect APIs

### Bug Fixes
  * Correct invalid loop count in `adi_fpga_apollo_core_jesd_rx_config()`

### Known Issues


### [0.0.194] (2023-01-25)
### Features
  * New APIs
    * DDC gain
      * `adi_apollo_cddc_gain_enable_set()`, `adi_apollo_cddc_gain_enable_get()`
      * `adi_apollo_fddc_gain_enable_set()`, `adi_apollo_fddc_gain_enable_get()`
    * DAC DDR clock trim
      * `adi_apollo_dac_clk_trim_set()`, `adi_apollo_dac_clk_trim_get()`
    * Warmboot cal data
      * `adi_apollo_cfg_adc_cal_data_get()` ...
      * `adi_apollo_cfg_dac_cal_data_get()` ...
      * `adi_apollo_cfg_serdes_rx_cal_data_get()` ...
      * `adi_apollo_cfg_serdes_tx_cal_data_get()` ...

  * Example Code
    * `ads10_apollo_ex_main` main example module
      * `tx_dac_trim.c` DAC clk trim for perf tunning

  * Misc Changes
    * Fixes for linux compliance
    * adi_api_utils_mod_128 for \_KERNEL\_ compile
    * adf4382 Use R_DIV and EN_RDBLR instead of PFD freq. when configuring RFOUT
    * ltc2977 and ltm4681 power module APIs

### Bug Fixes
  * block select bug in

### Known Issues

### [0.0.172] (2022-12-22)
### Features
  * FSRC example code
    * Added `fullchip_fsrc_dr` simultaneous Rx/Tx FSRC and FDUC dynamic reconfig trig via SPI
    * Added `tx_jesd_fsrc_dr` Tx JESD FSRC and FDUC dynamic reconfig trig via SPI

  * New APIs
    * `adi_apollo_fduc_subdp_gain_enable()`

  * Misc Changes
    * adi_apollo_dformat_pgm_t new fields added to support FSRC
    * Factored out some code duplication in example code, added fpga and vector files
    * Properly format FPGA version date code w/ 4 digits

### Bug Fixes
  * Fixed fnco indexing selection:
    * adi_apollo_fnco_profile_load(), adi_apollo_fnco_hop_pgm()
    * adi_apollo_fnco_chan_pgm(), adi_apollo_fnco_pgm(), adi_apollo_fnco_inspect()
  * Fixed selection mask in:
    * adi_apollo_jrx_fec_errors(), adi_apollo_jrx_j204b_lane_status_get(), adi_apollo_jrx_j204b_lane_error_get()

### Known Issues
  * Few compile warnings in example code

### [0.0.169] (2022-12-01)

### Features
  * Firmware updated to 20221026.1.503
  * Rx FSRC/FDDC dynamic reconfig via SPI (see examples/ads10_apollo_ex_main/rx_adc_fsrc_dr.c)
  * ADC Nyquist zone set
  * ADC background cal freeze/unfreeze
  * Serdes cal execution moved from example code into API
  * to_str utility APIs added to examples

  * New APIs
    * `adi_apollo_adc_nyquist_zone_set()`
    * `adi_apollo_adc_nyquist_zone_get()`
    * `adi_apollo_adc_bgcal_state_get()`
    * `adi_apollo_adc_bgcal_freeze()`
    * `adi_apollo_adc_bgcal_unfreeze()`
    * `adi_apollo_serdes_jrx_cal()`

  * Example code
    * `ads10_apollo_ex_main` main example module
        * `rx_adc_fsrc_dr.c` demonstrates Rx fsrc/fddc dynamic reconfig via SPI
        * `rx_adc_cc.c` run clock conditioning in loop simulating temperature change
        * `rx_adc_nz.c` demonstrates use of setting Rx Nyquist zone to reduce spurs

  * Changed APIs
    * `adi_apollo_fddc_ratio_e` enum replaces `adi_apollo_fine_ddc_dcm_e` for device profile alignment
    * `adi_apollo_trigts_cnco_trig_sel_mux_set()` renamed to `adi_apollo_trigts_cdrc_trig_sel_mux_set()` for clarity
    * `adi_apollo_trigts_fnco_trig_sel_mux_set()` renamed to `adi_apollo_trigts_fdrc_trig_sel_mux_set()` for clarity
    * `adi_apollo_adc_cal()` selection parameter changed to bit-or to align with API convention

### [0.0.151] (2022-10-29)
  * Core0 FW loading
  * Firmware updated to 20221026.0.495
  * Mailbox wrappers updated to include TyE

### [0.0.147] (2022-10-17)
### Features
* Firmware version: 20220824.4.471
* Device Profile version #7 (same as previous 0.0.134 release)
* Rx/Tx CFIR APIs and example code
* Rx/Tx FSRC APIs and example code.
* TX HW transport FPGA support
* Clock conditioning default limits modified
* ADC cal initializes from NVM cal coefficients
* New APIs
    * `adi_apollo_pfilt_inspect()`
    * `adi_apollo_pfilt_mode_enable_set()`
    * Functions for converting bitfield values into numeric values
        * `adi_apollo_cduc_interp_bf_to_val()`
        * `adi_apollo_fduc_interp_bf_to_val()`
        * `adi_apollo_cddc_dcm_bf_to_val()`
        * `adi_apollo_fddc_dcm_bf_to_val()`

* Example Code
    * `ads10_apollo_ex_main` new example module
        * `rx_adc_fsrc` RX FSRC block driven by ADC samples (to Apollo JTx)
        * `rx_adc_pfilt` RX PFILT block driven by ADC samples (to Apollo JTx)
        * `rx_bmem_cfir` RX CFIR block driven by BMEM-AWG mode (to Apollo JTx)
        * `rx_bmem_fsrc` RX FSRC block driven by BMEM-AWG mode (to Apollo JTx)
        * `rx_bmem_pfilt` RX PFILT block driven by BMEM-AWG mode (to Apollo JTx)
        * `tx_jesd_cfir` TX CFIR block driven by JESD link (from Apollo JRx)
        * `tx_jesd_fsrc` TX FSRC block driven by JESD link (from Apollo JRx)
    * `ads10_apollo_ex_rx_adc`
        * Replace the 20.625 profile id00_uc08 with id00_uc08_f.
    * Bug fixes
        * Fix time sensitive error check in jrx_prbs_test.


* ADS10 FPGA
    * FPGA Core APIs
    * `adi_fpga_apollo_core_jtx_link_cnt_get()`     Get number of JTx links supported per side
    * `adi_fpga_apollo_core_jrx_link_cnt_get()`     Get number of JRx links supported per side
    * `adi_fpga_apollo_core_supports_hw_tl_get()`   Indicates if FPGA supports JESD transport layer in HW
    * `adi_fpga_apollo_core_supports_hw_fsrc_get()` Indicates if FPGA supports HW FSRC
    * FPGA FSRC APIs (see fpga user guide for details)
        * `adi_fpga_apollo_hw_fsrc_sysref_counter_config()`
        * `adi_fpga_apollo_hw_fsrc_tx_sequencer_config()`
        * `adi_fpga_apollo_hw_fsrc_bidir_start_config()`
        * `adi_fpga_apollo_hw_fsrc_start_config()`
        * `adi_fpga_apollo_hw_fsrc_tx_pgm()`
        * `adi_fpga_apollo_hw_fsrc_rx_enable_get()`
        * `adi_fpga_apollo_hw_fsrc_rx_enable_set()`



### [0.0.134] (2022-09-11)
### Features
* Firmware version: 20220811.0.438
* Update to device profile version #7
* Adds drc_phase_dither_en, drc_amp_dither_en to cnco and fnco inspect
* New APIs
    * `adi_apollo_pfilt_inspect()`
    * `adi_apollo_pfilt_mode_enable_set()`
* Example Code
    * Adds filter coeff files for PFILT
    * Adds examples for loading from file, enable/bypass and setting active filter bank (see tx_nco_pfilt.c)
    * `adi_ads10_ex_pfilt_coeff_file_load()` load filter coeffs from file


### [0.0.127] (2022-09-07)
### Features
* Firmware version: 20220811.0.438
* Update to device profile version #7
* FPGA GPIO APIs and example code
* PFILT example code and some consolidation
* TMU api mailbox wrapper
* Inspect API for FSRC
* FPGA function to load image programmatically
* New APIs
    * `adi_apollo_bmem_awg_data16_config()`
    * `adi_apollo_cddc_dcm_bf_to_val()`
    * `adi_apollo_fddc_dcm_bf_to_val()`
    * `adi_apollo_device_tmu_get()`
    * `adi_apollo_fsrc_inspect()`
    * `adi_apollo_pfilt_coeff_transfer()`
* Deprecated
    * `adi_apollo_pfilt_transfer_coeff_regmode()`
* Changes
    * `ads10_hal` misc print statements and implicit delays removed
    * `ads10_hal` unused spi poll read removed
* Example Code
    * `ads10_apollo_ex_main` added
        * `rx_adc_pfilt, rx_bmem_fsrc, rx_bmem_pfilt, tx_nco, tx_nco_pfilt`
    * `ads10_apollo_ex_gpio` added
* ADS10 FPGA
    * `adi_fpga_apollo_config_load_image()`
    * `adi_fpga_apollo_core_rx_keep_link_mask_set()`
    * `adi_fpga_apollo_core_tx_keep_link_mask_set()`
    * `adi_fpga_apollo_core_fifo_ready_set()`
    * `adi_fpga_apollo_core_fifo_ready_get()`
    * `adi_fpga_apollo_gpio_output_set`
    * `adi_fpga_apollo_gpio_input_get`

### Bug Fixes
    * PFILT west side index to bit-or selector macro

### [0.0.114] (2022-08-10)

### Features

* SPO eye scan example improvements
* Optional non-streaming SPI transactions. Enable/disable in the SPI descriptor.
* FW and Device Profile Types:
    * FW version: 20220719.1.421
    * Device profile: 6.0.1
* New APIs
    * `adi_apollo_adc_sync_path_delay_set()`
* Changes
    * In struct `adi_apollo_hal_txn_config_t` the `stream_len` field changed from `uint16_t` to `uint32_t`


### [0.0.98] (2022-07-19)

### Features

* SW FSRC valid and invalid sample functions added to fpga api
* Add SW FSRC profiles to examples ex_rx_bmem and ex_tx_jesd
* Add fullchip example featuring simultaneous rx/tx
* SPI transaction reduction using spi streaming and fifo modes
* License file changed to SLA
* New APIs
    * `adi_apollo_cduc_inspect()`
    * `adi_apollo_fduc_inspect()`
    * `adi_apollo_cnco_inspect()`
    * `adi_apollo_fnco_inspect()`
    * `adi_apollo_cddc_inspect()`
    * `adi_apollo_fddc_inspect()`



### [0.0.75] (2022-06-14)

### Features

* ADC clock conditioning for maximizing ADC performance
* Device profile type 6.0
* Example code suite includes Rx and Tx data path w/ 20.625Gbps JESD lane rate
* Add API's for SW FSRC invalid sample remove, FPGA api (preliminary)
* New APIs
    * `adi_apollo_adc_cal()` for running ADC calibrations
    * `adi_apollo_clk_mcs_dyn_sync_sequence_run()` for sequencing clock enables
    * `adi_apollo_cnco_pow_set()` for setting CNCO phase offset word on the fly
    * `adi_apollo_fnco_pow_set()` for setting FNCO phase offset word on the fly
    * `adi_apollo_fnco_main_phase_offset_set()` for setting FNCO phase offset on the fly (non-hopping mode)
    * `adi_apollo_sysclk_cond_cal()` for executing clock conditioning cal. See `adi_apollo_sysclk_cond.h` for more CC APIs.
* Firmware version 202266.0.398

### Changes
* Removed `adi_apollo_cfg_clk_conditioning_defaults_set()` Now part of clock conditioning APIs
* Renamed `adi_apollo_rx_tmode_type_e` ti `adi_apollo_rx_tmode_type_sel_e`
* Rename `ADI_APOLLO_FINE_MXR_VAR_IF_MODE` to `ADI_APOLLO_MXR_VAR_IF_MODE` See `adi_apollo_nco_mixer_mode_e` for more name changes
* Reanme elements in `adi_apollo_rx_tmode_res_e` and `adi_apollo_rx_tmode_sel_e`. Moved to device profile

### Bug Fixes
* Fixe error in `adi_apollo_coarse_duc_dcm_e` for `ADI_APOLLO_CDUC_INTERP_12`

### [0.0.63] (2022-06-02)

### Features
* Add boot status to startup sequence in example code
* Add settling delay after one-shot sync in PRBS test to reduce start errors
* New APIs
    * `adi_apollo_jrx_link_inspect()` for obtaining JRx link params from device
    * `adi_apollo_jtx_link_inspect()` for obtaining JTx link params from device
    * `adi_apollo_invsinc_inspect()` for obtaining Inverse Sinc enables from device
* Firmware version 2022427.1.8

### Bug Fixes
    * `ads10_apollo_ex_rx_bmem example` with Fclk 10Gsps uc06a

### [0.0.59] (2022-05-23)

### Features
* New ADC example code
    * ADC->Rx DP->JTx->FPGA JRx->Cap mem
    * Use case 6. Fclk=20Gsps, Jesd=10.3125Gbps
    * Writes capture data files as interleaved I/Q samples per channel
* JRx horizontal eye-scan APIs.
* New example code features
    * TMU (temperature sensors)
    * INVSINC
    * Eye-Scan added to PRBS example
* Firmware version 2022427.1.8
* New APIs
    * `adi_apollo_clk_mcs_adc_path_offsets_set()` for setting clock path offsets
    * `adi_apollo_adc_tlines_offset_set()` for setting adc tline offsets
    * `adi_apollo_sysclk_cond_defaults_set()` for setting initial clock conditioning coeffs
    * `adi_apollo_fnco_main_phase_inc_set()` setting active fnco tuning word
    * JRx eye scan
        * `adi_apollo_serdes_jrx_clock_strobe()`
        * `adi_apollo_serdes_jrx_set_flash_mask()`
        * `adi_ads10_serdes_jrx_spo_set_dir()`
        * `adi_apollo_serdes_jrx_prbs_clear_error()`


### Bug Fixes
* BMEM end address setting in example code ads10_apollo_ex_rx_bmem
* BMEM awg tone 12-bits, left justified in 16-bit word per design

### Known Issues
 * ADC init cals may fail depending on device
 * rx_bmem capture example validated at 20Gsps only (locating issue @10Gsps))


### [0.0.43] (2022-04-22)


### Features

* Example code for JESD JRx link and Tx data path
    * See folder: `examples/ads10_apollo_ex_tx_jesd`

* New APIs
    * `adi_apollo_clk_mcs_adc_path_offsets_set()`

### Bug Fixes

* Fix incorrect results from `adi_apollo_jrx_link_status_get()`



### [0.0.31] (2022-04-06)


### Features

* JESD 27.5Gbps PRBS testing
* New APIs
    * `adi_apollo_bmem_sram_get()`

### Changes
* `adi_apollo_tmode_config_set()` now takes converter selection mask param
* `adi_apollo_tx_fduc_pgm()` renamed to `adi_apollo_tx_fduc_configure()`


### Bug Fixes
* 16-bit BMEM input tone reduced to 12-bit to fix sample saturation




### [0.0.25] (2022-04-01)


### Features

* Jrx SERDES PRBS at 20.625Gbps
* Examples folder refactoring to reduce code duplication
* Add mask field to SPI txn config
* Bit status poll read integration, mailbox busy wait
* Clock conditioning mailbox updated with cmd param
* New APIs
    * `adi_apollo_device_uuid_get()`

### Bug Fixes
* Use paged writes when rmw is enabled
* ADS10 lane mapping indexing missing last lane
* ICD adapter



### [0.0.24] (2022-03-20)


### Features
* ADS10 test app `ads10_apollo_rx` added 20Gsps uc6
* ADS10 test app `ads10_apollo_rx` configure FPGA JTx and JRx from device profile
* ADS10 test app `ads10_apollo_rx` capture through MicroZed
* Added FW version mailbox command
* New APIs
    * `adi_apollo_cddc_inspect()` and `adi_apollo_fddc_inspect()`

### Changes
* `adi_apollo_rxmisc_pgm()` now takes a `side` sel
* `adi_apollo_serdes_prbs_generator_enable()` now takes lane array argument

### Bug Fixes
* FNCO/CNCO DC test mode value set during device profile loading for Rx and Tx
* ADS10 HAL spi timing issues

### Known issues
* ADS10 test app `ads10_apollo_rx`: first capture after power-on will occasionally contain all FFs


### [0.0.23] (2022-03-11)

### Bug Fixes
* erpc/evalclient comm errors with ads10
* use overloaded function instead of optional parameter


### [0.0.22] (2022-03-10)

### Features
* Rx datapath example app using bmem with uc6a (`examples/ads10_apollo_rx`)
* New APIs
    * `adi_apollo_bmem_awg_normal_sram_set()`

### Changes
* `adi_apollo_rxmisc_pgm()` now takes a `side` sel

### Bug Fixes
* Fixed `adi_apollo_rxmux_xbar1_set()`
* Fixed `ADI_APOLLO_SIDE_IDX2B(side)` macro
* Rx datapath I/Q clock enables now configured by API during device profile processing

### Known issues
* Configuring FSRC block from top level data path config, `adi_apollo_rx_configure()`, causes all zero samples on data path. Disabled for this release
* Hard coded `debug_clkoff_n=0xff` and `debug_drc_clkoff_n=true` in adi_apollo_rx_cddc_configure() to enable datapath. Should be set in uc6 profiles
* The Rx dformat block is not configured from device profile and must temporarily be handled by the application. The total and link delimitations must be set or links won't come up. Refer to `adi_apollo_dformat_pgm()`



### [0.0.21] (2022-03-03)


### Features
* mzcomms_app spi clk to 2.5MHZ
* update ads10_apollo app fw and profile from ptk88


### [0.0.20] (2022-02-28)


### Features
* CFIR Rx and Tx device profile integration
* Add API's for spectrum sniffer
* Addition of FSRC cfg apis
* Add BMEM interface to server and eval client
* New APIs
    * adi_apollo_cfir_coeff_pgm()
    * adi_apollo_rx_cfir_configure()
    * adi_apollo_rx_fsrc_configure()
    * adi_apollo_tx_cfir_configure()
    * adi_apollo_tx_fsrc_configure()
    * adi_apollo_sniffer_pgm()
    * adi_apollo_sniffer_fft_data_get()

### Changes
* Change MZCOMMS spi clk to 2.5MHz in apollo server
* Calibration set APIs using stream writes

### Bug Fixes
* Fix indexing and cal set apis
* Fix undersized FNCO select param (uint8_t to uint16_t_)


### [0.0.19] (2022-02-22)


### Features

* mzcomms platform integrated with server
* rxmux crossbar1 block API refactor
* update ads10 example app to profile-types 2.1.0

### Changes
* page register base address mask
* reset control pin delay set to 1us

### Bug Fixes
* page base address write as 32-bits


### [0.0.18] (2022-02-11)

### Features
* integrate changes for profile types 2.1.0
* addition of BMEM awg normal mode apis
* API to load profile using byte array, `adi_apollo_arm_profile_buf_write()`
* cnco and fnco init apis take main and fractional nco words
* mailbox update to new fw
* new APIs
    * `adi_apollo_fnco_active_profile_set()`
    * `adi_apollo_fnco_profile_sel_mode_set()`
    * `adi_apollo_cnco_active_profile_set()`
    * `adi_apollo_cnco_profile_sel_mode_set()`

### Changes
* `adi_apollo_fnco_chan_pgm()` and `adi_apollo_cnco_chan_pgm()` now take a `profile_num` param
* `adi_apollo_fnco_ftw_set()` and `adi_apollo_cnco_ftw_set()` now take `profile_num` and `active_en` params

### Bug Fixes
*

### Known Issues
* `examples/ads10_apollo` demo app won't compile. Requires profiles based on version 2.1.0


---
### [0.0.17] (2022-02-07)

### Features

* Apollo JRx PRBS testing api
  * adi_apollo_serdes_jrx_prbs_checker_enable()
  * adi_apollo_serdes_jrx_prbs_checker_status()
* Addition of BMEM normal mode capture apis

### Bug Fixes

* Add missing invsinc interface to erpc server

---
### [0.0.16] (2022-02-01)


### Features

* JTx PRBS test with ADS10 example application. See examples/ads10_apollo/main.c
* JTx PRBS test added to FPGA API
* add modsw1 support for 8t8r devices
* add data path reset apis
* map fpga to apollo phy lanes

### Changes
* remove internal folder from apollo_api
* rename adi_apollo_txmux_invsinc_enable() to adi_apollo_invsinc_enable(), new interface

### Bug Fixes

* add missing rxmux and trigts to evalclient
* api added to return profile type struct version
* null pointer issue with mailbox command

---
### [0.0.15] (2022-01-21)


### Features

* 10GHz_testmode NCO app. See examples/ads10_apollo/main.c
* ads10 fpga api hal integration
* ads10_apollo makefile adds fpga api
* re-init device spi and hsci after reset
* updates for refactored adi_apollo_hw_open() now takes reset option

### Changes
* rename adi_apollo_hal_config_t to adi_fpga_apollo_hal_config_t
* factored out apollo types from adi_cms_common.h

### Bug Fixes
* changes in invsinc api interface and 4t4r case
