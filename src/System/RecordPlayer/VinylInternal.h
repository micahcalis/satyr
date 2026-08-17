#pragma once

#include "System/RecordPlayer//AudioAsset.h"
#include "System/RecordPlayer/Vinyl.h"

typedef struct SyrVinyl
{
    SyrVinylId id;
    SyrVinylMode mode;
    SyrVinylAssetOwnership ownership;
    SyrAudioAsset* audioAsset;
    uint64_t frameSegment[2];
    char name[32];
} SyrVinyl;

static inline SyrVinylId SyrVinylId_Create(uint32_t index, uint32_t generation)
{
    return ((uint64_t)generation << 32) | (uint64_t)index;
}

static inline uint32_t SyrVinylId_GetIndex(SyrVinylId id)
{
    return (uint32_t)(id & 0xFFFFFFFF);
}

static inline uint32_t SyrVinylId_GetGeneration(SyrVinylId id)
{
    return (uint32_t)(id >> 32);
}
