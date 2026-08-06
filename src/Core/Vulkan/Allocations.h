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
    VkBuffer bufferHandle;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VmaAllocator allocator;
} SyrBufferAllocation;

SyrResult SyrBufferAllocation_Initialize(const SyrBufferAllocParams params,
    VmaAllocator allocator,
    SyrBufferAllocation** allocation);

SyrResult SyrBufferAllocation_Upload(SyrBufferAllocation* bufferAllocation,
    const void* data,
    const size_t size,
    const size_t offset);

void SyrBufferAllocation_Destroy(SyrBufferAllocation* allocation);
