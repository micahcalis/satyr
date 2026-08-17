#include "RecordPlayer.h"
#include "System/RecordPlayer/VinylInternal.h"

typedef struct SyrSlot
{
    uint32_t denseIndex;
    uint32_t generation;
} SyrSlot;

typedef struct SyrRecordPlayer
{
    SyrVinyl dense[SYR_MAX_VINYLS];
    uint32_t denseToSlot[SYR_MAX_VINYLS];
    SyrSlot slots[SYR_MAX_VINYLS];
    uint32_t denseCount;
    uint32_t freeHead;
} SyrRecordPlayer;

#define SYR_SLOT_FREE_END 0xFFFFFFFF

static inline void SyrRecordPlayer_InitializeSlotMap(SyrRecordPlayer* recordPlayer)
{
    recordPlayer->denseCount = 0;
    recordPlayer->freeHead = 0;

    for (int i = 0; i < SYR_MAX_VINYLS; i++)
    {
        recordPlayer->slots[i].denseIndex = i + 1;
        recordPlayer->slots[i].generation = 1;
    }

    recordPlayer->slots[SYR_MAX_VINYLS - 1].denseIndex = SYR_SLOT_FREE_END;
}

SyrResult SyrRecordPlayer_Initialize(SyrRecordPlayer** recordPlayer)
{
    *recordPlayer = SYR_NEW(*recordPlayer);
    SyrRecordPlayer_InitializeSlotMap(*recordPlayer);

    return SYR_RESULT_SUCCESS;
}

static inline SyrVinylId SyrRecordPlayer_GenerateNewSlotHandle(SyrRecordPlayer* recordPlayer)
{
    uint32_t slotIndex = recordPlayer->freeHead;
    SyrSlot* slot = &recordPlayer->slots[slotIndex];
    recordPlayer->freeHead = slot->denseIndex;

    uint32_t denseIndex = recordPlayer->denseCount;
    slot->denseIndex = denseIndex;
    recordPlayer->denseToSlot[denseIndex] = slotIndex;

    return SyrVinylId_Create(slotIndex, slot->generation);
}

SyrVinylId SyrRecordPlayer_CreateVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylConfig* config)
{
    if (recordPlayer->freeHead == SYR_SLOT_FREE_END)
    {
        SYR_ERROR("Record Player Vinyl Limit Reached: %u", SYR_MAX_VINYLS);
        return SYR_INVALID_VINYL_ID;
    }

    SyrVinylId id = SyrRecordPlayer_GenerateNewSlotHandle(recordPlayer);

    uint32_t slotIndex = SyrVinylId_GetIndex(id);
    uint32_t denseIndex = recordPlayer->slots[slotIndex].denseIndex;

    SyrVinyl* vinyl = &recordPlayer->dense[denseIndex];
    vinyl->id = id;
    vinyl->mode = config->mode;
    vinyl->ownership = config->ownership;
    vinyl->audioAsset = config->audioAsset;
    vinyl->frameSegment[0] = config->frameSegmentBegin;
    vinyl->frameSegment[1] = config->frameSegmentEnd;
    SYR_STR_COPY(vinyl->name, config->name);

    recordPlayer->denseCount++;
    return id;
}

SyrVinyl* SyrRecordPlayer_GetVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylId id)
{
    if (id == SYR_INVALID_VINYL_ID)
    {
        SYR_ERROR("Record Player can't get Vinyl with invalid ID!");
        return NULL;
    }

    uint32_t slotIndex = SyrVinylId_GetIndex(id);
    uint32_t generation = SyrVinylId_GetGeneration(id);

    if (slotIndex >= SYR_MAX_VINYLS)
    {
        SYR_ERROR("Record Player can't get Vinyl with Slot Index higher than limit: %u", SYR_MAX_VINYLS);
        return NULL;
    }

    SyrSlot* slot = &recordPlayer->slots[slotIndex];

    if (slot->generation != generation)
    {
        SYR_ERROR("Record Player can't get Vinyl with stale or deleted handle");
        return NULL;
    }

    return &recordPlayer->dense[slot->denseIndex];
}

static inline void SyrRecordPlayer_SwapAndPopSlot(SyrRecordPlayer* recordPlayer,
    const uint32_t denseIndex,
    const uint32_t lastDenseIndex)
{
    if (denseIndex != lastDenseIndex)
    {
        recordPlayer->dense[denseIndex] = recordPlayer->dense[lastDenseIndex];
        uint32_t lastItemSlotIndex = recordPlayer->denseToSlot[lastDenseIndex];
        recordPlayer->slots[lastItemSlotIndex].denseIndex = denseIndex;
        recordPlayer->denseToSlot[denseIndex] = lastItemSlotIndex;
    }
}

static inline void SyrRecordPlayer_RecycleSlot(SyrRecordPlayer* recordPlayer,
    const uint32_t slotIndex)
{
    recordPlayer->slots[slotIndex].generation++;
    recordPlayer->slots[slotIndex].denseIndex = recordPlayer->freeHead;
    recordPlayer->freeHead = slotIndex;
}

SyrResult SyrRecordPlayer_DestroyVinyl(SyrRecordPlayer* recordPlayer,
    SyrVinylId id)
{
    SyrVinyl* vinyl = SyrRecordPlayer_GetVinyl(recordPlayer, id);

    if (vinyl == NULL)
    {
        SYR_ERROR("Record Player can't destroy Vinyl that is already destroyed or doesn't exist!");
        return SYR_RESULT_FAILED;
    }

    uint32_t slotIndex = SyrVinylId_GetIndex(id);
    uint32_t denseIndex = recordPlayer->slots[slotIndex].denseIndex;
    uint32_t lastDenseIndex = recordPlayer->denseCount - 1;

    if (vinyl->ownership == SYR_VINYL_ASSET_OWNERSHIP_STRICT && vinyl->audioAsset != NULL)
    {
        SyrAudioAsset_Destroy(vinyl->audioAsset);
    }

    SyrRecordPlayer_SwapAndPopSlot(recordPlayer, denseIndex, lastDenseIndex);
    SyrRecordPlayer_RecycleSlot(recordPlayer, slotIndex);

    recordPlayer->denseCount--;
    return SYR_RESULT_SUCCESS;
}
