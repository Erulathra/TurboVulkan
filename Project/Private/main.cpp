#include <iostream>

#include "LoggingMacros.h"

int32_t main(int argc, char *argv[])
{
    TV_LOG(LOG_TEMP, LOG_ERROR, "This is error. Build: {}", BUILD_TYPE_STR);

    TV_LOG(LOG_TEMP, LOG_ERROR, "This is error");
    TV_LOG(LOG_TEMP, LOG_WARN, "This is warning");
    TV_LOG(LOG_TEMP, LOG_INFO, "This is info");

    return 0;
}
