#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocations.h"

typedef struct SyrBarrier
{
    VkBufferMemoryBarrier barrierHandle;
    VkPipelineStageFlagBits sourceStage;
    VkPipelineStageFlagBits destinationStage;
} SyrBarrier;

typedef enum
{
    SYR_RESOURCE_ACTION_UNDEFINED,
    SYR_RESOURCE_ACTION_BUFFER_READ,
    SYR_RESOURCE_ACTION_BUFFER_WRITE,
    SYR_RESOURCE_ACTION_BUFFER_READ_WRITE,
    SYR_RESOURCE_ACTION_TRANSFER_WRITE,
    SYR_RESOURCE_ACTION_TRANSFER_READ
} SyrResourceAction;

SyrBarrier SyrBarrier_Initialize(const SyrResourceAction previousAction,
    const SyrResourceAction targetAction,
    const SyrBufferAllocation* buffer);

VkBufferMemoryBarrier* SyrBarrier_GetBarrierHandle(SyrBarrier* barrier);
