#include "bcomdef.h"
#include "OSAL.h"
#include "gap.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "peripheral.h"
#include "ll.h"
#include "gpio.h"
#include "log.h"

#include "ble_lamp_app.h"
#include "ble_lamp_service.h"
#include "led_light.h"

#define BLE_LAMP_P2_PIN                        P2
#define BLE_LAMP_DEFAULT_BRIGHTNESS            50
#define BLE_LAMP_ADV_RESTART_MS                300

#define BLE_LAMP_MIN_CONN_INTERVAL              24
#define BLE_LAMP_MAX_CONN_INTERVAL              80
#define BLE_LAMP_SLAVE_LATENCY                  0
#define BLE_LAMP_CONN_TIMEOUT                   500
#define BLE_LAMP_CONN_PAUSE                     6

static uint8 s_task_id;
static uint8 s_is_on;
static uint8 s_brightness = BLE_LAMP_DEFAULT_BRIGHTNESS;
static uint8 s_rgb[3] = { 0xFF, 0xFF, 0xFF };
static uint8 s_last_nonzero_rgb[3] = { 0xFF, 0xFF, 0xFF };

static uint8 s_scan_rsp[] =
{
    0x09, GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'B', 'L', 'E', '-', 'L', 'A', 'M', 'P',
    0x11, GAP_ADTYPE_128BIT_COMPLETE,
    BLE_LAMP_SERVICE_UUID_128
};

/*
 * The GONENGO manufacturer payload keeps this lamp visible in the existing
 * legacy-BLE list in the companion app. The company ID is mandatory on air
 * and is stripped by the Flutter BLE plugin before manufacturerData is exposed.
 */
static uint8 s_advert_data[] =
{
    0x02, GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    0x0A, GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    0xFF, 0xFF,
    'G', 'O', 'N', 'E', 'N', 'G', 'O'
};

static uint8 s_device_name[GAP_DEVICE_NAME_LEN] = "BLE-LAMP";

static void BleLamp_ApplyOutput(void)
{
    uint16 level = 0;

    if (s_is_on)
    {
        level = (uint16)(((uint32)s_brightness * LIGHT_TURN_ON + 50) / 100);
        light_output_set_rgb(s_rgb[0], s_rgb[1], s_rgb[2]);
    }
    else
    {
        light_output_set_rgb(0, 0, 0);
    }

    light_output_set_level(level);
}

static void BleLamp_ServiceCallback(uint8 event, uint8* data, uint16 len)
{
    switch (event)
    {
    case BLE_LAMP_EVT_SWITCH:
        if (len >= 1)
        {
            s_is_on = (data[0] != 0) ? 1 : 0;
            data[0] = s_is_on;
            if (s_is_on && !(s_rgb[0] | s_rgb[1] | s_rgb[2]))
            {
                osal_memcpy(s_rgb, s_last_nonzero_rgb, sizeof(s_rgb));
            }
        }
        break;

    case BLE_LAMP_EVT_BRIGHTNESS:
        if (len >= 4)
        {
            s_brightness = data[3];
            if (s_brightness > 100)
            {
                s_brightness = 100;
                data[3] = s_brightness;
            }
        }
        break;

    case BLE_LAMP_EVT_COLOR:
        if (len >= 6)
        {
            s_rgb[0] = data[3];
            s_rgb[1] = data[4];
            s_rgb[2] = data[5];
            if (s_rgb[0] | s_rgb[1] | s_rgb[2])
            {
                osal_memcpy(s_last_nonzero_rgb, s_rgb, sizeof(s_rgb));
            }
        }
        break;

    case BLE_LAMP_EVT_MODE:
        /* Mode is reserved for future effects; solid colour is mode 0. */
        break;

    default:
        return;
    }

    BleLamp_ApplyOutput();
    LOG("[BLE LAMP] on=%d brightness=%d rgb=%d,%d,%d\n",
        s_is_on, s_brightness, s_rgb[0], s_rgb[1], s_rgb[2]);
}

static void BleLamp_ProcessOSALMsg(osal_event_hdr_t* msg)
{
    (void)msg;
}

static void BleLamp_GapStateCB(gaprole_States_t new_state)
{
    switch (new_state)
    {
    case GAPROLE_STARTED:
        LOG("[BLE LAMP] GAP started\n");
        osal_start_timerEx(s_task_id, BLE_LAMP_ADVERTISE_EVT, BLE_LAMP_ADV_RESTART_MS);
        break;

    case GAPROLE_ADVERTISING:
        LOG("[BLE LAMP] advertising\n");
        break;

    case GAPROLE_CONNECTED:
        LOG("[BLE LAMP] connected\n");
        break;

    case GAPROLE_WAITING:
    case GAPROLE_WAITING_AFTER_TIMEOUT:
        LOG("[BLE LAMP] disconnected, restarting advertising\n");
        osal_start_timerEx(s_task_id, BLE_LAMP_ADVERTISE_EVT, BLE_LAMP_ADV_RESTART_MS);
        break;

    case GAPROLE_ERROR:
        LOG("[BLE LAMP] GAP error\n");
        break;

    default:
        break;
    }
}

static void BleLamp_RssiCB(int8 rssi)
{
    (void)rssi;
}

static gapRolesCBs_t s_gap_role_cbs =
{
    BleLamp_GapStateCB,
    BleLamp_RssiCB
};

void BleLamp_Init(uint8 task_id)
{
    s_task_id = task_id;
    s_is_on = 0;

    light_output_init(BLE_LAMP_P2_PIN);

    GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, BLE_LAMP_CONN_PAUSE);

    {
        uint8 advertise = FALSE;
        uint8 update_request = TRUE;
        uint8 channels = GAP_ADVCHAN_37 | GAP_ADVCHAN_38 | GAP_ADVCHAN_39;
        uint8 adv_type = GAP_ADTYPE_ADV_IND;
        uint16 min_interval = BLE_LAMP_MIN_CONN_INTERVAL;
        uint16 max_interval = BLE_LAMP_MAX_CONN_INTERVAL;
        uint16 latency = BLE_LAMP_SLAVE_LATENCY;
        uint16 timeout = BLE_LAMP_CONN_TIMEOUT;

        GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(adv_type), &adv_type);
        GAPRole_SetParameter(GAPROLE_ADV_CHANNEL_MAP, sizeof(channels), &channels);
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(advertise), &advertise);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(s_advert_data), s_advert_data);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(s_scan_rsp), s_scan_rsp);
        GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(update_request), &update_request);
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(min_interval), &min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(max_interval), &max_interval);
        GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(latency), &latency);
        GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(timeout), &timeout);
    }

    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, s_device_name);

    GGS_AddService(GATT_ALL_SERVICES);
    GATTServApp_AddService(GATT_ALL_SERVICES);
    BleLamp_AddService(BleLamp_ServiceCallback);

    BleLamp_ApplyOutput();
    osal_set_event(s_task_id, BLE_LAMP_START_DEVICE_EVT);
}

uint16 BleLamp_ProcessEvent(uint8 task_id, uint16 events)
{
    (void)task_id;

    if (events & SYS_EVENT_MSG)
    {
        uint8* msg;

        while ((msg = osal_msg_receive(s_task_id)) != NULL)
        {
            BleLamp_ProcessOSALMsg((osal_event_hdr_t*)msg);
            (void)osal_msg_deallocate(msg);
        }

        return events ^ SYS_EVENT_MSG;
    }

    if (events & BLE_LAMP_START_DEVICE_EVT)
    {
        (void)GAPRole_StartDevice(&s_gap_role_cbs);
        return events ^ BLE_LAMP_START_DEVICE_EVT;
    }

    if (events & BLE_LAMP_ADVERTISE_EVT)
    {
        uint8 advertise = TRUE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(advertise), &advertise);
        return events ^ BLE_LAMP_ADVERTISE_EVT;
    }

    return 0;
}
