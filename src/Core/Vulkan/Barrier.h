#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocations.h"

typedef struct SyrBarrierBatch
{
    SyrList(VkBufferMemoryBarrier) barrierHandles;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
} SyrBarrierBatch;

typedef enum
{
    SYR_RESOURCE_ACTION_UNDEFINED,
    SYR_RESOURCE_ACTION_BUFFER_READ,
    SYR_RESOURCE_ACTION_BUFFER_WRITE,
    SYR_RESOURCE_ACTION_BUFFER_READ_WRITE,
    SYR_RESOURCE_ACTION_TRANSFER_WRITE,
    SYR_RESOURCE_ACTION_TRANSFER_READ
} SyrResourceAction;

SyrResult SyrBarrierBatch_Initialize(const SyrResourceAction previousAction,
    const SyrResourceAction targetAction,
    const uint32_t barrierCount,
    SyrBarrierBatch** barrierBatch);

void SyrBarrierBatch_AttachBuffer(SyrBarrierBatch* barrierBatch,
    const SyrBufferAllocation* buffer,
    const uint32_t index);

void SyrBarrierBatch_Destroy(SyrBarrierBatch* barrierBatch);
