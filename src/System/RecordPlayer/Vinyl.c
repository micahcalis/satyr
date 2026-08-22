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
    const uint32_t frameSegmentBegin,
    const uint32_t frameSegmentEnd)
{
    vinyl->frameSegmentBegin = frameSegmentBegin;
    vinyl->frameSegmentEnd = frameSegmentEnd;
}
