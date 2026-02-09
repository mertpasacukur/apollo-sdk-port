/*!
 * \brief     ADF4382 HAL Functionality
 *
 * \copyright copyright(c) 2022 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

#include "adi_adf4382_hal.h"

#define SPI_IN_OUT_BUFF_SZ 0x3
#define GET_ADDRESS_MSB(addr)        ((addr >> 8) & 0x7F)
#define GET_ADDRESS_LSB(addr)        (addr & 0xFF)
#define REG_OFFSET_GET(info)         ((uint8_t)(info >> 0))
#define REG_WIDTH_GET(info)          ((uint8_t)(info >> 8))
#define REG_BYTES_GET(width, offset) (((width + offset) >> 3) + (((width + offset) & 7) == 0 ? 0 : 1))

typedef int32_t(*byte_index_calc)(uint8_t index, uint8_t size);

static void __txn_config_init(adi_cms_hal_txn_config_t* config);
static int32_t __hal_reg_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t *data);
static int32_t __hal_reg_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t data);
static int32_t __le_byte_index_get(uint8_t index, uint8_t size);
static int32_t __be_byte_index_get(uint8_t index, uint8_t size);
static int32_t adi_adf4382_hal_reg_tbl_set(adi_adf4382_device_t *adf4382, adi_cms_reg_data_t tbl[], uint32_t count);

static adi_cms_reg_data_t ADF4382_U2_INIT_TBL[] = {
    {0x000A, 0x0A},
    {0x0200, 0x00},
    {0x0201, 0x00},
    {0x0202, 0x00},
    {0x0203, 0x00},
    {0x0054, 0x00},
    {0x0053, 0x45},
    {0x0052, 0x00},
    {0x0051, 0x00},
    {0x0050, 0x00},
    {0x004F, 0x08},
    {0x004E, 0x06},
    {0x004D, 0x04},
    {0x004C, 0x2B},
    {0x004B, 0x5D},
    {0x004A, 0x00},
    {0x0048, 0x00},
    {0x0047, 0x00},
    {0x0046, 0x00},
    {0x0045, 0x06},
    {0x0044, 0x1B},
    {0x0043, 0xB8},
    {0x0042, 0x01},
    {0x0041, 0x00},
    {0x0040, 0x00},
    {0x003F, 0x82},
    {0x003E, 0x27},
    {0x003D, 0x00},
    {0x003C, 0x00},
    {0x003B, 0x00},
    {0x003A, 0xFA},
    {0x0039, 0x00},
    {0x0038, 0x7C},
    {0x0037, 0xCA},
    {0x0036, 0xC0},
    {0x0035, 0x3F},
    {0x0034, 0x36},
    {0x0033, 0x00},
    {0x0032, 0x40},
    {0x0031, 0x63},
    {0x0030, 0x0F},
    {0x002F, 0x3F},
    {0x002E, 0x00},
    {0x002D, 0xF1},
    {0x002C, 0x0C},
    {0x002B, 0x01},
    {0x002A, 0x30},
    {0x0029, 0x09},
    {0x0028, 0x00},
    {0x0027, 0xF0},
    {0x0026, 0x00},
    {0x0025, 0x01},
    {0x0024, 0x01},
    {0x0023, 0x00},
    {0x0022, 0x00},
    {0x0021, 0x00},
    {0x0020, 0xC1},
    {0x001F, 0x0F},
    {0x001E, 0x20},
    {0x001D, 0x00},
    {0x001C, 0x00},
    {0x001B, 0x00},
    {0x001A, 0x00},
    {0x0019, 0x00},
    {0x0018, 0x00},
    {0x0017, 0x00},
    {0x0016, 0x00},
    {0x0015, 0x06},
    {0x0014, 0x00},
    {0x0013, 0x00},
    {0x0012, 0x00},
    {0x0011, 0x00},
    {0x0010, 0x50}
};

static adi_cms_reg_data_t ADF4382_U4_INIT_TBL[] = {
    {0x000A, 0xA5},
    {0x0200, 0x00},
    {0x0201, 0x00},
    {0x0202, 0x00},
    {0x0203, 0x00},
    {0x0100, 0x25},
    {0x0101, 0x3F},
    {0x0102, 0x3F},
    {0x0103, 0x3F},
    {0x0104, 0x3F},
    {0x0105, 0x3F},
    {0x0106, 0x3F},
    {0x0107, 0x3F},
    {0x0108, 0x3F},
    {0x0109, 0x25},
    {0x010A, 0x25},
    {0x010B, 0x3F},
    {0x010C, 0x3F},
    {0x010D, 0x3F},
    {0x010E, 0x3F},
    {0x010F, 0x3F},
    {0x0110, 0x3F},
    {0x0111, 0x3F},
    {0x0054, 0x00},
    {0x0053, 0x45},
    {0x0052, 0x00},
    {0x0051, 0x00},
    {0x0050, 0x00},
    {0x004F, 0x08},
    {0x004E, 0x06},
    {0x004D, 0x00},
    {0x004C, 0x2B},
    {0x004B, 0x5D},
    {0x004A, 0x00},
    {0x0048, 0x00},
    {0x0047, 0x00},
    {0x0046, 0x00},
    {0x0045, 0x62},
    {0x0044, 0x3F},
    {0x0043, 0xB8},
    {0x0042, 0x01},
    {0x0041, 0x00},
    {0x0040, 0x00},
    {0x003F, 0x82},
    {0x003E, 0x4E},
    {0x003D, 0x00},
    {0x003C, 0x00},
    {0x003B, 0x00},
    {0x003A, 0xFA},
    {0x0039, 0x00},
    {0x0038, 0x71},
    {0x0037, 0x82},
    {0x0036, 0xC0},
    {0x0035, 0x00},
    {0x0034, 0x36},
    {0x0033, 0x00},
    {0x0032, 0x40},
    {0x0031, 0x6B},
    {0x0030, 0x0F},
    {0x002F, 0x3F},
    {0x002E, 0x00},
    {0x002D, 0xF1},
    {0x002C, 0x0E},
    {0x002B, 0x01},
    {0x002A, 0x30},
    {0x0029, 0x09},
    {0x0028, 0x00},
    {0x0027, 0xF0},
    {0x0026, 0x00},
    {0x0025, 0x01},
    {0x0024, 0x01},
    {0x0023, 0x00},
    {0x0022, 0x00},
    {0x0021, 0x00},
    {0x0020, 0xC1},
    {0x001F, 0x0F},
    {0x001E, 0x20},
    {0x001D, 0x00},
    {0x001C, 0x00},
    {0x001B, 0x00},
    {0x001A, 0x00},
    {0x0019, 0x00},
    {0x0018, 0x00},
    {0x0017, 0x00},
    {0x0016, 0x00},
    {0x0015, 0x06},
    {0x0014, 0x00},
    {0x0013, 0x00},
    {0x0012, 0x00},
    {0x0011, 0x00},
    {0x0010, 0x50},
};

static adi_cms_reg_data_t ADF4382_U5_INIT_TBL[] = {
    {0x0001, 0x00},
    {0x0002, 0x00},
    {0x0003, 0x06},
    {0x0004, 0x08},
    {0x0005, 0x00},
    {0x0006, 0x00},
    {0x0007, 0x00},
    {0x0008, 0x00},
    {0x0009, 0x00},
    {0x000A, 0xA5},
    {0x000B, 0x01},
    {0x000C, 0x56},
    {0x000D, 0x04},
    {0x000E, 0x00},
    {0x000F, 0x00},
    {0x0010, 0x50},
    {0x0011, 0x00},
    {0x0012, 0x00},
    {0x0013, 0x00},
    {0x0014, 0x00},
    {0x0015, 0x86},
    {0x0016, 0x85},
    {0x0017, 0x00},
    {0x0018, 0x00},
    {0x0019, 0x00},
    {0x001A, 0x28},
    {0x001B, 0x6B},
    {0x001C, 0xEE},
    {0x001D, 0x4A},
    {0x001E, 0x28},
    {0x001F, 0x1F},
    {0x0020, 0xC1},
    {0x0021, 0x00},
    {0x0022, 0x00},
    {0x0023, 0x00},
    {0x0024, 0x01},
    {0x0025, 0x01},
    {0x0026, 0x00},
    {0x0027, 0xF0},
    {0x0028, 0x20},
    {0x0029, 0x09},
    {0x002A, 0x30},
    {0x002B, 0x01},
    {0x002C, 0xC5},
    {0x002D, 0x31},
    {0x002E, 0x00},
    {0x002F, 0x3F},
    {0x0030, 0x0F},
    {0x0031, 0x63},
    {0x0032, 0x40},
    {0x0033, 0x00},
    {0x0034, 0x36},
    {0x0035, 0x3F},
    {0x0036, 0x80},
    {0x0037, 0xCA},
    {0x0038, 0x7C},
    {0x0039, 0x00},
    {0x003A, 0xFA},
    {0x003B, 0x00},
    {0x003C, 0x00},
    {0x003D, 0x00},
    {0x003E, 0x27},
    {0x003F, 0x82},
    {0x0040, 0x00},
    {0x0041, 0x00},
    {0x0042, 0x01},
    {0x0043, 0xB8},
    {0x0044, 0x2E},
    {0x0045, 0x52},
    {0x0046, 0x00},
    {0x0047, 0x00},
    {0x0048, 0x00},
    {0x0049, 0xFF},
    {0x004A, 0x00},
    {0x004B, 0x5D},
    {0x004C, 0x2B},
    {0x004D, 0x00},
    {0x004E, 0x06},
    {0x004F, 0x08},
    {0x0050, 0x00},
    {0x0051, 0x00},
    {0x0052, 0x00},
    {0x0053, 0x45},
    {0x0054, 0x01},
    {0x0055, 0x00},
    {0x0056, 0x00},
    {0x0057, 0x00},
    {0x0058, 0x08},
    {0x0059, 0x3F},
    {0x005A, 0x00},
    {0x005B, 0x6C},
    {0x005C, 0x00},
    {0x005D, 0xB8},
    {0x005E, 0x9B},
    {0x005F, 0x00},
    {0x0060, 0x00},
    {0x0061, 0x00},
    {0x0062, 0x00},
    {0x0063, 0x00},
    {0x0064, 0x4A},
    {0x0065, 0x08},
    {0x0066, 0x00},
    {0x0067, 0x0B},
    {0x0100, 0x3F},
    {0x0101, 0x3F},
    {0x0102, 0x3F},
    {0x0103, 0x3F},
    {0x0104, 0x3F},
    {0x0105, 0x3F},
    {0x0106, 0x3F},
    {0x0107, 0x3F},
    {0x0108, 0x3F},
    {0x0109, 0x25},
    {0x010A, 0x25},
    {0x010B, 0x3F},
    {0x010C, 0x3F},
    {0x010D, 0x3F},
    {0x010E, 0x3F},
    {0x010F, 0x3F},
    {0x0110, 0x3F},
    {0x0111, 0x3F},
    {0x0010, 0x5A},
};


int32_t adi_adf4382_hal_reg_default_set(adi_adf4382_device_t *adf4382, uint8_t chip_ver)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint16_t tbl_count = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_write);
    printf("ADF4382 Chip Variant: %d.\t", chip_ver);

    switch (chip_ver) {

        // Load default register values for U2.
        case ADI_ADF4382_CHIP_VER_U2:
            tbl_count = sizeof(ADF4382_U2_INIT_TBL) / sizeof(ADF4382_U2_INIT_TBL[0]);
            printf("U2 Table Count: %d.\n", tbl_count);

            err = adi_adf4382_hal_reg_tbl_set(adf4382, ADF4382_U2_INIT_TBL, tbl_count);
            ADI_CMS_ERROR_RETURN(err);
            break;

        // Load default register values for U4.
        case ADI_ADF4382_CHIP_VER_U4:
            tbl_count = sizeof(ADF4382_U4_INIT_TBL) / sizeof(ADF4382_U4_INIT_TBL[0]);
            printf("U4 Table Count: %d.\n", tbl_count);

            err = adi_adf4382_hal_reg_tbl_set(adf4382, ADF4382_U4_INIT_TBL, tbl_count);
            ADI_CMS_ERROR_RETURN(err);
            break;

        // Load default register values for U5.
        case ADI_ADF4382_CHIP_VER_U5_A:
        case ADI_ADF4382_CHIP_VER_U5_B:
        case ADI_ADF4382_CHIP_VER_U5_C:
            tbl_count = sizeof(ADF4382_U5_INIT_TBL) / sizeof(ADF4382_U5_INIT_TBL[0]);
            printf("U5 Table Count: %d.\n", tbl_count);

            err = adi_adf4382_hal_reg_tbl_set(adf4382, ADF4382_U5_INIT_TBL, tbl_count);
            ADI_CMS_ERROR_RETURN(err);
            break;

        default:
            printf("Invalid ADF4382 Chip Version provided: %d.\n", chip_ver);
            return API_CMS_ERROR_INVALID_PARAM;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_hal_reg_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t *data)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_read);
    ADI_CMS_NULL_PTR_CHECK(data);

    return __hal_reg_get(adf4382, reg, data);
}

int32_t adi_adf4382_hal_reg_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t data)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_write);

    return __hal_reg_set(adf4382, reg, data);
}

int32_t adi_adf4382_private_hal_bf_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint32_t info, uint8_t value[], uint8_t value_size_bytes)
{
    uint32_t endian_test_val = 0x11223344;
    byte_index_calc indexer = (*(uint8_t *)&endian_test_val == 0x44)
        ? &__le_byte_index_get
        : &__be_byte_index_get;
    int32_t err;
    uint8_t offset = REG_OFFSET_GET(info);
    uint8_t byte_offset;

    uint8_t width = REG_WIDTH_GET(info);
    uint8_t reg_bytes = REG_BYTES_GET(width, offset);

    uint8_t reg_value;
    uint8_t mask;

    err = __hal_reg_get(adf4382, reg, &reg_value);
    ADI_CMS_ERROR_RETURN(err);

    mask = (1 << width) - 1;
    value[indexer(0, reg_bytes)] = (reg_value >> offset) & mask;
    width = width < 8 ? 0 : width - (8 - offset);

    for (byte_offset = 1; byte_offset < reg_bytes; byte_offset++)
    {
        err = __hal_reg_get(adf4382, reg + byte_offset, &reg_value);
        ADI_CMS_ERROR_RETURN(err);
        mask = (1 << width) - 1;
        value[indexer(byte_offset -1, reg_bytes)] |= (((reg_value & mask) & ((1 << offset) - 1)) << offset);
        width = width < offset ? 0 : width - offset;
        if (width > 0)
        {
            mask = (1 << width) - 1;
            value[indexer(byte_offset, reg_bytes)] = (reg_value >> offset) & mask;
            width = width < (8 - offset) ? 0 : width - (8 - offset);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4382_private_hal_bf_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint32_t info, uint64_t value)
{
    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_read);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_write);

    int32_t err;
    uint8_t offset = REG_OFFSET_GET(info);
    uint8_t byte_offset;

    uint8_t width = REG_WIDTH_GET(info);
    uint8_t reg_bytes = REG_BYTES_GET(width, offset);
    uint8_t mask = ((1 << width) - 1) << offset;
    //uint16_t size;

    uint8_t reg_value;

    err = __hal_reg_get(adf4382, reg, &reg_value);
    ADI_CMS_ERROR_RETURN(err);
    err = __hal_reg_set(adf4382, reg, (reg_value & ~mask) | ((value << offset) & mask));
    ADI_CMS_ERROR_RETURN(err);
    width = width < 8 ? 0 : width - (8 - offset);

    for (byte_offset = 1; byte_offset < reg_bytes; byte_offset++)
    {
        mask = (1 << width) - 1;
        err = __hal_reg_get(adf4382, reg + byte_offset, &reg_value);
        ADI_CMS_ERROR_RETURN(err);
        err = __hal_reg_set(adf4382, reg + byte_offset, (reg_value & ~mask) | ((value >> (8 * byte_offset- offset)) & mask));
        ADI_CMS_ERROR_RETURN(err);
        width = width < 8 ? 0 : width - 8;
    }

    return API_CMS_ERROR_OK;
}

static int32_t __hal_reg_get(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t *data)
{
    int32_t err;
    uint8_t in_data[] = {
        GET_ADDRESS_MSB(reg) | 0x80,
        GET_ADDRESS_LSB(reg),
        0
    };
    uint8_t out_data[SPI_IN_OUT_BUFF_SZ] = { 0 };
    adi_cms_hal_txn_config_t txn_config;

    __txn_config_init(&txn_config);

    err = adf4382->hal_info.spi_read(
        adf4382->hal_info.user_data,
        in_data,
        out_data,
        SPI_IN_OUT_BUFF_SZ,
        &txn_config);

    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    *data = out_data[2];

    return API_CMS_ERROR_OK;
}

static int32_t __hal_reg_set(adi_adf4382_device_t *adf4382, uint32_t reg, uint8_t data)
{
    int32_t err;
    uint8_t in_data[] = {
        GET_ADDRESS_MSB(reg),
        GET_ADDRESS_LSB(reg),
        data
    };

    adi_cms_hal_txn_config_t txn_config;

    __txn_config_init(&txn_config);

    err = adf4382->hal_info.spi_write(
        adf4382->hal_info.user_data,
        in_data,
        SPI_IN_OUT_BUFF_SZ,
        &txn_config);

    if (err != API_CMS_ERROR_OK) {
        return API_CMS_ERROR_SPI_XFER;
    }

    return API_CMS_ERROR_OK;
}

static int32_t __le_byte_index_get(uint8_t index, uint8_t size)
{
    return index;
}

static int32_t __be_byte_index_get(uint8_t index, uint8_t size)
{
    return size - 1 - index;
}

static void __txn_config_init(adi_cms_hal_txn_config_t* config)
{
    config->addr_len = 2;
    config->data_len = 1;
    config->stream_len = 0;
    config->mask = 0x00FFFFFF; // 24bit
}

int32_t adi_adf4382_hal_reg_tbl_set(adi_adf4382_device_t *adf4382, adi_cms_reg_data_t tbl[], uint32_t count)
{
    int32_t err = API_CMS_ERROR_ERROR;
    uint16_t num = 0;

    ADI_CMS_NULL_PTR_CHECK(adf4382);
    ADI_CMS_NULL_PTR_CHECK(adf4382->hal_info.spi_write);
    ADI_CMS_NULL_PTR_CHECK(tbl);

    for (num = 0; num < count; ++num) {
        err = __hal_reg_set(adf4382, tbl[num].reg, tbl[num].val);
        ADI_CMS_ERROR_RETURN(err);
    }
    return API_CMS_ERROR_OK;
}