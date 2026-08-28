#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdatomic.h>
#include <math.h>
#include "vulkan/vulkan.h"

typedef enum
{
    SYR_RESULT_SUCCESS = 0,
    SYR_RESULT_FAILED = 1,
    SYR_RESULT_VULKAN_FAILED = 2,
    SYR_RESULT_WAITING = 3,
    SYR_RESULT_RUNTIME_ERROR = 4,
    SYR_RESULT_MINIAUDIO_FAILED = 5
} SyrResult;

typedef struct SyrConfig
{
    bool bootupOnStartup;
    char* pipelineCachePath;
    bool playbackStereoEnabled;
    bool overrideStandardSampleRate;
    uint32_t overrideSampleRate;
} SyrConfig;

#define SYR_VULKAN_VERSION VK_API_VERSION_1_4

// Implementation Settings (REMOVE ON RELEASE)

#define SYR_ENABLE_VULKAN_PIPELINE_CACHE
#define SYR_DEBUG

#include "Utilities/SatyrDebug.h"
#include "Utilities/DynamicArray.h"
#include "Utilities/SlotMap.h"
#include "Utilities/Math.h"

#define SYR_STR_COPY(dest, src) strncpy_s(dest, sizeof(dest), src, sizeof(dest) - 1)
