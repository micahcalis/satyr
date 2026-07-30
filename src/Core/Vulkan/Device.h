#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/VulkInstance.h"

typedef struct SyrDevice SyrDevice;

SyrResult SyrDevice_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice** device);

VkDevice SyrDevice_GetDeviceHandle(const SyrDevice* device);
uint32_t SyrDevice_GetComputeFamilyIndex(SyrDevice* device);
void SyrDevice_Destroy(SyrDevice* device);
