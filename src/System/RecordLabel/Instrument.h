#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocator.h"
#include "System/RecordPlayer/AudioAsset.h"

#define SYR_INSTRUMENT_SSBO_COUNT 2

typedef struct SyrTimeAudioSample
{
    float sample;
} SyrTimeAudioSample;

typedef struct SyrFrequencyAudioSample
{
    float real;
    float imaginary;
} SyrFrequencyAudioSample;

typedef struct SyrAudioBuffer SyrAudioBuffer;

SyrResult SyrAudioBuffer_Initialize(const uint32_t samples,
    SyrAllocator* allocator,
    SyrAudioBuffer** audioBuffer);

void SyrAudioBuffer_Destroy(SyrAudioBuffer* audioBuffer);

typedef struct SyrInstrumentConfig
{
    uint32_t samples;
    const char name[32];
} SyrInstrumentConfig;

typedef struct SyrInstrument SyrInstrument;

SyrResult SyrInstrument_Initialize(const uint32_t samples,
    const char name[32],
    SyrAllocator* allocator,
    SyrInstrument** instrument);

SyrResult SyrInstrument_UploadAsset(SyrInstrument* instrument,
    const SyrAudioAsset* audioAsset);

void SyrInstrument_Destroy(SyrInstrument* instrument);
