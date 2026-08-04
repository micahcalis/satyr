#include "CommandBuffer.h"

typedef struct SyrCommandBuffer
{
    VkCommandBuffer commandBufferHandle;
    VkCommandPool commandPoolHandle;
    VkDevice device;
} SyrCommandBuffer;

SyrResult SyrCommandBuffer_Initialize(VkCommandBuffer commandBufferHandle,
    VkCommandPool pool,
    VkDevice device,
    SyrCommandBuffer** commandBuffer)
{
    *commandBuffer = SYR_NEW(*commandBuffer);
    (*commandBuffer)->commandBufferHandle = commandBufferHandle;
    (*commandBuffer)->commandPoolHandle = pool;
    (*commandBuffer)->device = device;

    return SYR_RESULT_SUCCESS;
}

void SyrCommandBuffer_Destroy(SyrCommandBuffer* commandBuffer)
{
    if (commandBuffer == NULL)
        return;

    if (commandBuffer->commandBufferHandle != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(commandBuffer->device,
            commandBuffer->commandPoolHandle,
            1,
            &commandBuffer->commandBufferHandle);
    }

    free(commandBuffer);
}
