#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordPlayer/AudioAsset.h"
#include "Core/Vulkan/Allocator.h"

typedef struct SyrDiscAsset
{
    SyrAudioAsset** discAssets;
    uint32_t discCount;
    char albumName[32];
} SyrDiscAsset;

typedef struct SyrMasterDisc SyrMasterDisc;

SyrResult SyrMasterDisc_Initialize(const uint32_t discCount,
    const size_t totalDiscSize,
    const char albumName[32],
    SyrAllocator* allocator,
    SyrMasterDisc** masterDisc);

SyrResult SyrMasterDisc_BurnAsset(SyrMasterDisc* masterDisc, SyrDiscAsset* discData);
SyrBufferAllocation* SyrMasterDisc_GetDiscBufferAlloc(SyrMasterDisc* masterDisc);
const char* SyrMasterDisc_GetAlbumName(const SyrMasterDisc* masterDisc);
void SyrMasterDisc_Destroy(SyrMasterDisc* masterDisc);
