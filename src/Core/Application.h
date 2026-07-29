#pragma once
#include "SatyrCore.h"

typedef struct SyrApplication SyrApplication;
typedef struct SyrConfig
{
    bool initializeOnStartup;
} SyrConfig;

typedef enum
{
    SYR_RESULT_SUCCES = 0,
    SYR_RESULT_VULKAN_FAILED = 1
} SyrResult;

SyrResult SyrApplication_Initialize(const SyrConfig* config, SyrApplication** application);
void SyrApplication_Terminate(SyrApplication* application);

