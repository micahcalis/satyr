#pragma once

#include "System/RecordPlayer//AudioAsset.h"
#include "System/RecordPlayer/Vinyl.h"

typedef struct SyrVinyl
{
    SyrVinylMode mode;
    SyrVinylAssetOwnership ownership;
    SyrAudioAsset* audioAsset;
    uint64_t frameSegmentBegin;
    uint64_t frameSegmentEnd;
    char name[32];
} SyrVinyl;
