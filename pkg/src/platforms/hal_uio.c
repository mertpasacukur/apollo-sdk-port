#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     HAL: discover, setup and mmap the UIO devices from the DeviceTree
 *
 * @copyright copyright(c) 2025 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM_ADS10__
 * @{
 */

/*============= I N C L U D E S ============*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dirent.h>
#include <string.h>
#include "ads10_hal.h"
#include "hal_uio.h"


/*============= D E F I N E S ==============*/
#define UIO_DBG     0

#ifdef _WIN32
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#if UIO_DBG
    #define UIO_LOG_DBG(...) \
        uio_log(ADI_CMS_LOG_MSG, __FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
    #define UIO_LOG_DBG(...)
#endif
#define UIO_LOG_MSG(...) \
    uio_log(ADI_CMS_LOG_MSG, __FILENAME__, __LINE__, __func__, __VA_ARGS__)
#define UIO_LOG_ERR(...) \
    uio_log(ADI_CMS_LOG_ERR, __FILENAME__, __LINE__, __func__, __VA_ARGS__)
#define UIO_LOG_WARN(...) \
    uio_log(ADI_CMS_LOG_WARN, __FILENAME__, __LINE__, __func__, __VA_ARGS__)

#define UIO_LOG_MSG_SIZE_MAX 512

/*============= S T R U C T S ==============*/

/**
 * @brief Struct to sore the UIO device information
 */
typedef struct uio_device {
    char        *uio_name;
    void        *mem_reg;
    uint32_t    mem_size;
    uint32_t    uio_number;
} uio_device_t;

/*============= D A T A ====================*/ 

static uint32_t g_device_count = 0;
static uio_device_t *devices = NULL;


/*=========== S T A T I C  F U N C T I O N S  D E C L A R A T I O N ==========*/

/**
 * @brief Logging function used by hal_uio module. It relies on ads10_log_write() API.
 */
static void uio_log(int32_t log_type, const char *file, uint32_t line,
                    const char *fcn, const char *fmt, ...);


/*=========== S T A T I C  F U N C T I O N  D E F I N I T I O N S ============*/

static void uio_log(int32_t log_type, const char *file, uint32_t line,
                    const char *fcn, const char *fmt, ...)
{
    char buffer[UIO_LOG_MSG_SIZE_MAX];
    va_list args;
    va_start (args, fmt);
    snprintf(buffer, UIO_LOG_MSG_SIZE_MAX, "%s[%d]:%s(): ", file, line, fcn);
    size_t offset = strlen(buffer);
    vsnprintf(buffer + offset, UIO_LOG_MSG_SIZE_MAX, fmt, args);
    ads10_log_write(NULL, log_type, buffer, args);
    va_end(args);
}


/**
 * @brief   Discovers UIO devices present and accessible.
 *          Maps discovered UIOs (e.g. /dev/uioX ) to virtual address space.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 * @return      <0                  Failed. @ref adi_cms_error_e for details.
 */
int32_t adi_uio_devices_init(void)
{
    struct dirent *entry;
    DIR *dp;
    int32_t count = 0;
    size_t size = 256;
    char path[512];
    FILE *file;
    int32_t err = API_CMS_ERROR_HW_OPEN;


    UIO_LOG_DBG("Opening \"/sys/class/uio\" folder...");
    dp = opendir("/sys/class/uio");
    if (dp == NULL) {
        UIO_LOG_ERR("opendir() failed: %s", strerror(errno));
        goto end;
    }
    UIO_LOG_DBG("Success!");

    while ((entry = readdir(dp))) {
        UIO_LOG_DBG("Processing [%s]...", entry->d_name);
        if (strncmp(entry->d_name, "uio", 3) == 0) {
            count++;
            devices = realloc(devices, count * sizeof(uio_device_t));
            if (devices == NULL) {
                UIO_LOG_ERR("realloc() failed: %s", strerror(errno));
                goto err_realloc;
            }
            memset((devices + (count - 1)), 0x00, sizeof(uio_device_t));

            // Read uio_name
            UIO_LOG_DBG("Reading UIO's name...");
            devices[count-1].uio_name = malloc(size * sizeof(char)); 
            if (devices[count-1].uio_name == NULL) {
                UIO_LOG_ERR("malloc() failed: %s", strerror(errno));
                goto err_realloc;
            }
            snprintf(path, sizeof(path), "/sys/class/uio/%s/name", entry->d_name);
            file = fopen(path, "rb");
            if (file == NULL) {
                UIO_LOG_WARN("fopen(%s) failed: %s", path, strerror(errno));
                UIO_LOG_DBG("Moving to the next entry...");
                continue;
            }
            fscanf(file, "%s", devices[count-1].uio_name);
            fclose(file);
            UIO_LOG_DBG("uio_name: %s", devices[count-1].uio_name);

            // Read mem_size
            UIO_LOG_DBG("Reading the size of UIO reg map...");
            snprintf(path, sizeof(path), "/sys/class/uio/%s/maps/map0/size",
                     entry->d_name);
            file = fopen(path, "rb");
            if (file == NULL) {
                UIO_LOG_WARN("fopen(%s) failed: %s", path, strerror(errno));
                UIO_LOG_MSG("Moving to the next entry...");
                continue;
            }
            fscanf(file, "%s", path);
            fclose(file);
            char *endptr;
            devices[count-1].mem_size = (uint32_t)strtoul(path, &endptr, 16);
            if (*endptr != '\0') {
                UIO_LOG_WARN("Cannot convert [%s] to U32!", path);
                devices[count-1].mem_size = 0x0;
            }
            UIO_LOG_DBG("mem_size: 0x%X", devices[count-1].mem_size);

            // Open device file
            UIO_LOG_DBG("Opening the device file /dev/%s ...", entry->d_name);
            snprintf(path, sizeof(path), "/dev/%s", entry->d_name);
            int32_t fd = open(path, O_RDWR);
            if (fd < 0) {
                UIO_LOG_WARN("open(%s) failed: %s", path, strerror(errno));
                UIO_LOG_MSG("Moving to the next entry...");
                continue;
            }
            UIO_LOG_DBG("fd: %d", fd);
            // Memory map the device file
            UIO_LOG_DBG("Mapping /dev/%s file into memory...", entry->d_name);
            devices[count-1].mem_reg = mmap(NULL, devices[count-1].mem_size,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (devices[count-1].mem_reg == MAP_FAILED) {
                UIO_LOG_WARN("mmap(/dev/%s) failed: %s", entry->d_name, strerror(errno));
                devices[count-1].mem_reg = NULL;
            }
            UIO_LOG_DBG("mem_reg: %p\n", devices[count-1].mem_reg);
            close(fd);

            // Assign uio_number
            sscanf(entry->d_name + 3, "%u", &devices[count-1].uio_number);
        } else {
            UIO_LOG_DBG("Not an UIO entry; moving to the next entry...\n");
        }
    }
    err = API_CMS_ERROR_OK;
err_realloc:
    g_device_count = count;
    UIO_LOG_MSG("Number of UIO devices discovered: %u", g_device_count);
    closedir(dp);
end:
    return err;
}


/**
 * @brief   Get the starting address for the mapping of a UIO device.
 *
 * param[in]    name        Name of the UIO device the starting address we want
 *                          to get for.
 * param[in]    length      Length of the "name" string.
 *
 * @return      Non NULL address    The specified device exists and successfully
 *                                  mapped.
 * @return      NULL                The device doesn't exist or couldn't be
 *                                  mapped.
 */
void *adi_uio_device_mem_get(const char *name, uint32_t length)
{
    uint32_t i = 0;
    void *mem = NULL;

    if (name == NULL) {
        UIO_LOG_ERR("NULL pointer passed!");
        goto end;
    }

    for (i = 0; i < g_device_count; i++) {
        if (strncmp(name, devices[i].uio_name, length) == 0) {
            UIO_LOG_MSG("UIO device [%s] is mapped at address %p",
                        name, devices[i].mem_reg);
            mem = devices[i].mem_reg;
            goto end;
        }
    }
    UIO_LOG_WARN("UIO device [%s] not found!", name);
end: 
    return mem;
}


/**
 * @brief   Removes the UIO mappings
 *          Releases the memory.
 *
 * @return      API_CMS_ERROR_OK    API Completed Successfully.
 */
int32_t adi_uio_device_close(void)
{
    uint32_t i = 0;

    for(i = 0; i < g_device_count; i++) {
        free(devices[i].uio_name);
        devices[i].uio_name = NULL;
        if (devices[i].mem_reg != NULL) {
            munmap(devices[i].mem_reg, devices[i].mem_size);
        }
    }
    free(devices);
    devices = NULL;
    return API_CMS_ERROR_OK;
}


/**
 * @brief   Get the UIO number (e.g. /dev/uioX) of a particular UIO device
 *
 * param[in]    name    Name of the UIO device to get the UIO number for.
 * param[out]   uio_no  The UIO device number corresponding to the \p name
 *
 * @return      API_CMS_ERROR_OK     The API completed successfully and the UIO
 *                                  device number has been returned in \p uio_no.
 * @retun       API_CMS_ERROR_ERROR  Failed to find the \p name UIO.
 * @return      <(-1)      The API failed. @ref adi_cms_error_e for details.
 */
int32_t adi_uio_device_get_no(const char *name, uint32_t *uio_no)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint32_t i = 0;

    if (name == NULL || uio_no == NULL) {
        UIO_LOG_ERR("NULL pointer passed!");
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    for (i = 0; i < g_device_count; i++) {
        if (strcmp(name, devices[i].uio_name) == 0) {
            UIO_LOG_MSG("UIO device [%s] is /dev/uio%u",
                        name, devices[i].uio_number);
            *uio_no =  devices[i].uio_number;
            err = API_CMS_ERROR_OK;
            goto end;
        }
    }
    UIO_LOG_WARN("UIO device [%s] not found!", name);
end:
    return err;
}

/**
 * @brief   Prints the list of UIO devices discovered in the system.
 *          Helper/Debug API.
 *
 * @return API_CMS_ERROR_OK     API Completed Successfully.
 * @return <0                   The pointer to the list of devices is NULL.
 */
int32_t adi_uio_devices_print(void)
{
    int32_t err = API_CMS_ERROR_OK;
    uint32_t i = 0;

    if (devices == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    for (i = 0; i < g_device_count; i++) {
        printf("UIO Device %d:\n", i);
        UIO_LOG_MSG("UIO Device %d:", i);
        printf("  Name: %s\n", devices[i].uio_name);
        UIO_LOG_MSG("\tName: %s", devices[i].uio_name);
        printf("  Mem Size: %u\n", devices[i].mem_size);
        UIO_LOG_MSG("\tMem Size: %u", devices[i].mem_size);
        printf("  Mem Reg: %p\n", devices[i].mem_reg);
        UIO_LOG_MSG("\tMem Reg: %p", devices[i].mem_reg);
        printf("  UIO Number: %u\n", devices[i].uio_number);
        UIO_LOG_MSG("\tUIO Number: %u\n", devices[i].uio_number);
    }

end:
    return err;
}


#endif /* !defined(VERSAL_PLATFORM) */
