#include "Allocator.h"

typedef struct SyrAllocator
{
    VmaAllocator vmaHandle;
    VkDevice device;
    VkDescriptorPool descriptorPool;
} SyrAllocator;

static SyrResult SyrAllocator_CreateAllocator(SyrAllocator* allocator,
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

static SyrResult SyrAllocator_CreateDescriptorPool(SyrAllocator* allocator)
{
    VkDescriptorPoolCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = SYR_DESCRIPTOR_COUNT;
    createInfo.poolSizeCount = sizeof(SYR_DESCRIPTOR_POOL_SIZES) / sizeof(SYR_DESCRIPTOR_POOL_SIZES[0]);
    createInfo.pPoolSizes = SYR_DESCRIPTOR_POOL_SIZES;

    if (vkCreateDescriptorPool(allocator->device,
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
    (*allocator)->device = SyrDevice_GetLogicalDeviceHandle(device);
    (*allocator)->vmaHandle = VK_NULL_HANDLE;
    (*allocator)->descriptorPool = VK_NULL_HANDLE;

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

static SyrResult SyrAllocator_CreateDescriptorLayout(const uint32_t ssboCount,
    SyrAllocator* allocator,
    VkDescriptorSetLayout* layout)
{
    if (ssboCount >= SYR_DESCRIPTOR_SSBO_LIMIT)
    {
        SYR_ERROR("SSBO Count is higher than max limit: 32!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    VkDescriptorSetLayoutBinding bindings[SYR_DESCRIPTOR_SSBO_LIMIT + 1];

    bindings[SYR_UNIFORM_SETTINGS_SLOT] = (VkDescriptorSetLayoutBinding){
        .binding = SYR_UNIFORM_SETTINGS_SLOT,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    bindings[SYR_STORAGE_SETTINGS_SLOT] = (VkDescriptorSetLayoutBinding){
        .binding = SYR_STORAGE_SETTINGS_SLOT,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    for (uint32_t i = SYR_SETTING_SSBO_COUNT; i < ssboCount + SYR_SETTING_SSBO_COUNT; i++)
    {
        bindings[i] = (VkDescriptorSetLayoutBinding){
            .binding = i,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = NULL};
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = ssboCount + SYR_SETTING_SSBO_COUNT;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(allocator->device, &layoutInfo, NULL, &(*layout))
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Descriptor Set Layout!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrAllocator_CreateDescriptorSet(VkDescriptorSetLayout layout,
    SyrAllocator* allocator,
    VkDescriptorSet* set)
{
    VkDescriptorSetAllocateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    createInfo.descriptorPool = allocator->descriptorPool;
    createInfo.descriptorSetCount = 1;
    createInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(allocator->device,
            &createInfo,
            &(*set))
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Descriptor Set!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrDescriptor* SyrAllocator_AllocateDescriptor(const uint32_t ssboCount,
    SyrAllocator* allocator)
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    if (SyrAllocator_CreateDescriptorLayout(ssboCount,
            allocator,
            &layout)
        != SYR_RESULT_SUCCESS)
    {
        return NULL;
    }

    VkDescriptorSet set = VK_NULL_HANDLE;

    if (SyrAllocator_CreateDescriptorSet(layout,
            allocator,
            &set)
        != SYR_RESULT_SUCCESS)
    {
        return NULL;
    }

    SyrDescriptor* descriptor = NULL;

    if (SyrDescriptor_Initialize(layout,
            set,
            ssboCount,
            allocator->device,
            allocator->descriptorPool,
            &descriptor)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to allocate Descriptor, SBBO count of: %u", (uint32_t)ssboCount);
        return NULL;
    }

    return descriptor;
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
        vkDestroyDescriptorPool(allocator->device,
            allocator->descriptorPool,
            NULL);
    }

    SYR_FREE(allocator);
}
