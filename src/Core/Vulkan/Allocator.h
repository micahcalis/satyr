#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Device.h"
#include "Core/Vulkan/Allocations.h"

#define VMA_VULKAN_VERSION 1004000
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#include "vk_mem_alloc.h"

typedef struct SyrAllocator SyrAllocator;

SyrResult SyrAllocator_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice* device,
    SyrAllocator** allocator);

SyrBufferAllocation* SyrAllocator_AllocateBuffer(const SyrBufferAllocParams params,
    SyrAllocator* allocator);

VmaAllocator SyrAllocator_GetAllocatorHandle(SyrAllocator* allocator);
void SyrAllocator_Destroy(SyrAllocator* allocator);
