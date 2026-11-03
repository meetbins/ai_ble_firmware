#include "bcomdef.h"
#include "OSAL.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "ble_lamp_service.h"

#define BLE_LAMP_ATTR_COUNT 9

/*
 * UUID byte order follows the BLE stack's little-endian 128-bit format.
 * The canonical UUIDs are documented in the project README.
 */
static CONST uint8 s_service_uuid[ATT_UUID_SIZE] =
{
    BLE_LAMP_SERVICE_UUID_128
};

static CONST uint8 s_switch_uuid[ATT_UUID_SIZE] =
{
    0x01, 0x01, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x01, 0x3A, 0x00, 0x00
};

static CONST uint8 s_brightness_uuid[ATT_UUID_SIZE] =
{
    0x01, 0x01, 0x9B, 0x5F, 0x81, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x01, 0x3A, 0x00, 0x00
};

static CONST uint8 s_color_uuid[ATT_UUID_SIZE] =
{
    0x01, 0x01, 0x9B, 0x5F, 0x82, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x01, 0x3A, 0x00, 0x00
};

static CONST uint8 s_mode_uuid[ATT_UUID_SIZE] =
{
    0x01, 0x01, 0x9B, 0x5F, 0x83, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x01, 0x3A, 0x00, 0x00
};

static CONST gattAttrType_t s_service = { ATT_UUID_SIZE, s_service_uuid };
static uint8 s_switch_props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8 s_brightness_props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8 s_color_props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;
static uint8 s_mode_props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

static uint8 s_switch_value[1] = { 0 };
static uint8 s_brightness_value[4] = { 0x00, 0x00, 0x00, 50 };
static uint8 s_color_value[6] = { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF };
static uint8 s_mode_value[2] = { 0x00, 0x00 };

static ble_lamp_service_cb_t s_callback = NULL;

static gattAttribute_t s_attr_table[BLE_LAMP_ATTR_COUNT] =
{
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8*)&s_service
    },
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &s_switch_props
    },
    {
        { ATT_UUID_SIZE, s_switch_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        s_switch_value
    },
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &s_brightness_props
    },
    {
        { ATT_UUID_SIZE, s_brightness_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        s_brightness_value
    },
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &s_color_props
    },
    {
        { ATT_UUID_SIZE, s_color_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        s_color_value
    },
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &s_mode_props
    },
    {
        { ATT_UUID_SIZE, s_mode_uuid },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        s_mode_value
    }
};

static uint8 BleLamp_ReadAttrCB(uint16 connHandle, gattAttribute_t* attr,
                                uint8* value, uint16* len, uint16 offset,
                                uint8 maxLen)
{
    uint16 value_len = 0;
    (void)connHandle;
    (void)maxLen;

    if (offset != 0)
    {
        return ATT_ERR_ATTR_NOT_LONG;
    }

    if (attr->pValue == s_switch_value)
    {
        value_len = sizeof(s_switch_value);
    }
    else if (attr->pValue == s_brightness_value)
    {
        value_len = sizeof(s_brightness_value);
    }
    else if (attr->pValue == s_color_value)
    {
        value_len = sizeof(s_color_value);
    }
    else if (attr->pValue == s_mode_value)
    {
        value_len = sizeof(s_mode_value);
    }
    else
    {
        *len = 0;
        return ATT_ERR_ATTR_NOT_FOUND;
    }

    *len = value_len;
    osal_memcpy(value, attr->pValue, value_len);
    return SUCCESS;
}

static bStatus_t BleLamp_WriteAttrCB(uint16 connHandle, gattAttribute_t* attr,
                                     uint8* value, uint16 len, uint16 offset)
{
    uint8 event;
    uint16 max_len;
    (void)connHandle;

    if (offset != 0)
    {
        return ATT_ERR_ATTR_NOT_LONG;
    }

    if (attr->pValue == s_switch_value)
    {
        event = BLE_LAMP_EVT_SWITCH;
        max_len = sizeof(s_switch_value);
    }
    else if (attr->pValue == s_brightness_value)
    {
        event = BLE_LAMP_EVT_BRIGHTNESS;
        max_len = sizeof(s_brightness_value);
    }
    else if (attr->pValue == s_color_value)
    {
        event = BLE_LAMP_EVT_COLOR;
        max_len = sizeof(s_color_value);
    }
    else if (attr->pValue == s_mode_value)
    {
        event = BLE_LAMP_EVT_MODE;
        max_len = sizeof(s_mode_value);
    }
    else
    {
        return ATT_ERR_ATTR_NOT_FOUND;
    }

    if ((len == 0) || (len > max_len))
    {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }

    osal_memset(attr->pValue, 0, max_len);
    osal_memcpy(attr->pValue, value, len);

    if (s_callback != NULL)
    {
        s_callback(event, attr->pValue, len);
    }

    return SUCCESS;
}

static CONST gattServiceCBs_t s_service_cbs =
{
    BleLamp_ReadAttrCB,
    BleLamp_WriteAttrCB,
    NULL
};

bStatus_t BleLamp_AddService(ble_lamp_service_cb_t callback)
{
    s_callback = callback;
    return GATTServApp_RegisterService(s_attr_table,
                                       GATT_NUM_ATTRS(s_attr_table),
                                       &s_service_cbs);
}
