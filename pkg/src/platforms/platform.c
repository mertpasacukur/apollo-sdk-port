#if defined(__linux__)


/*!
 * @brief     Hardware Abstraction Layer.
 *
 * @copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __PLATFORM__
 * @{
 */

/*============= I N C L U D E S ============*/
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "platform.h"
#include "adi_cms_api_common.h"


/*============= D E F I N E S ==============*/
#define PLATFORM_ID_DEVICE  "uio_platform_id"
#define UIO_SYSFS           "/sys/class/uio/uio"
#define UIO_DEV             "/dev/uio"

/*============= D A T A ====================*/

/*============= S T A T I C  F U N C T I O N S  D E C L A R A T I O N ========*/


/*============= H A L  F U N C T I O N S  D E F I N I T I O N ================*/
int32_t get_platform_id(uint32_t *platform_type)
{
    const uint32_t max_size = 64;
    const uint32_t max_uios = 99;
    uint32_t map_size = 0, found = 0, i = 0;
    int32_t err = API_CMS_ERROR_OK;
    int fd = -1;
    char uio_name[max_size], name[max_size];
    void *platform_id_mem = NULL;
    char *buff = NULL;

    if (platform_type == NULL)
        return API_CMS_ERROR_ERROR;

    memset(uio_name, 0x00, max_size);
    memset(name, 0x00, max_size);

    for (i = 0; i <= max_uios; i++) {
        snprintf(uio_name, max_size, "%s%d/name", UIO_SYSFS, i);
        if (access(uio_name, F_OK) != 0) {
            /* this means we got at the end of uio devices, 99% of the time */
            break;
        }
        fd = open(uio_name, O_RDONLY);
        if (fd < 0) {
            printf("%s(): ERROR opening file %s: %s\n",
                   __func__, uio_name, strerror(errno));
            err = API_CMS_ERROR_ERROR;
            goto end;
        }
        read(fd, name, max_size);
        close(fd);
        buff = strchr(name, '\n');
        *buff = '\0';

        if (strcmp(name, PLATFORM_ID_DEVICE) == 0) {
            found = 1;
            /*get map size*/
            memset(uio_name, 0x00, max_size);
            memset(name, 0x00, max_size);

            snprintf(uio_name, max_size, "%s%d/maps/map0/size", UIO_SYSFS, i);
            fd = open(uio_name, O_RDONLY);
            if (fd < 0) {
                printf("%s(): ERROR opening file %s: %s\n",
                       __func__, uio_name, strerror(errno));
                err = API_CMS_ERROR_ERROR;
                goto end;
            }
            read(fd, name, max_size);
            close(fd);

            map_size = strtoul(name, NULL, 16);
            if (map_size == 0) {
                printf("%s(): Error getting the map size!\n", __func__);
                err = API_CMS_ERROR_ERROR;
                goto end;
            }

            break;
        }
    }

    if (!found) {
        err = 1;
        goto end;
    }

    memset(uio_name, 0x00, max_size);
    err = snprintf(uio_name, max_size, "%s%d", UIO_DEV, i);
    if (err < 0) {
        printf("%s(): ERROR snprintf-ing UIO_DEV!\n", __func__);
        goto end;
    }
    err = API_CMS_ERROR_OK;

    fd = open(uio_name, O_RDWR);
    if (fd < 0) {
        printf("%s(): ERROR opening %s: %s\n",
               __func__, uio_name, strerror(errno));
        err = API_CMS_ERROR_ERROR;
        goto end;
    }
    platform_id_mem = mmap(NULL, map_size, PROT_READ|PROT_WRITE,
                           MAP_SHARED, fd, 0);
    close(fd);
    if (platform_id_mem == MAP_FAILED) {
        printf("%s(): UIO mmap ERROR!\n", __func__);
        err = API_CMS_ERROR_ERROR;
        goto end;
    }
    *platform_type = *((uint32_t *)platform_id_mem);
    printf("PLATFORM_ID: 0x%08X\n", *platform_type);
    munmap(platform_id_mem, map_size);
end:
    return err;
}

/*============= S T A T I C  F U N C T I O N S  D E F I N I T I O N ==========*/




#endif /* defined(__linux__) */
