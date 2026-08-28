#include "MasterDisc.h"

void SyrDiscAsset_DestroyAudioAssets(SyrDiscAsset* discAsset)
{
    if (discAsset == NULL)
        return;

    for (uint32_t i = 0; i < discAsset->discCount; i++)
    {
        SyrAudioAsset* audioAsset = discAsset->audioAssets[i];

        if (audioAsset != NULL)
        {
            SyrAudioAsset_Destroy(audioAsset);
            audioAsset = NULL;
        }
    }

    discAsset->discCount = 0;
}

void SyrDiscAsset_Destroy(SyrDiscAsset* discAsset,
    const bool destroyAudioAssets)
{
    if (discAsset == NULL)
        return;

    if (destroyAudioAssets)
    {
        SyrDiscAsset_DestroyAudioAssets(discAsset);
    }

    SYR_FREE(discAsset);
}

typedef struct SyrMasterDisc
{
    SyrBufferAllocation* discBufferAlloc;
    uint32_t discCount;
    SyrDevice* device;
    char albumName[32];
} SyrMasterDisc;

static SyrResult SyrMasterDisc_CreateDiscBufferAlloc(SyrMasterDisc* masterDisc,
    SyrAllocator* allocator,
    const size_t totalDiscSize)
{
    SyrBufferAllocParams allocParams = {0};
    allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO;
    allocParams.usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    allocParams.size = totalDiscSize;

    masterDisc->discBufferAlloc = SyrAllocator_AllocateBuffer(allocParams, allocator);

    if (masterDisc->discBufferAlloc == NULL)
    {
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrMasterDisc_Initialize(const uint32_t discCount,
    const size_t totalDiscSize,
    const char albumName[32],
    SyrAllocator* allocator,
    SyrDevice* device,
    SyrMasterDisc** masterDisc)
{
    if (discCount == 0)
    {
        SYR_ERROR("Can't create Master Disc with 0 discs for Album: %s!", albumName);
        return SYR_RESULT_FAILED;
    }

    if (totalDiscSize == 0)
    {
        SYR_ERROR("Can't create Master Disc with Size 0 for Album: %s", albumName);
        return SYR_RESULT_FAILED;
    }

    *masterDisc = SYR_NEW(*masterDisc);
    (*masterDisc)->discCount = discCount;
    (*masterDisc)->device = device;
    SYR_STR_COPY((*masterDisc)->albumName, albumName);

    if (SyrMasterDisc_CreateDiscBufferAlloc(*masterDisc, allocator, totalDiscSize) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Disc Buffer Album: %s", albumName);
        SyrMasterDisc_Destroy(*masterDisc);
        *masterDisc = NULL;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrMasterDisc_CopyMemory(SyrMasterDisc* masterDisc,
    SyrDiscAsset* discAsset,
    const size_t* songOffsets,
    const uint32_t songCount)
{
    size_t totalOffset = 0;
    uint8_t* discBufferMemory = (uint8_t*)masterDisc->discBufferAlloc->info.pMappedData;

    for (uint32_t i = 0; i < songCount; i++)
    {
        size_t songSize = songOffsets[i];
        float* targetMemory = discAsset->audioAssets[i]->pcmData;
        memcpy(targetMemory, discBufferMemory + totalOffset, songSize);
        totalOffset += songSize;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrMasterDisc_InvalidateMemory(SyrMasterDisc* masterDisc)
{
    if (vmaInvalidateAllocation(
            masterDisc->discBufferAlloc->allocator,
            masterDisc->discBufferAlloc->allocation,
            0,
            VK_WHOLE_SIZE)
        != VK_SUCCESS)
    {
        SYR_ERROR("VMA Failed to Invaliidate Master Disc memory for Album: %s", masterDisc->albumName);
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrMasterDisc_BurnAsset(SyrMasterDisc* masterDisc,
    SyrDiscAsset* discAsset,
    const size_t* songOffsets,
    const uint32_t songCount,
    const size_t totalSize)
{
    if (songCount > masterDisc->discCount)
    {
        SYR_ERROR("Song Count greater than Master Disc Count for Album: %s!", masterDisc->albumName);
        return SYR_RESULT_FAILED;
    }

    if (totalSize > masterDisc->discBufferAlloc->info.size)
    {
        SYR_ERROR("Total Burn Size is greater than Master Disc Allocation Size (Album: %s)!", masterDisc->albumName);
        return SYR_RESULT_FAILED;
    }

    if (songCount == 0)
    {
        SYR_ERROR("Can't Burn Disc with 0 Song Count (Album: %s)!", masterDisc->albumName);
        return SYR_RESULT_FAILED;
    }

    if (SyrMasterDisc_InvalidateMemory(masterDisc) != SYR_RESULT_SUCCESS)
    {
        return SYR_RESULT_FAILED;
    }

    if (SyrMasterDisc_CopyMemory(masterDisc,
            discAsset,
            songOffsets,
            songCount)
        != SYR_RESULT_SUCCESS)
    {
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrBufferAllocation* SyrMasterDisc_GetDiscBufferAlloc(SyrMasterDisc* masterDisc)
{
    return masterDisc->discBufferAlloc;
}

const char* SyrMasterDisc_GetAlbumName(const SyrMasterDisc* masterDisc)
{
    return masterDisc->albumName;
}

void SyrMasterDisc_Destroy(SyrMasterDisc* masterDisc)
{
    if (masterDisc == NULL)
        return;

    if (masterDisc->discBufferAlloc != NULL)
    {
        SyrBufferAllocation_Destroy(masterDisc->discBufferAlloc);
    }

    SYR_FREE(masterDisc);
}
