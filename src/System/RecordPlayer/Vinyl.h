#pragma once

#include "System/RecordPlayer/AudioAsset.h"

#define SYR_MAX_VINYLS 512

typedef SyrSlotId SyrVinylId;

typedef enum
{
    SYR_VINYL_MODE_LOOP,
    SYR_VINYL_MODE_PLAY_ONCE,
    SYR_VINYL_MODE_LOOP_SEGMENT,
    SYR_VINYL_MODE_PLAY_ONCE_SEGMENT
} SyrVinylMode;

typedef enum
{
    SYR_VINYL_ASSET_OWNERSHIP_STRICT,
    SYR_VINYL_ASSET_OWNERSHIP_RELAXED
} SyrVinylAssetOwnership;

typedef struct SyrVinylConfig
{
    SyrVinylMode mode;
    SyrVinylAssetOwnership ownership;
    SyrAudioAsset* audioAsset;
    uint64_t frameSegmentBegin;
    uint64_t frameSegmentEnd;
    char name[32];
} SyrVinylConfig;

typedef struct SyrVinyl SyrVinyl;

void SyrVinyl_SetAudioAsset(SyrVinyl* vinyl,
    SyrAudioAsset* audioAsset);

void SyrVinyl_SetSegment(SyrVinyl* vinyl,
    const uint32_t frameSegmentBegin,
    const uint32_t frameSegmentEnd);
