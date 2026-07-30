#include "Allocator.h"

typedef struct SyrAllocator
{
    VmaAllocator vmaHandle;
    VkDevice logicalDevice;
    VkDescriptorPool descriptorPool;
} SyrAllocator;

SyrResult SyrAllocator_CreateAllocator(SyrAllocator* allocator,
    SyrVulkInstance* vulkInstance,
    SyrDevice* device)
{
    VmaAllocatorCreateInfo createInfo = {0};
    createInfo.instance = SyrVulkInstance_GetInstanceHandle(vulkInstance);
    createInfo.physicalDevice = SyrDevice_GetPhysicalDeviceHandle(device);
    createInfo.device = SyrDevice_GetLogicalDeviceHandle(device);
    createInfo.vulkanApiVersion = SYR_VULKAN_VERSION;

    if (vmaCreateAllocator(&createInfo, &allocator->vmaHandle) != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create VMA allocator!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static const uint32_t SYR_DESCRIPTOR_COUNT = 1000;
static const VkDescriptorPoolSize SYR_DESCRIPTOR_POOL_SIZES[] = {
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SYR_DESCRIPTOR_COUNT},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SYR_DESCRIPTOR_COUNT},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SYR_DESCRIPTOR_COUNT},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SYR_DESCRIPTOR_COUNT},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, SYR_DESCRIPTOR_COUNT}};

SyrResult SyrAllocator_CreateDescriptorPool(SyrAllocator* allocator)
{
    VkDescriptorPoolCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = SYR_DESCRIPTOR_COUNT;
    createInfo.poolSizeCount = sizeof(SYR_DESCRIPTOR_POOL_SIZES) / sizeof(SYR_DESCRIPTOR_POOL_SIZES[0]);
    createInfo.pPoolSizes = SYR_DESCRIPTOR_POOL_SIZES;

    if (vkCreateDescriptorPool(allocator->logicalDevice,
            &createInfo,
            NULL,
            &allocator->descriptorPool)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Descriptor Pool!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAllocator_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice* device,
    SyrAllocator** allocator)
{
    if (device == NULL)
    {
        SYR_ERROR("Satyr Device uninitialized trying to create Allocator");
        return SYR_RESULT_VULKAN_FAILED;
    }

    *allocator = SYR_NEW(*allocator);
    (*allocator)->logicalDevice = SyrDevice_GetLogicalDeviceHandle(device);

    if (SyrAllocator_CreateAllocator(*allocator, vulkInstance, device) == SYR_RESULT_VULKAN_FAILED)
    {
        SyrAllocator_Destroy(*allocator);
        *allocator = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    if (SyrAllocator_CreateDescriptorPool(*allocator) == SYR_RESULT_VULKAN_FAILED)
    {
        SyrAllocator_Destroy(*allocator);
        *allocator = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrBufferAllocation* SyrAllocator_AllocateBuffer(const SyrBufferAllocParams params,
    SyrAllocator* allocator)
{
    SyrBufferAllocation* allocation = NULL;

    if (SyrBufferAllocation_Initialize(params,
            allocator->vmaHandle,
            &allocation)
        == SYR_RESULT_RUNTIME_ERROR)
    {
        SYR_ERROR("Failed to allocate Buffer, size of: %u", (uint32_t)params.size);
        return NULL;
    }

    return allocation;
}

VmaAllocator SyrAllocator_GetAllocatorHandle(SyrAllocator* allocator)
{
    return allocator->vmaHandle;
}

void SyrAllocator_Destroy(SyrAllocator* allocator)
{
    if (allocator == NULL)
        return;

    if (allocator->vmaHandle != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(allocator->vmaHandle);
    }

    if (allocator->descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(allocator->logicalDevice,
            allocator->descriptorPool,
            NULL);
    }

    free(allocator);
}
