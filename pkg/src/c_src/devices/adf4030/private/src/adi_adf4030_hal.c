/*!
 * \brief     ADF4030 HAL Functionality
 *
 * \copyright copyright(c) 2024 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*============= I N C L U D E S ============*/
#include "adi_adf4030_hal.h"
#include "adi_adf4030_types.h"

/*============= D E F I N E S ==============*/
#define ADF4030_BUFF_SIZE_BYTES		    (3)
#define ADF4030_WRITE			        (0 << 15)
#define ADF4030_READ			        (1 << 15)
#define ADF4030_DEVICE_NUMBER(x)		(x << 9)

#define PTR_CHECK(ptr, err)          if (ptr == ADI_INVALID_POINTER) return err
#define NULL_CHECK(val)              if (val == ADI_INVALID_POINTER) return API_CMS_ERROR_NULL_PARAM
#define CHECK_ERR_OK(err)            if (err != API_CMS_ERROR_OK) return err
#define REG_OFFSET_GET(info)         ((uint8_t)(info >> 0))
#define REG_WIDTH_GET(info)          ((uint8_t)(info >> 8))
#define REG_BYTES_GET(width, offset) (((width + offset) >> 3) + (((width + offset) & 7) == 0 ? 0 : 1))

typedef int32_t(*byte_index_calc)(uint8_t index, uint8_t size);

/*============= D A T A ====================*/
static adi_cms_reg_data_t ADF4030_INIT_TBL[] = {
    {0x0066, 0x80},
    {0x0064, 0x1E},
    {0x0063, 0x1E},
    {0x0062, 0x4C},
    {0x0061, 0x05},
    {0x0060, 0x2B},
    {0x005F, 0x5D},
    {0x005E, 0x32},
    {0x005D, 0x10},
    {0x005C, 0x1E},
    {0x005B, 0xC9},
    {0x005A, 0x17},
    {0x0059, 0x49},
    {0x0058, 0x53},
    {0x0057, 0x45},
    {0x0056, 0x7D},
    {0x0055, 0x01},
    {0x0054, 0x90},
    {0x0053, 0x19},
    {0x0052, 0xE9},
    {0x0050, 0xE9},
    {0x004E, 0xE9},
    {0x004C, 0xE9},
    {0x004A, 0xE9},
    {0x0048, 0xE9},
    {0x0046, 0xE9},
    {0x0044, 0xE9},
    {0x0042, 0xE9},
    {0x0040, 0xE9},
    {0x003C, 0xFF},
    {0x0037, 0x62},
    {0x0035, 0x05},
    {0x0034, 0x24},
    {0x0033, 0x1D},
    {0x0032, 0x1D},
    {0x0031, 0x45},
    {0x0016, 0x0C},
    {0x0011, 0x1F},
    {0x0010, 0x1F},
    {0x000A, 0xA5},
};

/*============= C O D E ====================*/


static void __txn_config_init(adi_cms_hal_txn_config_t *config);
static int32_t __hal_reg_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t *data);
static int32_t __hal_reg_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t data);
static int32_t __le_byte_index_get(uint8_t index, uint8_t size);
static int32_t __be_byte_index_get(uint8_t index, uint8_t size);

int32_t adi_adf4030_hal_reg_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t *data);
int32_t adi_adf4030_hal_reg_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t data);


int32_t adi_adf4030_hal_reg_default_set(adi_adf4030_device_t *adf4030)
{
    int32_t err;
    uint16_t i;
    uint16_t tbl_count = 0;

    PTR_CHECK(adf4030, API_CMS_ERROR_INVALID_HANDLE_PTR);
    PTR_CHECK(adf4030->hal_info.spi_write, API_CMS_ERROR_INVALID_XFER_PTR);

    tbl_count = sizeof(ADF4030_INIT_TBL) / sizeof(ADF4030_INIT_TBL[0]);

    err = __hal_reg_set(adf4030, 0x00, 0x81);
    CHECK_ERR_OK(err);

    adf4030->hal_info.delay_us(NULL, 1000);
    CHECK_ERR_OK(err);

    err = __hal_reg_set(adf4030, 0x00, 0x18);
    CHECK_ERR_OK(err);
    err = __hal_reg_set(adf4030, 0x01, 0x00);
    CHECK_ERR_OK(err);

    for (i = 0; i < tbl_count; ++i) {
        err = __hal_reg_set(adf4030, ADF4030_INIT_TBL[i].reg, ADF4030_INIT_TBL[i].val);
        CHECK_ERR_OK(err);
    }

    err = __hal_reg_set(adf4030, 0x3C, 0x1F);
    CHECK_ERR_OK(err);

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4030_private_hal_bf_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint32_t info, uint8_t value[], uint8_t value_size_bytes)
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

    err = __hal_reg_get(adf4030, reg, &reg_value);
    CHECK_ERR_OK(err);

    mask = (1 << width) - 1;
    value[indexer(0, reg_bytes)] = (reg_value >> offset) & mask;
    width = width < 8 ? 0 : width - (8 - offset);

    for (byte_offset = 1; byte_offset < reg_bytes; byte_offset++) {
        err = __hal_reg_get(adf4030, reg + byte_offset, &reg_value);
        CHECK_ERR_OK(err);

        if (width > 0) {
            mask = (1 << width) - 1;
            value[indexer(byte_offset, reg_bytes)] = (reg_value >> offset) & mask;
            width = width < (8 - offset) ? 0 : width - (8 - offset);
        }
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4030_private_hal_bf_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint32_t info, uint64_t value)
{
    int32_t err;
    uint8_t offset = REG_OFFSET_GET(info);
    uint8_t byte_offset;

    uint8_t width = REG_WIDTH_GET(info);
    uint8_t reg_bytes = REG_BYTES_GET(width, offset);
    uint8_t mask = ((1 << width) - 1) << offset;

    uint8_t reg_value;

    PTR_CHECK(adf4030, API_CMS_ERROR_INVALID_HANDLE_PTR);
    PTR_CHECK(adf4030->hal_info.spi_read, API_CMS_ERROR_INVALID_XFER_PTR);
    PTR_CHECK(adf4030->hal_info.spi_write, API_CMS_ERROR_INVALID_XFER_PTR);

    err = __hal_reg_get(adf4030, reg, &reg_value);
    CHECK_ERR_OK(err);
    err = __hal_reg_set(adf4030, reg, (reg_value & ~mask) | ((value << offset) & mask));
    CHECK_ERR_OK(err);
    width = width < 8 ? 0 : width - (8 - offset);

    for (byte_offset = 1; byte_offset < reg_bytes; byte_offset++) {
        mask = (1 << width) - 1;
        err = __hal_reg_get(adf4030, reg + byte_offset, &reg_value);
        CHECK_ERR_OK(err);
        err = __hal_reg_set(adf4030, reg + byte_offset, (reg_value & ~mask) | ((value >> (8 * byte_offset - offset)) & mask));
        CHECK_ERR_OK(err);
        width = width < 8 ? 0 : width - 8;
    }

    return API_CMS_ERROR_OK;
}

int32_t adi_adf4030_hal_reg_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t *data)
{
    PTR_CHECK(adf4030, API_CMS_ERROR_INVALID_HANDLE_PTR);
    PTR_CHECK(adf4030->hal_info.spi_read, API_CMS_ERROR_INVALID_XFER_PTR);
    NULL_CHECK(data);

    return __hal_reg_get(adf4030, reg, data);
}

int32_t adi_adf4030_hal_reg_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t data)
{
    PTR_CHECK(adf4030, API_CMS_ERROR_INVALID_HANDLE_PTR);
    PTR_CHECK(adf4030->hal_info.spi_write, API_CMS_ERROR_INVALID_XFER_PTR);

    return __hal_reg_set(adf4030, reg, data);
}


static void __txn_config_init(adi_cms_hal_txn_config_t *config)
{
    config->addr_len = 2;
    config->data_len = 1;
    config->stream_len = 0;
    config->mask = 0x00FFFFFF; // 24bit
}

static int32_t __hal_reg_get(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t *data)
{
    int32_t err;
    uint8_t in_data[ADF4030_BUFF_SIZE_BYTES] = {0};
    uint8_t out_data[ADF4030_BUFF_SIZE_BYTES] = {0};
    uint16_t cmd = 0;
    adi_cms_hal_txn_config_t txn_config;

    cmd = ADF4030_READ | ADF4030_DEVICE_NUMBER(adf4030->dev_id) | reg;

    in_data[0] = cmd >> 8;
    in_data[1] = cmd & 0xFF;
    in_data[2] = 0;

    __txn_config_init(&txn_config);

    err = adf4030->hal_info.spi_read(adf4030->hal_info.user_data, in_data, out_data, ADF4030_BUFF_SIZE_BYTES, &txn_config);
    CHECK_ERR_OK(err);

    *data = out_data[ADF4030_BUFF_SIZE_BYTES - 1];
    return API_CMS_ERROR_OK;
}

static int32_t __hal_reg_set(adi_adf4030_device_t *adf4030, uint32_t reg, uint8_t data)
{
    int32_t err;
    uint8_t in_data[ADF4030_BUFF_SIZE_BYTES] = {0};
    uint16_t cmd = 0;
    adi_cms_hal_txn_config_t txn_config;

    cmd = ADF4030_WRITE | ADF4030_DEVICE_NUMBER(adf4030->dev_id) | reg;

    in_data[0] = cmd >> 8;
    in_data[1] = cmd & 0xFF;
    in_data[2] = data;

    __txn_config_init(&txn_config);

    err = adf4030->hal_info.spi_write(adf4030->hal_info.user_data, in_data, ADF4030_BUFF_SIZE_BYTES, &txn_config);
    CHECK_ERR_OK(err);

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
