#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Barrier.h"
#include "Core/Vulkan/TimelineSemaphore.h"
#include "Core/Vulkan/Descriptor.h"
#include "Core/Vulkan/Pipeline.h"

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

void SyrCommandBuffer_Reset(SyrCommandBuffer* commandBuffer);

void SyrCommandBuffer_RecordBarrierBatch(SyrCommandBuffer* commandBuffer, const SyrBarrierBatch* barrierBatch);

void SyrCommandBuffer_BindDescriptor(SyrCommandBuffer* commandBuffer,
    const SyrDescriptor* descriptor,
    const SyrPipeline* pipeline);

void SyrCommandBuffer_BindPipeline(SyrCommandBuffer* commandBuffer, const SyrPipeline* pipeline);
void SyrCommandBuffer_Dispatch(SyrCommandBuffer* commandBuffer, const uint32_t threadGroupCount);

SyrResult SyrCommandBuffer_CopyBuffer(SyrCommandBuffer* commandBuffer,
    SyrBufferAllocation* sourceBuffer,
    SyrBufferAllocation* destinationBuffer,
    const size_t destinationOffset);

void SyrCommandBuffer_Destroy(SyrCommandBuffer* commandBuffer);
