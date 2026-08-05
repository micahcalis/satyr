#pragma once

#include "Core/SatyrCore.h"

typedef struct SyrShaderModule SyrShaderModule;

SyrResult SyrShaderModule_Initialize(const char* shaderPath,
    VkDevice device,
    SyrShaderModule** shaderModule);

VkShaderModule SyrShaderModule_GetModuleHandle(SyrShaderModule* shaderModule);
void SyrShaderModule_Destroy(SyrShaderModule* shaderModule);
