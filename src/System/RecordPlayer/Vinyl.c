#include "Vinyl.h"
#include "VinylInternal.h"

void SyrVinyl_SetAudioAsset(SyrVinyl* vinyl,
    SyrAudioAsset* audioAsset)
{
    if (vinyl->ownership == SYR_VINYL_ASSET_OWNERSHIP_STRICT
        && vinyl->audioAsset != NULL)
    {
        SyrAudioAsset_Destroy(vinyl->audioAsset);
    }

    vinyl->audioAsset = audioAsset;
}

void SyrVinyl_SetSegment(SyrVinyl* vinyl,
    const uint64_t frameSegmentBegin,
    const uint64_t frameSegmentEnd)
{
    vinyl->frameSegment[0] = frameSegmentBegin;
    vinyl->frameSegment[1] = frameSegmentEnd;
}

SyrVinylId SyrVinyl_GetId(const SyrVinyl* vinyl)
{
    return vinyl->id;
}
