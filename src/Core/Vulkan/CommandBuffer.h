#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Barrier.h"

typedef struct SyrCommandBuffer SyrCommandBuffer;

SyrResult SyrCommandBuffer_Initialize(VkCommandBuffer commandBufferHandle,
    VkCommandPool pool,
    VkDevice device,
    SyrCommandBuffer** commandBuffer);

SyrResult SyrCommandBuffer_Begin(SyrCommandBuffer* commandBuffer);
SyrResult SyrCommandBuffer_End(SyrCommandBuffer* commandBuffer);
void SyrCommandBuffer_RecordBarrier(SyrCommandBuffer* commandBuffer, const SyrBarrier barrier);
void SyrCommandBuffer_Destroy(SyrCommandBuffer* commandBuffer);
