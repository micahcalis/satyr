#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Device.h"
#include "Core/Vulkan/ShaderModule.h"

typedef struct SyrPipelineCache SyrPipelineCache;
typedef struct SyrPipeline SyrPipeline;

SyrResult SyrPipelineCache_Initialize(const SyrConfig* config,
    SyrDevice* device,
    SyrPipelineCache** pipelineCache);

void SyrPipelineCache_Destroy(SyrPipelineCache* pipelineCache);

