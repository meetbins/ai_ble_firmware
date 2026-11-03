#ifndef BLE_LAMP_APP_H
#define BLE_LAMP_APP_H

#include "types.h"

#define BLE_LAMP_START_DEVICE_EVT   0x0001
#define BLE_LAMP_ADVERTISE_EVT      0x0002

extern void BleLamp_Init(uint8 task_id);
extern uint16 BleLamp_ProcessEvent(uint8 task_id, uint16 events);

#endif /* BLE_LAMP_APP_H */
