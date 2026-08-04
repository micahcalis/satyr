#include "Barrier.h"

static VkAccessFlagBits SyrBarrier_GetBufferAccessFlags(const SyrResourceAction action)
{
    switch (action)
    {
    case SYR_RESOURCE_ACTION_UNDEFINED:
        return 0;
    case SYR_RESOURCE_ACTION_BUFFER_READ:
        return VK_ACCESS_SHADER_READ_BIT;
    case SYR_RESOURCE_ACTION_BUFFER_WRITE:
        return VK_ACCESS_SHADER_WRITE_BIT;
    case SYR_RESOURCE_ACTION_BUFFER_READ_WRITE:
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case SYR_RESOURCE_ACTION_TRANSFER_READ:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case SYR_RESOURCE_ACTION_TRANSFER_WRITE:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    }
}

static VkPipelineStageFlagBits SyrBarrier_GetBufferPipelineStageFlags(const SyrResourceAction action)
{
    switch (action)
    {
    case SYR_RESOURCE_ACTION_UNDEFINED:
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case SYR_RESOURCE_ACTION_BUFFER_READ:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case SYR_RESOURCE_ACTION_BUFFER_WRITE:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case SYR_RESOURCE_ACTION_BUFFER_READ_WRITE:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case SYR_RESOURCE_ACTION_TRANSFER_READ:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case SYR_RESOURCE_ACTION_TRANSFER_WRITE:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
}

SyrBarrier SyrBarrier_Initialize(const SyrResourceAction previousAction,
    const SyrResourceAction targetAction,
    const SyrBufferAllocation* buffer)
{
    SyrBarrier barrier = {0};
    barrier.barrierHandle.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.barrierHandle.buffer = buffer->bufferHandle;
    barrier.barrierHandle.size = buffer->info.size;
    barrier.barrierHandle.offset = buffer->info.offset;

    barrier.barrierHandle.srcAccessMask = SyrBarrier_GetBufferAccessFlags(previousAction);
    barrier.sourceStage = SyrBarrier_GetBufferPipelineStageFlags(previousAction);
    barrier.barrierHandle.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.barrierHandle.dstAccessMask = SyrBarrier_GetBufferAccessFlags(targetAction);
    barrier.destinationStage = SyrBarrier_GetBufferPipelineStageFlags(targetAction);
    barrier.barrierHandle.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    return barrier;
}

VkBufferMemoryBarrier* SyrBarrier_GetBarrierHandle(SyrBarrier* barrier)
{
    return &barrier->barrierHandle;
}
