#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Device.h"
#include "Core/Vulkan/Allocations.h"
#include "Core/Vulkan/Descriptor.h"

#define VMA_VULKAN_VERSION 1004000
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#include "VMA/vk_mem_alloc.h"

typedef struct SyrAllocator SyrAllocator;

SyrResult SyrAllocator_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice* device,
    SyrAllocator** allocator);

SyrBufferAllocation* SyrAllocator_AllocateBuffer(const SyrBufferAllocParams params,
    SyrAllocator* allocator);

SyrDescriptor* SyrAllocator_AllocateDescriptor(const uint32_t ssboCount,
    SyrAllocator* allocator);

VmaAllocator SyrAllocator_GetAllocatorHandle(SyrAllocator* allocator);
void SyrAllocator_Destroy(SyrAllocator* allocator);
