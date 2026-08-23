#include "MasterDisc.h"

typedef struct SyrMasterDisc
{
    SyrBufferAllocation* discBufferAlloc;
    uint32_t discCount;
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

SyrResult SyrMasterDisc_BurnAsset(SyrMasterDisc* masterDisc, SyrDiscAsset* discData)
{
    // trasnfer readback gpu buffer into asset
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
