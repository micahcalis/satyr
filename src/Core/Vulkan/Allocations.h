#pragma once

#include "Core/SatyrCore.h"

#define VMA_VULKAN_VERSION 1004000
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#include "vk_mem_alloc.h"

typedef struct SyrBufferAllocParams
{
    VkDeviceSize size;
    VkBufferUsageFlags usageFlags;
    VmaMemoryUsage memoryFlags;
    VmaAllocationCreateFlags createFlags;
} SyrBufferAllocParams;

typedef struct SyrBufferAllocation
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VmaAllocator allocator;
} SyrBufferAllocation;

SyrResult SyrBufferAllocation_Initialize(const SyrBufferAllocParams params,
    VmaAllocator allocator,
    SyrBufferAllocation** allocation);

void SyrBufferAllocation_Destroy(SyrBufferAllocation* allocation);
