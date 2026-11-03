#include "OSAL.h"
#include "OSAL_PwrMgr.h"

int app_main(void)
{
    osal_init_system();
    osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
    osal_start_system();
    return 0;
}
