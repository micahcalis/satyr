#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Barrier.h"
#include "Core/Vulkan/TimelineSemaphore.h"

typedef struct SyrCommandBuffer SyrCommandBuffer;

SyrResult SyrCommandBuffer_Initialize(VkCommandBuffer commandBufferHandle,
    VkCommandPool pool,
    VkDevice device,
    VkQueue computeQueue,
    SyrCommandBuffer** commandBuffer);

SyrResult SyrCommandBuffer_Begin(SyrCommandBuffer* commandBuffer);

SyrResult SyrCommandBuffer_EndSubmit(SyrCommandBuffer* commandBuffer,
    SyrTimelineSemaphore* timelineSemaphore,
    const SyrTimelineTicket* timelineTicket);

void SyrCommandBuffer_RecordBarrierBatch(SyrCommandBuffer* commandBuffer, const SyrBarrierBatch*);

void SyrCommandBuffer_Destroy(SyrCommandBuffer* commandBuffer);
