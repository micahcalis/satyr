#pragma once

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "vulkan/vulkan.h"
#include "System/Utilities/DynamicArray.h"

#define SYR_LOG(format, ...) printf("[Satyr] " format "\n", ##__VA_ARGS__)
#define SYR_ERROR(format, ...) fprintf(stderr, "[Satyr Error] " format "\n", ##__VA_ARGS__)
#define SYR_ALLOC(type) calloc(1, sizeof(type))
#define SYR_NEW(ptr) calloc(1, sizeof(*(ptr)))
#define SYR_ALLOC_ARRAY(type, count) calloc((count), sizeof(type))

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
