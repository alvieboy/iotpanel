#include "error.h"
#include "ets_sys.h"

static const char *errorList[] = {
    "Success",
    "Out of bounds",
    "Invalid argument",
    "Invalid property",
    "Not found",
    "Internal Error",
    "Invalid Magic",
    "Invalid CRC",
    "Unknown",
    "Already exists",
    "Too many",
    "Malformed",
    "Invalid length",
    "Too big",
    "Not enough memory"
};

const char *ICACHE_FLASH_ATTR getErrorString(int errno)
{
    errno = -errno;
    if (errno<0 || errno>(int)(sizeof(errorList)/sizeof(errorList[0]))) {
        return "Unknown error";
    }
    return errorList[errno];
}