#include "Allocations.h"

SyrResult SyrBufferAllocation_Initialize(const SyrBufferAllocParams params,
    VmaAllocator allocator,
    SyrBufferAllocation** allocation)
{
    if (allocation == NULL)
    {
        SYR_ERROR("Provided allocation pointer is NULL");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    VkBufferCreateInfo bufferCreateInfo = {0};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = params.size;
    bufferCreateInfo.usage = params.usageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {0};
    allocCreateInfo.usage = params.memoryFlags;
    allocCreateInfo.flags = params.createFlags;

    *allocation = SYR_NEW(*allocation);
    (*allocation)->allocator = allocator;

    if (vmaCreateBuffer(allocator,
            &bufferCreateInfo,
            &allocCreateInfo,
            &(*allocation)->bufferHandle,
            &(*allocation)->allocation,
            &(*allocation)->info)
        != VK_SUCCESS)
    {
        SyrBufferAllocation_Destroy(*allocation);
        *allocation = NULL;
        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrBufferAllocation_Upload(SyrBufferAllocation* bufferAllocation,
    const void* data,
    const size_t size,
    const size_t offset)
{
    if (offset > bufferAllocation->info.size || size > bufferAllocation->info.size - offset)
    {
        SYR_ERROR("Buffer Upload Overflow: size %zu + offset %zu > allocation size %zu",
            size,
            offset,
            bufferAllocation->info.size);

        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (bufferAllocation->info.pMappedData == NULL)
    {
        SYR_ERROR("Cannot directly upload to unmapped GPU memory!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    void* destination = (char*)bufferAllocation->info.pMappedData + offset;
    memcpy(destination, data, size);

    return SYR_RESULT_SUCCESS;
}

void SyrBufferAllocation_Destroy(SyrBufferAllocation* allocation)
{
    if (allocation == NULL)
        return;

    if (allocation->bufferHandle != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(allocation->allocator, allocation->bufferHandle, allocation->allocation);
    }

    free(allocation);
}
