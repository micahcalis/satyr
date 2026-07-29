#include "Device.h"
#include "Core/SatyrCore.h"

typedef struct SyrDevice
{
} SyrDevice;

SyrResult SyrDevice_Initialize(const SyrConfig* config, SyrDevice** device)
{
    *device = SYR_NEW(*device);

    return SYR_RESULT_SUCCESS;
}

void SyrDevice_Destroy(SyrDevice* device)
{
    free(device);
}
