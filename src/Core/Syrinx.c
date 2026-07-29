#include "Syrinx.h"
#include "SatyrCore.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkInstance.h"

typedef struct SyrSyrinx
{
    SyrVulkInstance* instance;
    SyrDevice* device;
} SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config)
{
    SyrSyrinx* syrinx = SYR_ALLOC(SyrSyrinx);
    return syrinx;
}

SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config)
{
    if (SyrVulkInstance_Initialize(config, &syrinx->instance) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrDevice_Initialize(config, &syrinx->device) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static void SyrSyrinx_CleanupVulkan(SyrSyrinx* syrinx)
{
    SyrVulkInstance_Destroy(syrinx->instance);
    SyrDevice_Destroy(syrinx->device);
}

void SyrSyrinx_Destroy(SyrSyrinx* syrinx)
{
    if (syrinx == NULL)
        return;

    SyrSyrinx_CleanupVulkan(syrinx);
    free(syrinx);
}
