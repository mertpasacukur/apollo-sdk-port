/*!
 * \brief     configureVUP - Application to load a platform FPGA image
 *
 * This example will program the platform FPGA image with the file specified
 * on the command line.
 *
 *
 * usage: configureVUP <image_file>
 *    or: configureVUP --help
 *    or: configureVUP no args
 *
 * <image_file>    fpga image file, absolute path if starts with '/'
 * --help          print this usage text
 * no args         load default fpga image
 *
 *  
 * The image_file is expected to exist on the platform.
 * 
 * If the image_file starts with '/', then will assume an absolute path to the file on target. 
 * Otherwise, will look in the target's default FPGA image folder location for file.
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "adi_utils.h"
#include "adi_cms_api_common.h"
#include "adi_ads10_apollo_ex.h"
#include "adi_fpga_image_loader.h"
#include "adi_fpga_image_loader_ads10.h"
#include "adi_fpga_apollo_core.h"

#define LOG_FILENAME "configureVUP.log"

static int32_t initialize(adi_fpga_apollo_device_t *fpga_device);
static void print_usage(char *pgm_name);

int main(int argc, char *argv[])
{
    int err = 0;
    char *fpga_image_fname;
    uint8_t is_ads10_open = 0;
    uint8_t fpga_image_fname_path[MAX_PATH_LEN];
    uint32_t fpga_image_dir_len;
    uint32_t fpga_image_ver;
    adi_fpga_image_loader_t *img_ldr;
    adi_fpga_apollo_device_t fpga_device = {{0}};   /* FPGA device object */

#ifndef ADS10
    printf("Only ADS10 platform is supported.");
    return API_CMS_ERROR_NOT_SUPPORTED;
#endif

    /* Validate the number of parameters */
    if (argc == 1) {
        fpga_image_fname = DEFAULT_FPGA_IMAGE_FILE;
    } else if (argc == 2) {
        if (!strcmp(argv[1], "--help")) {
            print_usage(argv[0]);
            err = API_CMS_ERROR_OK;
            goto end;
        } else {
            fpga_image_fname = argv[1];
        }
    } else {
        printf("\nerror: invalid number of parameters\n");
        print_usage(argv[0]);
        err = API_CMS_ERROR_INVALID_PARAM;
        goto end;
    }
    
    /* Create and FPGA image loader for ADS10 platform */
    img_ldr = adi_fpga_image_loader_ads10_create(FPGA_IMAGE_FILE_PATH);
    if (img_ldr == NULL) {
        printf("Error creating ADS10 FPGA image loader\n");
        goto end;
    }

    /* Get the default directory for storing FPGA images on target */
    if (fpga_image_fname[0] == '/') {
        /* Absolute path */
        strcpy((char *) fpga_image_fname_path, fpga_image_fname);
    } else {
        /* File is located in the default folder assigned by the target */
        err = adi_fpga_image_loader_dir_get(img_ldr, fpga_image_fname_path, &fpga_image_dir_len);
        ADI_CMS_ERROR_GOTO(err, err_free);
        
        /* Create full target path of FPGA image file */
        strcat(strcat((char *) fpga_image_fname_path, "/"), fpga_image_fname);
    }

    /* Load the FPGA image on the Platform */
    err = adi_fpga_image_loader_configure(img_ldr, fpga_image_fname_path, strlen((char *)fpga_image_fname_path));
    ADI_CMS_ERROR_GOTO(err, err_free);

    printf("FPGA image loaded successfully! (%s)\n", (char *)fpga_image_fname_path);

    /* Initialize the ADS10 FPGA Device
     * Note: fpga_device isn't necessry for FPGA image loading.
     *       It's used for reading back image version.
     */
    err = initialize(&fpga_device);
    if (err != API_CMS_ERROR_OK)
    {
        printf("Error initializing platform\n");
        goto end;
    }

    is_ads10_open = 1;
    
    /* Read back the FPGA image version */
    err = adi_fpga_apollo_core_image_ver_get(&fpga_device, &fpga_image_ver);
    ADI_CMS_ERROR_GOTO(err, err_free);

    printf("FPGA Image Ver: 0x%08x\n", fpga_image_ver);

    /* Read back the FPGA platform info */
    err = adi_fpga_apollo_core_platform_identify(&fpga_device);
    ADI_CMS_ERROR_GOTO(err, err_free);

    printf("FPGA Design: %d\n", fpga_device.state_info.design_id);

    err_free:
        free(img_ldr);
        
    end:
        if (is_ads10_open) {
            ads10_hw_close();
        }

    printf("Goodbye. Return err code: %d\n", err);

    return err;
}


static int32_t initialize(adi_fpga_apollo_device_t *fpga_device)
{
    int32_t err = API_CMS_ERROR_OK;

    /* Open ADS10 platform - This will init the FPGA SPI Master */
    if (err = ads10_hw_open(LOG_FILENAME), err != API_CMS_ERROR_OK) {
        printf("ADS10 failed to open. Exiting...\n");
        return API_CMS_ERROR_HW_OPEN;
    }

    /* Create an ADS10 platform instance. This provides FPGA register access. */
    adi_fpga_apollo_hal_config_t *ads10_platform = ads10_apollo_hal_instance();

    /* Wire up the platform specific FPGA HAL for the FPGA API */
    err = adi_ads10_apollo_ex_configure_fpga_hal(fpga_device, ads10_platform);

    return err;
}

static void print_usage(char *pgm_name)
{
    printf("\n");
    printf("usage: %s <image_file>\n", (strrchr(pgm_name, '/') + 1));
    printf("   or: %s --help\n", (strrchr(pgm_name, '/') + 1));
    printf("   or: %s no args\n\n", (strrchr(pgm_name, '/') + 1));

    printf("<image_file>    fpga image file, absolute path if starts with '/'\n");
    printf("--help          print this usage text\n");
    printf("no args         load default fpga image\n\n");
}
