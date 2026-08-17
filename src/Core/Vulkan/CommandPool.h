#pragma once

#include "Core/SatyrCore.h"
#include "CommandBuffer.h"

typedef struct SyrCommandPool SyrCommandPool;

SyrResult SyrCommandPool_Initialize(VkDevice deviceHandle,
    VkQueue queueHandle,
    const uint32_t computeFamilyIndex,
    SyrCommandPool** commandPool);

SyrCommandBuffer* SyrCommandPool_AllocateCommandBuffer(SyrCommandPool* commandPool);
void SyrCommandPool_Destroy(SyrCommandPool* commandPool);
