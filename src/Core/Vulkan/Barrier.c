#include "Barrier.h"

static VkAccessFlags SyrBarrierBatch_GetBufferAccessFlags(const SyrResourceAction action)
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

static VkPipelineStageFlags SyrBarrierBatch_GetBufferPipelineStageFlags(const SyrResourceAction action, const bool isDestination)
{
    switch (action)
    {
    case SYR_RESOURCE_ACTION_BUFFER_READ:
    case SYR_RESOURCE_ACTION_BUFFER_WRITE:
    case SYR_RESOURCE_ACTION_BUFFER_READ_WRITE:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case SYR_RESOURCE_ACTION_TRANSFER_READ:
    case SYR_RESOURCE_ACTION_TRANSFER_WRITE:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case SYR_RESOURCE_ACTION_UNDEFINED:
        return isDestination ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
}

SyrResult SyrBarrierBatch_Initialize(const SyrResourceAction previousAction,
    const SyrResourceAction targetAction,
    const uint32_t barrierCount,
    SyrBarrierBatch** barrierBatch)
{
    if (barrierCount == 0)
    {
        SYR_ERROR("Can't Initialize Barrier with Count 0!");
        return SYR_RESULT_RUNTIME_ERROR;
    }

    *barrierBatch = SYR_NEW(*barrierBatch);
    (*barrierBatch)->barrierHandles = NULL;
    (*barrierBatch)->sourceStage = SyrBarrierBatch_GetBufferPipelineStageFlags(previousAction, false);
    (*barrierBatch)->destinationStage = SyrBarrierBatch_GetBufferPipelineStageFlags(targetAction, true);

    SyrList_Reserve((*barrierBatch)->barrierHandles, barrierCount);

    VkBufferMemoryBarrier baseBarrier = {0};
    baseBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    baseBarrier.srcAccessMask = SyrBarrierBatch_GetBufferAccessFlags(previousAction);
    baseBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    baseBarrier.dstAccessMask = SyrBarrierBatch_GetBufferAccessFlags(targetAction);
    baseBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    for (uint32_t i = 0; i < barrierCount; i++)
    {
        SyrList_Push((*barrierBatch)->barrierHandles, baseBarrier);
    }

    return SYR_RESULT_SUCCESS;
}

void SyrBarrierBatch_AttachBuffer(SyrBarrierBatch* barrierBatch,
    const SyrBufferAllocation* buffer,
    const uint32_t index)
{
    if (index >= SyrList_Count(barrierBatch->barrierHandles))
    {
        SYR_ERROR("Attach Buffer Index (%u) out of bounds for Barrier Count (%zu)!",
            index,
            SyrList_Count(barrierBatch->barrierHandles));

        return;
    }

    VkBufferMemoryBarrier* barrier = &barrierBatch->barrierHandles[index];
    barrier->buffer = buffer->bufferHandle;
    barrier->size = VK_WHOLE_SIZE;
    barrier->offset = 0;
}

void SyrBarrierBatch_Destroy(SyrBarrierBatch* barrierBatch)
{
    if (barrierBatch == NULL)
        return;

    if (barrierBatch->barrierHandles != NULL)
    {
        SyrList_Free(barrierBatch->barrierHandles);
    }

    SYR_FREE(barrierBatch);
}
