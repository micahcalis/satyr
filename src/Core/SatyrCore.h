#pragma once

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vulkan/vulkan.h"

#define SYR_LOG(format, ...) printf("[Satyr] " format "\n", ##__VA_ARGS__)
#define SYR_ERROR(format, ...) fprintf(stderr, "[Satyr Error] " format "\n", ##__VA_ARGS__)

typedef enum
{
    SYR_RESULT_SUCCESS = 0,
    SYR_RESULT_VULKAN_FAILED = 1,
    SYR_RESULT_WAITING = 2
} SyrResult;
