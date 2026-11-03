#ifndef BLE_LAMP_SERVICE_H
#define BLE_LAMP_SERVICE_H

#include "bcomdef.h"
#include "gatt.h"

#define BLE_LAMP_SERVICE_UUID_128 \
    0x01, 0x00, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, \
    0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BLE_LAMP_EVT_SWITCH = 0,
    BLE_LAMP_EVT_BRIGHTNESS,
    BLE_LAMP_EVT_COLOR,
    BLE_LAMP_EVT_MODE
} ble_lamp_event_t;

typedef void (*ble_lamp_service_cb_t)(uint8 event, uint8* data, uint16 len);

extern bStatus_t BleLamp_AddService(ble_lamp_service_cb_t callback);

#define BLE_LAMP_SERVICE_UUID_128 \
    0x01, 0x00, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, \
    0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF
#ifdef __cplusplus
}
#endif

#endif /* BLE_LAMP_SERVICE_H */
