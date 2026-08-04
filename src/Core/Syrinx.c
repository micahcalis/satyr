#include "Syrinx.h"
#include "SatyrCore.h"
#include "Vulkan/Device.h"
#include "Vulkan/VulkInstance.h"
#include "Vulkan/Allocator.h"

typedef struct SyrSyrinx
{
    SyrVulkInstance* vulkInstance;
    SyrDevice* device;
    SyrAllocator* allocator;
} SyrSyrinx;

SyrSyrinx* SyrSyrinx_Create(const SyrConfig* config)
{
    SyrSyrinx* syrinx = SYR_ALLOC(SyrSyrinx);
    return syrinx;
}

SyrResult SyrSyrinx_InitializeVulkan(SyrSyrinx* syrinx, const SyrConfig* config)
{
    if (SyrVulkInstance_Initialize(config, &syrinx->vulkInstance) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrDevice_Initialize(config, syrinx->vulkInstance, &syrinx->device) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrAllocator_Initialize(config, syrinx->vulkInstance, syrinx->device, &syrinx->allocator) == SYR_RESULT_VULKAN_FAILED)
    {
        return SYR_RESULT_VULKAN_FAILED;
    }

    // SyrBufferAllocParams allocParams = {0};
    // allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    //     | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    // allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO;
    // allocParams.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    // SyrBufferAllocation* bufferAllocation = SyrAllocator_AllocateBuffer(allocParams,
    //     syrinx->allocator);

    // SyrBufferAllocation_Destroy(bufferAllocation);

    // SyrDescriptor* descriptor = SyrAllocator_AllocateDescriptor(8,
    //     syrinx->allocator);

    // SyrDescriptor_Destroy(descriptor);

    return SYR_RESULT_SUCCESS;
}

static void SyrSyrinx_CleanupVulkan(SyrSyrinx* syrinx)
{
    SyrAllocator_Destroy(syrinx->allocator);
    SyrDevice_Destroy(syrinx->device);
    SyrVulkInstance_Destroy(syrinx->vulkInstance);
}

void SyrSyrinx_Destroy(SyrSyrinx* syrinx)
{
    if (syrinx == NULL)
        return;

    SyrSyrinx_CleanupVulkan(syrinx);
    free(syrinx);
}
