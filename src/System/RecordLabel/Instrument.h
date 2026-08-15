#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocator.h"
#include "System/RecordPlayer/AudioAsset.h"

#define SYR_INSTRUMENT_SSBO_COUNT 2
#define SYR_MAX_INSTRUMENTS 14

enum SyrInstrumentSlot
{
    SYR_INSTRUMENT_SLOT_0 = 0,
    SYR_INSTRUMENT_SLOT_1 = 1,
    SYR_INSTRUMENT_SLOT_2 = 2,
    SYR_INSTRUMENT_SLOT_3 = 3,
    SYR_INSTRUMENT_SLOT_4 = 4,
    SYR_INSTRUMENT_SLOT_5 = 5,
    SYR_INSTRUMENT_SLOT_6 = 6,
    SYR_INSTRUMENT_SLOT_7 = 7,
    SYR_INSTRUMENT_SLOT_8 = 8,
    SYR_INSTRUMENT_SLOT_9 = 9,
    SYR_INSTRUMENT_SLOT_10 = 10,
    SYR_INSTRUMENT_SLOT_11 = 11,
    SYR_INSTRUMENT_SLOT_12 = 12,
    SYR_INSTRUMENT_SLOT_13 = 13
};

typedef struct SyrTimeAudioSample
{
    float sample;
} SyrTimeAudioSample;

typedef struct SyrFrequencyAudioSample
{
    float real;
    float imaginary;
} SyrFrequencyAudioSample;

typedef struct SyrAudioBuffer
{
    SyrBufferAllocation* timeAllocation;
    SyrBufferAllocation* frequencyAllocation;
    uint32_t samples;
} SyrAudioBuffer;

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

const SyrAudioBuffer* SyrInstrument_GetAudioBuffer(const SyrInstrument* instrument);
const char* SyrInstrument_GetName(const SyrInstrument* instrument);

void SyrInstrument_Destroy(SyrInstrument* instrument);
