#include <stddef.h>

#include "vscphelperlogging.h"

int main(void)
{
    if (vscphelper_configureLogging(VSCP_HELPER_LOG_LEVEL_OFF,
                                    VSCP_HELPER_LOG_LEVEL_OFF,
                                    NULL, 0, 0) != 0) {
        return 1;
    }
    if (vscphelper_getLastLoggingError()[0] != '\0') {
        return 2;
    }
    return vscphelper_flushLogging();
}
