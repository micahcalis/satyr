#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/VulkInstance.h"

typedef enum
{
    SYR_QUEUE_PRIORITY_LEVEL_HIGH = 0,
    SYR_QUEUE_PRIORITY_LEVEL_MEDIUM = 1,
    SYR_QUEUE_PRIORITY_LEVEL_LOW = 2
} SyrQueuePriorityLevel;

#define SYR_QUEUE_PRIORITY_LEVELS 3

typedef struct SyrDevice SyrDevice;

SyrResult SyrDevice_Initialize(const SyrConfig* config,
    SyrVulkInstance* vulkInstance,
    SyrDevice** device);

VkPhysicalDevice SyrDevice_GetPhysicalDeviceHandle(const SyrDevice* device);
VkDevice SyrDevice_GetLogicalDeviceHandle(const SyrDevice* device);
uint32_t SyrDevice_GetComputeFamilyIndex(SyrDevice* device);
VkQueue SyrDevice_GetComputeQueue(SyrDevice* device, const SyrQueuePriorityLevel level);
void SyrDevice_WaitIdle(SyrDevice* device);
void SyrDevice_Destroy(SyrDevice* device);
