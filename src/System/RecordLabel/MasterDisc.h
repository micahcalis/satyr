#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordPlayer/AudioAsset.h"
#include "Core/Vulkan/Allocator.h"

#define SYR_MAX_DISC_SONGS 64

typedef struct SyrDiscAsset
{
    SyrAudioAsset* audioAssets[SYR_MAX_DISC_SONGS];
    uint32_t discCount;
    char albumName[32];
} SyrDiscAsset;

void SyrDiscAsset_DestroyAudioAssets(SyrDiscAsset* discAsset);

void SyrDiscAsset_Destroy(SyrDiscAsset* discAsset,
    const bool destroyAudioAssets);

typedef struct SyrMasterDisc SyrMasterDisc;

SyrResult SyrMasterDisc_Initialize(const uint32_t discCount,
    const size_t totalDiscSize,
    const char albumName[32],
    SyrAllocator* allocator,
    SyrDevice* device,
    SyrMasterDisc** masterDisc);

SyrResult SyrMasterDisc_BurnAsset(SyrMasterDisc* masterDisc,
    SyrDiscAsset* discAsset,
    const size_t* songOffsets,
    const uint32_t songCount,
    const size_t totalSize);

SyrBufferAllocation* SyrMasterDisc_GetDiscBufferAlloc(SyrMasterDisc* masterDisc);
const char* SyrMasterDisc_GetAlbumName(const SyrMasterDisc* masterDisc);
void SyrMasterDisc_Destroy(SyrMasterDisc* masterDisc);
