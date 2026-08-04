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

SyrResult SyrCommandBuffer_Begin(SyrCommandBuffer* commandBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer->commandBufferHandle, &beginInfo) != VK_SUCCESS)
    {
        SYR_ERROR("Failed to begin Command Buffer!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrCommandBuffer_End(SyrCommandBuffer* commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer->commandBufferHandle) != VK_SUCCESS)
    {
        SYR_ERROR("Failed to end Command Buffer!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    // TODO: Timeline sempahores submitting

    return SYR_RESULT_SUCCESS;
}

void SyrCommandBuffer_RecordBarrier(SyrCommandBuffer* commandBuffer, const SyrBarrier barrier)
{
    vkCmdPipelineBarrier(commandBuffer->commandBufferHandle,
        barrier.sourceStage,
        barrier.destinationStage,
        0,
        0,
        NULL,
        1,
        &barrier.barrierHandle,
        0,
        NULL);
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
