#include "CommandPool.h"

typedef struct SyrCommandPool
{
    VkCommandPool commandPoolHandle;
    VkDevice deviceHandle;
    VkQueue queueHandle;
} SyrCommandPool;

static SyrResult SyrCommandPool_CreateCommandPool(SyrCommandPool* commandPool,
    const uint32_t computeFamilyIndex)
{
    VkCommandPoolCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = computeFamilyIndex;

    if (vkCreateCommandPool(commandPool->deviceHandle,
            &createInfo,
            NULL,
            &commandPool->commandPoolHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to create Vulkan Command Pool!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrCommandPool_Initialize(VkDevice deviceHandle,
    VkQueue queueHandle,
    const uint32_t computeFamilyIndex,
    SyrCommandPool** commandPool)
{
    *commandPool = SYR_NEW(*commandPool);
    (*commandPool)->deviceHandle = deviceHandle;
    (*commandPool)->queueHandle = queueHandle;

    if (SyrCommandPool_CreateCommandPool(*commandPool, computeFamilyIndex) != SYR_RESULT_SUCCESS)
    {
        SyrCommandPool_Destroy(*commandPool);
        *commandPool = NULL;
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrCommandBuffer* SyrCommandPool_AllocateCommandBuffer(SyrCommandPool* commandPool)
{
    VkCommandBufferAllocateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    createInfo.commandPool = commandPool->commandPoolHandle;
    createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    createInfo.commandBufferCount = 1;

    VkCommandBuffer commandBufferHandle = VK_NULL_HANDLE;

    if (vkAllocateCommandBuffers(commandPool->deviceHandle,
            &createInfo,
            &commandBufferHandle)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to allocate Command Buffer from Pool");
        return NULL;
    }

    SyrCommandBuffer* commandBuffer = NULL;

    if (SyrCommandBuffer_Initialize(commandBufferHandle,
            commandPool->commandPoolHandle,
            commandPool->deviceHandle,
            commandPool->queueHandle,
            &commandBuffer)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to initialize Satyr Command Buffer Object");
        return NULL;
    }

    return commandBuffer;
}

void SyrCommandPool_Destroy(SyrCommandPool* commandPool)
{
    if (commandPool == NULL)
        return;

    if (commandPool->commandPoolHandle != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(commandPool->deviceHandle,
            commandPool->commandPoolHandle,
            NULL);
    }

    free(commandPool);
}
