#include "CommandBuffer.h"

typedef struct SyrCommandBuffer
{
    VkCommandBuffer commandBufferHandle;
    VkCommandPool commandPoolHandle;
    VkDevice device;
    VkQueue computeQueue;
} SyrCommandBuffer;

SyrResult SyrCommandBuffer_Initialize(VkCommandBuffer commandBufferHandle,
    VkCommandPool pool,
    VkDevice device,
    VkQueue computeQueue,
    SyrCommandBuffer** commandBuffer)
{
    *commandBuffer = SYR_NEW(*commandBuffer);
    (*commandBuffer)->commandBufferHandle = commandBufferHandle;
    (*commandBuffer)->commandPoolHandle = pool;
    (*commandBuffer)->device = device;
    (*commandBuffer)->computeQueue = computeQueue;

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

SyrResult SyrCommandBuffer_EndSubmit(SyrCommandBuffer* commandBuffer,
    SyrTimelineSemaphore* timelineSemaphore,
    const SyrTimelineTicket* timelineTicket)
{
    if (vkEndCommandBuffer(commandBuffer->commandBufferHandle) != VK_SUCCESS)
    {
        SYR_ERROR("Failed to end Command Buffer!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    VkTimelineSemaphoreSubmitInfo timelineInfo = SyrTimelineSemaphore_GetSubmitInfo(timelineTicket);
    VkSemaphore semaphoreHandle = SyrTimelineSemaphore_GetSemaphoreHandle(timelineSemaphore);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->commandBufferHandle;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphoreHandle;
    submitInfo.pNext = &timelineInfo;

    if (vkQueueSubmit(commandBuffer->computeQueue,
            1,
            &submitInfo,
            NULL)
        != VK_SUCCESS)
    {
        SYR_ERROR("Failed to submit Command Buffer!");
        return SYR_RESULT_VULKAN_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

void SyrCommandBuffer_Reset(SyrCommandBuffer* commandBuffer)
{
    vkResetCommandBuffer(commandBuffer->commandBufferHandle, 0);
}

void SyrCommandBuffer_RecordBarrierBatch(SyrCommandBuffer* commandBuffer, const SyrBarrierBatch* barrierBatch)
{
    if (SyrList_Count(barrierBatch->barrierHandles) == 0)
    {
        SYR_ERROR("Cant record Barrier Batch on Command Buffer with 0 Barriers!");
        return;
    }

    vkCmdPipelineBarrier(commandBuffer->commandBufferHandle,
        barrierBatch->sourceStage,
        barrierBatch->destinationStage,
        0,
        0,
        NULL,
        (uint32_t)SyrList_Count(barrierBatch->barrierHandles),
        barrierBatch->barrierHandles,
        0,
        NULL);
}

void SyrCommandBuffer_BindDescriptor(SyrCommandBuffer* commandBuffer,
    const SyrDescriptor* descriptor,
    const SyrPipeline* pipeline)
{
    VkDescriptorSet descriptorSet = SyrDescriptor_GetSet(descriptor);

    vkCmdBindDescriptorSets(commandBuffer->commandBufferHandle,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        SyrPipeline_GetLayout(pipeline),
        0,
        1,
        &descriptorSet,
        0,
        NULL);
}

void SyrCommandBuffer_BindPipeline(SyrCommandBuffer* commandBuffer, const SyrPipeline* pipeline)
{
    vkCmdBindPipeline(commandBuffer->commandBufferHandle,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        SyrPipeline_GetPipelineHandle(pipeline));
}

void SyrCommandBuffer_Dispatch(SyrCommandBuffer* commandBuffer, const uint32_t threadGroupCount)
{
    if (threadGroupCount == 0)
    {
        SYR_ERROR("Can't Dispatch Compute Kernel with 0 Thread Group Count!");
        return;
    }

    vkCmdDispatch(commandBuffer->commandBufferHandle,
        threadGroupCount,
        1,
        1);
}

SyrResult SyrCommandBuffer_CopyBuffer(SyrCommandBuffer* commandBuffer,
    SyrBufferAllocation* sourceBuffer,
    SyrBufferAllocation* destinationBuffer,
    const size_t copySize,
    const size_t destinationOffset)
{
    if ((destinationOffset + copySize) > destinationBuffer->info.size)
    {
        SYR_ERROR("Buffer copy exceeds destination buffer bounds!");
        return SYR_RESULT_FAILED;
    }

    if (copySize > sourceBuffer->info.size)
    {
        SYR_ERROR("Buffer copy exceeds source buffer bounds!");
        return SYR_RESULT_FAILED;
    }

    if (copySize == 0)
    {
        SYR_ERROR("Can't Buffer Copy with Size 0!");
        return SYR_RESULT_FAILED;
    }

    VkBufferCopy bufferCopy = {0};
    bufferCopy.size = copySize;
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = destinationOffset;

    vkCmdCopyBuffer(commandBuffer->commandBufferHandle,
        sourceBuffer->bufferHandle,
        destinationBuffer->bufferHandle,
        1,
        &bufferCopy);

    return SYR_RESULT_SUCCESS;
}

void SyrCommandBuffer_PushConstants(SyrCommandBuffer* commandBuffer,
    SyrPipeline* pipeline,
    const void* data,
    const size_t size)
{
    vkCmdPushConstants(commandBuffer->commandBufferHandle,
        SyrPipeline_GetLayout(pipeline),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        size,
        data);
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

    SYR_FREE(commandBuffer);
}
