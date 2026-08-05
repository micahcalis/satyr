#pragma once

#include "Core/SatyrCore.h"

#define SYR_DESCRIPTOR_SSBO_LIMIT 32

typedef struct SyrDescriptor SyrDescriptor;

SyrResult SyrDescriptor_Initialize(VkDescriptorSetLayout layout,
    VkDescriptorSet set,
    const uint32_t ssboCount,
    VkDevice device,
    VkDescriptorPool pool,
    SyrDescriptor** descriptor);

VkDescriptorSetLayout SyrDescriptor_GetLayout(SyrDescriptor* descriptor);
VkDescriptorSet SyrDescriptor_GetSet(SyrDescriptor* descriptor);
uint32_t SyrDescriptor_GetSSBOCount(SyrDescriptor* descriptor);
void SyrDescriptor_Destroy(SyrDescriptor* descriptor);
