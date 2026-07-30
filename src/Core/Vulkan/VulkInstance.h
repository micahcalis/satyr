#pragma once

#include "Core/SatyrCore.h"

typedef struct SyrVulkInstance SyrVulkInstance;

SyrResult SyrVulkInstance_Initialize(const SyrConfig* config, SyrVulkInstance** vulkInstance);
VkInstance SyrVulkInstance_GetInstanceHandle(SyrVulkInstance* vulkInstance);
void SyrVulkInstance_Destroy(SyrVulkInstance* vulkInstance);
