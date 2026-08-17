#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Device.h"

typedef struct SyrPipelineCache SyrPipelineCache;
typedef struct SyrPipeline SyrPipeline;

SyrResult SyrPipelineCache_Initialize(const SyrConfig* config,
    SyrDevice* device,
    SyrPipelineCache** pipelineCache);

void SyrPipelineCache_Destroy(SyrPipelineCache* pipelineCache);

SyrResult SyrPipeline_Initialize(const char* shaderPath,
    const uint32_t kernelIndex,
    VkDescriptorSetLayout setLayout,
    SyrDevice* device,
    SyrPipelineCache* pipelineCache,
    SyrPipeline** pipeline);

VkPipeline SyrPipeline_GetPipelineHandle(const SyrPipeline* pipeline);
VkPipelineLayout SyrPipeline_GetLayout(const SyrPipeline* pipeline);

void SyrPipeline_Destroy(SyrPipeline* pipeline);
