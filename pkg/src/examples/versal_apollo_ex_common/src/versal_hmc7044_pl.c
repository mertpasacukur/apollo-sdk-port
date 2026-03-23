#if !defined(VERSAL_PLATFORM)

/*!
 * @brief     HMC7044 Custom PL IP — 3-Wire SPI Implementation
 *
 * @copyright copyright(c) 2026. Port for Versal Standalone.
 */

#include <sleep.h>
#include "versal_hmc7044_pl.h"
#include "versal_debug.h"
#include "xparameters.h"
#include "xil_io.h"

typedef union
{
    struct
    {
        unsigned int wr_data: 8;
        unsigned int wr_address: 15;
        unsigned int rw: 1;
        unsigned int sel: 1;
        unsigned int reserved: 7;
    };
    unsigned int uiVal;
} __attribute__((packed)) sHMCWriteDataFormat;

int32_t hmcWrite(uint16_t address, uint8_t data)
{
    int32_t status = 0;
    sHMCWriteDataFormat sData;

    sData.wr_data = data;
    sData.wr_address = address;
    sData.rw = 0;
    sData.sel = 1;
    Xil_Out32(XPAR_SDTM_AXI_MEM_SPACE_1_BASEADDR + 0xC, sData.uiVal);
    usleep(1000);

    sData.sel = 0;
    Xil_Out32(XPAR_SDTM_AXI_MEM_SPACE_1_BASEADDR + 0xC, sData.uiVal);

    dbg_printf(DBG_INFO, "[HMC7044] WRITE 0x%04X = 0x%02X (status=%d)\r\n", address, data, status);

    return status;
}

int32_t hmcRead(uint16_t address, uint8_t *data)
{
    int32_t status = 0;
    sHMCWriteDataFormat sTxData;

    if (data == NULL)
    {
        dbg_printf(DBG_ERROR, "[HMC7044] READ error: NULL pointer\r\n");
        return -1;
    }

    sTxData.wr_data = 0;
    sTxData.wr_address = address;
    sTxData.rw = 1;
    sTxData.sel = 1;
    Xil_Out32(XPAR_SDTM_AXI_MEM_SPACE_1_BASEADDR + 0xC, sTxData.uiVal);
    usleep(100);

    sTxData.sel = 0;
    Xil_Out32(XPAR_SDTM_AXI_MEM_SPACE_1_BASEADDR + 0xC, sTxData.uiVal);
    usleep(100);

    *data = (Xil_In32(XPAR_SDTM_AXI_MEM_SPACE_0_BASEADDR + 0x8) & 0xFF);
    dbg_printf(DBG_INFO, "[HMC7044] READ  0x%04X = 0x%02X (status=%d)\r\n", address, *data, status);

    return status;
}

#endif /* !defined(VERSAL_PLATFORM) */
