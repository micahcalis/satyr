#pragma once

#include "Core/SatyrCore.h"

typedef enum
{
    SYR_AUDIO_ASSET_SAMPLE_MODE_MONO,
    SYR_AUDIO_ASSET_SAMPLE_MODE_STEREO
} SyrAudioAssetSampleMode;

typedef enum
{
    SYR_AUDIO_ASSET_FILE_TYPE_WAV,
    SYR_AUDIO_ASSET_FILE_TYPE_MP3,
    SYR_AUDIO_ASSET_FILE_TYPE_FLAC
} SyrAudioAssetFileType;

#define SYR_AUDIO_SAMPLE_RATE 48000
#define SYR_AUDIO_OUTPUT_CHANNELS 2

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
    SyrAudioAssetFileType fileType;
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

SyrResult SyrAudioAsset_Export(const SyrAudioAssetExportConfig* config,
    SyrAudioAsset** audioAsset);

uint32_t SyrAudioAsset_GetTotalSamples(const SyrAudioAsset* audioAsset, const SyrAudioAssetSampleMode sampleMode);
bool SyrAudioAsset_IsStereo(const SyrAudioAsset* audioAsset);

void SyrAudioAsset_Destroy(SyrAudioAsset* audioAsset);
