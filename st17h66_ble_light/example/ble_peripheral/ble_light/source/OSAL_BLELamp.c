#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "ll.h"
#include "hci_tl.h"
#include "osal_cbTimer.h"
#include "l2cap.h"
#include "gap.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"
#include "gatt.h"
#include "gattservapp.h"
#include "peripheral.h"
#include "ble_lamp_app.h"

__ATTR_SECTION_SRAM__ const pTaskEventHandlerFn tasksArr[] =
{
    LL_ProcessEvent,
    HCI_ProcessEvent,
    OSAL_CBTIMER_PROCESS_EVENT(osal_CbTimerProcessEvent),
    L2CAP_ProcessEvent,
    SM_ProcessEvent,
    GAP_ProcessEvent,
    GATT_ProcessEvent,
    GAPRole_ProcessEvent,
    GATTServApp_ProcessEvent,
    BleLamp_ProcessEvent
};

__ATTR_SECTION_SRAM__ const uint8 tasksCnt = sizeof(tasksArr) / sizeof(tasksArr[0]);
uint16* tasksEvents;

void osalInitTasks(void)
{
    uint8 task_id = 0;

    tasksEvents = (uint16*)osal_mem_alloc(sizeof(uint16) * tasksCnt);
    osal_memset(tasksEvents, 0, sizeof(uint16) * tasksCnt);

    LL_Init(task_id++);
    HCI_Init(task_id++);
    osal_CbTimerInit(task_id);
    task_id += OSAL_CBTIMER_NUM_TASKS;
    L2CAP_Init(task_id++);
    SM_Init(task_id++);
    GAP_Init(task_id++);
    GATT_Init(task_id++);
    GAPRole_Init(task_id++);
    GATTServApp_Init(task_id++);
    BleLamp_Init(task_id++);
}
