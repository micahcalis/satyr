#pragma once

#include "Core/SatyrCore.h"

typedef enum
{
    SYR_AUDIO_ASSET_SAMPLE_MODE_MONO = 1,
    SYR_AUDIO_ASSET_SAMPLE_MODE_STEREO = 2
} SyrAudioAssetSampleMode;

#define SYR_AUDIO_SAMPLE_RATE 48000
#define SYR_AUDIO_OUTPUT_CHANNELS 2
#define SYR_MA_FORMAT ma_format_f32

typedef struct SyrAudioAssetLoadConfig
{
    char* filePath;
    char name[64];
    uint32_t sampleRate;
    SyrAudioAssetSampleMode sampleMode;
} SyrAudioAssetLoadConfig;

typedef struct SyrAudioAssetExportConfig
{
    char* filePath;
    uint32_t sampleRate;
    SyrAudioAssetSampleMode sampleMode;
} SyrAudioAssetExportConfig;

typedef struct SyrAudioAsset
{
    float* pcmData;
    uint32_t sampleRate;
    uint8_t channels;
    uint64_t totalFrames;
    char name[64];
} SyrAudioAsset;

SyrResult SyrAudioAsset_Load(const SyrAudioAssetLoadConfig* config,
    SyrAudioAsset** audioAsset);

SyrResult SyrAudioAsset_ExportWAV(SyrAudioAsset* audioAsset,
    const SyrAudioAssetExportConfig* config);

uint64_t SyrAudioAsset_GetTotalSamples(const SyrAudioAsset* audioAsset, const SyrAudioAssetSampleMode sampleMode);
bool SyrAudioAsset_IsStereo(const SyrAudioAsset* audioAsset);

void SyrAudioAsset_Destroy(SyrAudioAsset* audioAsset);
