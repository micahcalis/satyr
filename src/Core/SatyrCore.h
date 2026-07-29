#pragma once

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vulkan/vulkan.h"

#define SYR_LOG(format, ...) printf("[Satyr] " format "\n", ##__VA_ARGS__)
#define SYR_ERROR(format, ...) fprintf(stderr, "[Satyr Error] " format "\n", ##__VA_ARGS__)
#define SYR_ALLOC(type) calloc(1, sizeof(type))
#define SYR_NEW(ptr) calloc(1, sizeof(*(ptr)))
#define SYR_ALLOC_ARRAY(type, count) calloc((count), sizeof(type))

typedef enum
{
    SYR_RESULT_SUCCESS = 0,
    SYR_RESULT_VULKAN_FAILED = 1,
    SYR_RESULT_WAITING = 2
} SyrResult;

typedef struct SyrConfig
{
    bool bootupOnStartup;
} SyrConfig;
