#if defined(__linux__)

/*!
 * \brief     Basic FPGA ADS10 config functions
 *
 * \copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __FPGA_ADS10__
 */

/*============= I N C L U D E S ============*/
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "ads10_fpga_config.h"
#include "adi_cms_api_common.h"
#include "cdmaapi.h"
#include "smbus.h"
#include "axi_dmac.h"

/*============= D E F I N E S ==============*/

#define M_AXI_REGBUS_MZ         0x60010000
#define M_AXI_REGBUS_MZ_SIZE    0x10000
// Configure MicroZed reserved memory base_addr & size as: MZDDR_ADDR+MMAP_SIZE = 0x40000000 
#define RESV_BUF_ADDR           0x3C000000
#define RESV_BUF_SIZE           0x04000000
#define M_AXI_FIFO              0x90000000

#define FILE_NAME_SIZE      64
#define BUFF_SIZE           3
#define GPIO_NO             3

#define MAX_LOG_LINE_LENGTH     1000
#define FPGA_HAL_LOG_ENABLED    0

/*============= D A T A ====================*/
static int g_baseaddr;
static int g_size;
static int g_fd = -1;
static int g_i2c = -1;
static void* g_membase = NULL;

/*======== S T A T I C  F U N C T I O N S  D E C L A R A T I O N ============*/
static int32_t program_vup(uint8_t bin_file[],
    uint32_t length);
static int32_t Hal_FpgaInit(uint32_t baseaddr,
    uint32_t size);
static int32_t Hal_FpgaDeinit(void);
static uint32_t Hal_FpgaIn32(uint32_t reg);
static uint32_t Hal_FpgaOut32(uint32_t reg,
    uint32_t value);
static int32_t i2c_access_open(char* i2c_dev);
static int32_t i2c_access_close(void);
static int32_t i2c_access_byte(uint8_t i2c_write,
    uint8_t slave_addr, uint8_t reg_addr, uint8_t wr_data, uint8_t *rd_data);
static uint32_t program_b_pulse(void);
static void ads10_fifo_config(uint8_t data[],
    uint32_t length, uint32_t config_smapx8);
static int32_t gpios_export(void);
static int32_t gpios_unexport(void);
static int32_t gpios_set_direction(void);
static int32_t ads10_fpga_config_load_image2(uint8_t file[], uint32_t length);

/*===== F P G A  C O N F I G  F U N C T I O N S  D E F I N I T I O N S ======*/
int32_t ads10_fpga_config_load_image(uint8_t file[], uint32_t length)
{
    int32_t err;

    err = ads10_fpga_config_load_image2(file, length);
    
    if (err != API_CMS_ERROR_OK) {

        printf("\n\n[***** Retry FPGA Image Load *****]\n\n");

        /* clear and retry */
        cdma_clear();

        err = ads10_fpga_config_load_image2(file, length);
     }

     return err;
}


static int32_t ads10_fpga_config_load_image2(uint8_t file[], uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    int i;
    int gpio964_fd, gpio965_fd;
    char gpio964[1], gpio965[1];
    uint32_t c2c_status = 0;


    if (file == NULL) {
        err = API_CMS_ERROR_NULL_PARAM;
        goto end;
    }

    err = gpios_export();
    if (err != API_CMS_ERROR_OK) goto end;

    err = gpios_set_direction();
    if (err != API_CMS_ERROR_OK) goto end_gpios_unexport;


    err = program_vup(file, length);
    if (err != API_CMS_ERROR_OK) {
        goto end_gpios_unexport;
    }
    
    for (i = 0; i < 100; i++) {
        c2c_status = 0;
        usleep(100000);

        gpio964_fd = open("/sys/class/gpio/gpio964/value", O_RDONLY);
        if (gpio964_fd == -1) {
            err = -1;
            goto end_gpios_unexport;
        }
        gpio965_fd = open("/sys/class/gpio/gpio965/value", O_RDONLY);
        if (gpio965_fd == -1) {
            err = -1;
            goto err_gpio964;
        }

        err = read(gpio964_fd, &gpio964, 1);
        if (err == -1) {
            err = API_CMS_ERROR_ERROR;
            goto err_gpio965;
        }
        c2c_status += atoi(gpio964);

        err = read(gpio965_fd, &gpio965, 1);
        if (err == -1) {
            err = API_CMS_ERROR_ERROR;
            goto err_gpio965;
        }
        c2c_status += (atoi(gpio965) << 1);

        if (c2c_status == 0x3) {
            break;
        }
        close(gpio964_fd);
        close(gpio965_fd);
    }

    if (c2c_status ==  0x3) {
        err = API_CMS_ERROR_OK;
    } else {
        err = API_CMS_ERROR_ERROR;
    }

err_gpio965:
    close(gpio965_fd);
err_gpio964:
    close(gpio964_fd);
end_gpios_unexport:
    gpios_unexport();
end:
    return err;
}


/*========== S T A T I C  F U N C T I O N S  D E F I N I T I O N S ===========*/


static int32_t program_vup(uint8_t bin_file[],
    uint32_t length)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t config_mode = 0;
    uint8_t config_smapx8 = 0; 

    err = Hal_FpgaInit(M_AXI_REGBUS_MZ, M_AXI_REGBUS_MZ_SIZE);
    if (err != API_CMS_ERROR_OK) goto end;

    //read config_mode register bit (in real, it is read from i2c io expander)
    config_mode = Hal_FpgaIn32(0x11) & 0x1; //reg[0x11][0]=config_m0

    uint8_t i2c_wr_data = 0;
    uint8_t i2c_rd_data = 0;

    // Switch to selected I2C bus.
    // Try to access a i2c different than i2c-4 to force i2c switch from whatever state to i2c-4 (it has some chache memory) -> i2c-1 ADM1178 (U54)
    err = i2c_access_open("/dev/i2c-1");
    if (err != API_CMS_ERROR_OK)
        goto end;

    // It could fail since i2c switch is not correctly set, so result is ommited.
    (void)i2c_access_byte(0, 0x72, 0x01, 0, &i2c_rd_data);
    i2c_access_close();

    // Set CONFIG_SEL to 0
    err = i2c_access_open("/dev/i2c-4");
    if (err != API_CMS_ERROR_OK) goto end;

    err = i2c_access_byte(0, 0x34, 0x2B, 0, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;
    i2c_wr_data = ((i2c_rd_data) | 0x20) & 0xFE;

    err = i2c_access_byte(1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;
    i2c_access_close();

    uint8_t buf[1];
    int sel_fd = open("/sys/class/gpio/gpio963/value", O_RDONLY);
    (void)read(sel_fd, &buf[0], 1);
    close(sel_fd);

    if (config_mode == 0) {
        //SMAPx8 mode
        config_smapx8 = 1;
        Hal_FpgaOut32(0x4, 0x1); //i2c_config_sel mux select in FPGA design
    } else {
        //SERIALx1 mode
        config_smapx8 = 0;
        Hal_FpgaOut32(0x4, 0x0);	//i2c_config_sel mux select in FPGA design
    }

    if (config_smapx8 == 1) {
        //SMAPx8 mode
        //flush dma and fifo
        Hal_FpgaOut32(0x0, 0x2);
        usleep(10 * 1000);
        Hal_FpgaOut32(0x0, 0x0);

        //toggle fifo reset
        Hal_FpgaOut32(0x0, 0x1);
        Hal_FpgaOut32(0x0, 0x0);

        //set image_ready to 1
        Hal_FpgaOut32(0x1, 0x1);
        usleep(10 * 1000);
    }

    //sent the program_b pulse
    err = program_b_pulse();
    if (err != API_CMS_ERROR_OK) goto end;

    ads10_fifo_config(bin_file, length, config_smapx8);

    // Set CONFIG_SEL to 1
    err = i2c_access_open("/dev/i2c-4");
    if (err != API_CMS_ERROR_OK) goto end;
    err = i2c_access_byte(0, 0x34, 0x2B, 0, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;
    i2c_wr_data = (i2c_rd_data | 0x01);
    // It has been proved that first write to i2c with a fresh FPGA programmed may fail. Thus first result is ignored
    (void)i2c_access_byte(1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    err = i2c_access_byte(1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;
    i2c_access_close();
end:
    Hal_FpgaDeinit();
    return err;
}


static int32_t Hal_FpgaInit(uint32_t baseaddr,
    uint32_t size)
{
    int32_t err = API_CMS_ERROR_OK;

    g_baseaddr = g_size = 0;
    g_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (g_fd > 0) {
        g_membase = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
            g_fd, baseaddr);
        if (g_membase != (void*) - 1) {
            g_baseaddr = baseaddr;
            g_size = size;
            err = 0;
        } else {
            err = API_CMS_ERROR_ERROR;
        }
    } else {
        err = API_CMS_ERROR_ERROR;
    }
    return err;
}


static int32_t Hal_FpgaDeinit(void)
{
    if ((g_baseaddr != 0) && (g_size != 0)) {
        munmap((void*)g_membase, g_size);
        g_baseaddr = g_size = 0;
    }
    close(g_fd);
    g_membase = NULL;

    return 0;
}


static uint32_t Hal_FpgaIn32(uint32_t reg)
{
    uint32_t vl;

    vl = *((volatile uint32_t*)((uint32_t)g_membase + (reg << 2)));
    return vl;
}


static uint32_t Hal_FpgaOut32(uint32_t reg,
    uint32_t value)
{
    *((volatile uint32_t*)((uint32_t)g_membase + (reg << 2))) = value;

    return 0;
}


static int32_t i2c_access_open(char* i2c_dev)
{
    int32_t err = -1;

    g_i2c = open(i2c_dev, O_RDWR);
    if (g_i2c < 0) {
        err = -1;
    } else {
        err = 0;
    }
    return err;
}


static int32_t i2c_access_byte (uint8_t i2c_write,
    uint8_t slave_addr, uint8_t reg_addr, uint8_t wr_data, uint8_t *rd_data)
{
    int32_t err = -1;

    //open the device file
    if (g_i2c < 0) {
        goto end;
    }

    //specify slave device address
    if (ioctl(g_i2c, I2C_SLAVE, slave_addr) < 0) {
        goto end;
    }

    if (i2c_write == 0) { //read byte operation
        err = i2c_smbus_read_byte_data(g_i2c, reg_addr);
        if (err >= 0){
            *rd_data = err;
            err = API_CMS_ERROR_OK;
        }
        
    } else {
        err = i2c_smbus_write_byte_data(g_i2c, reg_addr, wr_data);
        if (err >= 0) {
            err = API_CMS_ERROR_OK;
        }
    }
end:
    return err;
}


static int32_t i2c_access_close(void)
{
    close(g_i2c);
    return 0;
}


static uint32_t program_b_pulse (void)
{
    int32_t err = API_CMS_ERROR_OK;
    uint8_t i2c_wr_data = 0;
    uint8_t i2c_rd_data = 0;

    //set prog_b pulse through I2C IO expander
/* Set I2C MUX PCA9545 at 0x70:
 * 0x1=PMU_SDA/SCL, 0x2=CLK_SDA/SCL, 0x4=FMC_SDA/SCL, 0x8=SDA/SCL(I2C IO expander)
* i2c_access_byte (0, 0x70, 0x8, i2c_wr_data, &i2c_rd_data);
* I2C IO expander at 0x34:
*	C0 (GPIO9)  = config_sel (default dir: input,  0x31[0]=0)(default GPO val: 0x2B[0]=0)
*	C1 (GPIO10) = program_b  (default dir: output, 0x31[1]=1)(default GPO val: 0x2B[1]=1)
*	C2 (GPIO11) = fmc_pg_c2m (default dir: input,  0x31[2]=0)(default GPO val: 0x2B[2]=0)
*	C3 (GPIO12) = fx3_status (default dir: output, 0x31[3]=1)(default GPO val: 0x2B[3]=0)
*	C4 (GPIO13) = fmc_pg_m2c (default dir: input,  0x31[4]=0)(default GPO val: 0x2B[4]=0)
*/
    err = i2c_access_open("/dev/i2c-4");
    if (err != API_CMS_ERROR_OK) goto end;

    err = i2c_access_byte (0, 0x34, 0x2B, 0, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;

    i2c_wr_data = (i2c_rd_data & 0xFD) | (0x2);

    //set program_b bit 0x2B[1] to 1 (default)
    err = i2c_access_byte (1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;

    err = i2c_access_byte (0, 0x34, 0x2B, 0, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;

    i2c_wr_data = (i2c_rd_data & 0xFD) | (0x0);

    //set program_b bit 0x2B[1] to 0, start the pulse, min 250ns
    err = i2c_access_byte (1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;

    usleep(1);

    err = i2c_access_byte (0, 0x34, 0x2B, 0, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;
    i2c_wr_data = (i2c_rd_data & 0xFD) | (0x2);

    //set program_b bit 0x2B[1] to 1, finish the pulse
    err = i2c_access_byte (1, 0x34, 0x2B, i2c_wr_data, &i2c_rd_data);
    if (err != API_CMS_ERROR_OK) goto end;

    i2c_access_close();
    /* Then CDMA will write to the FIFO and start configuration process by reading out the FIFO */
end:
    return err;
}


static void ads10_fifo_config(uint8_t data[],
    uint32_t length, uint32_t config_smapx8)
{
    uint32_t MZDDRAddr_0 = RESV_BUF_ADDR;
    uint32_t FIFOAddr = M_AXI_FIFO;
    uint32_t mmap_size = DMA_1MSIZE;

    uint32_t i, fd;
    uint32_t rd_loop, rd_last;
    void*    mem_base;

    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == 0) {
        goto end;
    }

    mem_base = mmap(0, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, MZDDRAddr_0);
    close(fd);
    if (mem_base == (void*) - 1) {
        goto end;
    }

    rd_last = length % DMA_1MSIZE;
    rd_loop = length / DMA_1MSIZE;

    if (config_smapx8 != 1) {// SERIALx1 mode
        goto end_unmap;
    }

    uint32_t offset = 0;
    for (i = 0; i <= rd_loop; i++) {
        uint32_t count = (i == rd_loop) ? rd_last : DMA_1MSIZE;

        memcpy((uint8_t*)mem_base, (data + offset), count);

#ifndef __MICROBALZE__
        if (CdmaTransfer(MZDDRAddr_0, FIFOAddr, count, 1, 1) != 0) {
            break;		
#else
        if (axi_dmac_transfer_start(MZDDRAddr_0, FIFOAddr, count, 1, 1) != 0) {
            break;
#endif /* __MICRBOALZE__ */
        }
        offset += count;
    }

end_unmap:
    munmap(mem_base, mmap_size);
end:
    return;
}


static int32_t gpios_unexport(void)
{
    int32_t err = API_CMS_ERROR_OK, fd, bytes_wr;
    char file_name[FILE_NAME_SIZE] = "/sys/class/gpio/unexport";
    uint32_t gpio[GPIO_NO] = {963, 964, 965}, i;

    fd = open(file_name, O_WRONLY);
    if (fd == -1) {
        return -1;
    }

    for(i = 0; i < GPIO_NO; i++) {
        snprintf(file_name, FILE_NAME_SIZE,
             "/sys/class/gpio/gpio%u/value", gpio[i]);
        if (access(file_name, F_OK) == 0) {
            snprintf(file_name, FILE_NAME_SIZE,
                "%u", gpio[i]);
            err = write(fd, file_name, sizeof(file_name));
            bytes_wr = write(fd, file_name, sizeof(file_name));
            if (bytes_wr != sizeof(file_name)) {
                err = -1;
                goto end_fd;
            }
        }
    }

end_fd:
    close(fd);
    return err;
}


static int32_t gpios_export(void)
{
    int32_t err  = API_CMS_ERROR_OK, fd, bytes_wr;
    char file_name[FILE_NAME_SIZE] = "/sys/class/gpio/export";
    uint32_t gpio[GPIO_NO] = {963, 964, 965}, i;

    fd = open(file_name, O_WRONLY);
    if (fd == -1) {
        return -1;
    }

    for (i = 0; i < GPIO_NO; i++) {
        snprintf(file_name, FILE_NAME_SIZE,
             "/sys/class/gpio/gpio%u/value", gpio[i]);
        if (access(file_name, F_OK) != 0) {
            snprintf(file_name, FILE_NAME_SIZE,
                "%u", gpio[i]);
            bytes_wr = write(fd, file_name, sizeof(file_name));
            if (bytes_wr != sizeof(file_name)) {
                err = -1;
                goto end_fd;
            }
        }
    }

end_fd:
    close(fd);
    return err;
}


static int32_t gpios_set_direction(void)
{
    int32_t err = API_CMS_ERROR_OK, fd;
    uint32_t gpio[GPIO_NO] = {963, 964, 965}, i;
    char file_name[FILE_NAME_SIZE] = "";
    char* dir[GPIO_NO] = {"in", "in", "in"};

    for (i = 0; i < GPIO_NO; i++) {
        snprintf(file_name, FILE_NAME_SIZE,
             "/sys/class/gpio/gpio%u/direction", gpio[i]);
        fd = open(file_name, O_WRONLY);
        if (fd == -1) {
            err = -1;
            goto end;
        }
        if (write(fd, dir[i], sizeof(dir[i])) != sizeof(dir[i])) {
            goto end_fd;
        }
        close(fd);
    }
end:
    return err;
end_fd:
    close(fd);
    return -1;
}
#endif /* defined(__linux__) */
