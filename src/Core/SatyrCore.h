#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "vulkan/vulkan.h"

typedef enum
{
    SYR_RESULT_SUCCESS = 0,
    SYR_RESULT_FAILED = 1,
    SYR_RESULT_VULKAN_FAILED = 2,
    SYR_RESULT_WAITING = 3,
    SYR_RESULT_RUNTIME_ERROR = 4
} SyrResult;

typedef struct SyrConfig
{
    bool bootupOnStartup;
    char* pipelineCachePath;
} SyrConfig;

#define SYR_VULKAN_VERSION VK_API_VERSION_1_4

// Implementation Settings (REMOVE ON RELEASE)

#define SYR_ENABLE_VULKAN_PIPELINE_CACHE
#define SYR_DEBUG

#include "Utilities/SatyrDebug.h"
#include "Utilities/DynamicArray.h"

#define SYR_STR_COPY(dest, src) strncpy_s(dest, sizeof(dest), src, sizeof(dest) - 1)
