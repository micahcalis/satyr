#pragma once

#include "Core/SatyrCore.h"

typedef struct SyrCommandBuffer SyrCommandBuffer;

SyrResult SyrCommandBuffer_Initialize(VkCommandBuffer commandBufferHandle,
    VkCommandPool pool,
    VkDevice device,
    SyrCommandBuffer** commandBuffer);

void SyrCommandBuffer_Destroy(SyrCommandBuffer* commandBuffer);
