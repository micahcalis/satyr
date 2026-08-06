#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocations.h"

#define SYR_DESCRIPTOR_SSBO_LIMIT 32

typedef enum
{
    SYR_BUFFER_TYPE_UNIFORM,
    SYR_BUFFER_TYPE_SSBO
} SyrBufferType;

typedef struct SyrDescriptor SyrDescriptor;

SyrResult SyrDescriptor_Initialize(VkDescriptorSetLayout layout,
    VkDescriptorSet set,
    const uint32_t ssboCount,
    VkDevice device,
    VkDescriptorPool pool,
    SyrDescriptor** descriptor);

void SyrDescriptor_WriteBuffer(SyrDescriptor* descriptor,
    SyrBufferAllocation* bufferAllocation,
    const uint32_t binding,
    const SyrBufferType bufferType,
    const size_t size,
    const size_t offset);

VkDescriptorSetLayout SyrDescriptor_GetLayout(SyrDescriptor* descriptor);
VkDescriptorSet SyrDescriptor_GetSet(SyrDescriptor* descriptor);
uint32_t SyrDescriptor_GetSSBOCount(SyrDescriptor* descriptor);
void SyrDescriptor_Destroy(SyrDescriptor* descriptor);
