#pragma once

#include "Core/SatyrCore.h"

typedef enum
{
    SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
    SYR_AUDIO_ASSET_SAMPLE_MODE_STEREO
} SyrAudioAssetSampleMode;

typedef struct SyrAudioAsset
{
    float* pcmData;
    uint32_t sampleRate;
    uint32_t channels;
    uint64_t totalFrames;
    char name[64];
} SyrAudioAsset;

SyrResult SyrAudioAsset_LoadWAV(const char* filePath,
    const char name[64],
    SyrAudioAsset** audioAsset);

SyrResult SyrAudioAsset_LoadMP3(const char* filePath,
    const char name[64],
    SyrAudioAsset** audioAsset);

SyrResult SyrAudioAsset_ExportWAV(const SyrAudioAsset* audioAsset, const char* filePath);
uint32_t SyrAudioAsset_GetTotalSamples(const SyrAudioAsset* audioAsset, const SyrAudioAssetSampleMode sampleMode);
bool SyrAudioAsset_IsStereo(const SyrAudioAsset* audioAsset);

void SyrAudioAsset_Destroy(SyrAudioAsset* audioAsset);
