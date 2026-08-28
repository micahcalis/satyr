#pragma once

#include "Core/SatyrCore.h"

#define SYR_INVALID_SLOT_ID 0
#define SYR_SLOT_FREE_END 0xFFFFFFFF

typedef uint64_t SyrSlotId;

static inline SyrSlotId SyrSlotId_Create(uint32_t index, uint32_t generation)
{
    return ((uint64_t)generation << 32) | (uint64_t)index;
}

static inline uint32_t SyrSlotId_GetIndex(SyrSlotId id)
{
    return (uint32_t)(id & 0xFFFFFFFF);
}

static inline uint32_t SyrSlotId_GetGeneration(SyrSlotId id)
{
    return (uint32_t)(id >> 32);
}
typedef struct SyrSlot
{
    uint32_t denseIndex;
    uint32_t generation;
} SyrSlot;

#define SYR_DEFINE_SLOT_MAP(type, name, maxCount)                                   \
    static const uint32_t name##_Max = maxCount;                                    \
                                                                                    \
    typedef struct name                                                             \
    {                                                                               \
        type dense[maxCount];                                                       \
        uint32_t denseToSlot[maxCount];                                             \
        SyrSlot slots[maxCount];                                                    \
        uint32_t denseCount;                                                        \
        uint32_t freeHead;                                                          \
    } name;                                                                         \
                                                                                    \
    static inline void name##_Initialize(name* s)                                   \
    {                                                                               \
        s->denseCount = 0;                                                          \
        s->freeHead = 0;                                                            \
        for (uint32_t i = 0; i < maxCount; i++)                                     \
        {                                                                           \
            s->slots[i].denseIndex = i + 1;                                         \
            s->slots[i].generation = 1;                                             \
        }                                                                           \
        s->slots[maxCount - 1].denseIndex = SYR_SLOT_FREE_END;                      \
    }                                                                               \
                                                                                    \
    static inline SyrSlotId name##_Insert(name* s, type item)                       \
    {                                                                               \
        if (s->freeHead == SYR_SLOT_FREE_END)                                       \
        {                                                                           \
            SYR_ERROR("Slot Map Limit Reached (%u) for " #type "!", maxCount);      \
            return SYR_INVALID_SLOT_ID;                                             \
        }                                                                           \
                                                                                    \
        uint32_t slotIndex = s->freeHead;                                           \
        SyrSlot* slot = &s->slots[slotIndex];                                       \
        s->freeHead = slot->denseIndex;                                             \
                                                                                    \
        uint32_t denseIndex = s->denseCount++;                                      \
        slot->denseIndex = denseIndex;                                              \
        s->denseToSlot[denseIndex] = slotIndex;                                     \
        s->dense[denseIndex] = item;                                                \
                                                                                    \
        return SyrSlotId_Create(slotIndex, slot->generation);                       \
    }                                                                               \
                                                                                    \
    static inline type* name##_Get(name* s, SyrSlotId id)                           \
    {                                                                               \
        if (id == SYR_INVALID_SLOT_ID)                                              \
            return NULL;                                                            \
                                                                                    \
        uint32_t slotIndex = SyrSlotId_GetIndex(id);                                \
        uint32_t generation = SyrSlotId_GetGeneration(id);                          \
        if (slotIndex >= maxCount)                                                  \
            return NULL;                                                            \
                                                                                    \
        SyrSlot* slot = &s->slots[slotIndex];                                       \
        if (slot->generation != generation)                                         \
            return NULL;                                                            \
                                                                                    \
        uint32_t denseIndex = slot->denseIndex;                                     \
        if (denseIndex >= s->denseCount || s->denseToSlot[denseIndex] != slotIndex) \
        {                                                                           \
            return NULL;                                                            \
        }                                                                           \
                                                                                    \
        return &s->dense[denseIndex];                                               \
    }                                                                               \
                                                                                    \
    static inline void name##_SwapAndPop(name* s, const uint32_t denseIndex)        \
    {                                                                               \
        uint32_t lastDenseIndex = --s->denseCount;                                  \
        if (denseIndex != lastDenseIndex)                                           \
        {                                                                           \
            s->dense[denseIndex] = s->dense[lastDenseIndex];                        \
            uint32_t lastItemSlotIndex = s->denseToSlot[lastDenseIndex];            \
            s->slots[lastItemSlotIndex].denseIndex = denseIndex;                    \
            s->denseToSlot[denseIndex] = lastItemSlotIndex;                         \
        }                                                                           \
    }                                                                               \
                                                                                    \
    static inline void name##_Recycle(name* s, const uint32_t slotIndex)            \
    {                                                                               \
        s->slots[slotIndex].generation++;                                           \
        s->slots[slotIndex].denseIndex = s->freeHead;                               \
        s->freeHead = slotIndex;                                                    \
    }                                                                               \
                                                                                    \
    static inline SyrResult name##_Remove(name* s, SyrSlotId id)                    \
    {                                                                               \
        if (id == SYR_INVALID_SLOT_ID)                                              \
            return SYR_RESULT_FAILED;                                               \
                                                                                    \
        uint32_t slotIndex = SyrSlotId_GetIndex(id);                                \
        uint32_t generation = SyrSlotId_GetGeneration(id);                          \
        if (slotIndex >= maxCount)                                                  \
            return SYR_RESULT_FAILED;                                               \
                                                                                    \
        SyrSlot* slot = &s->slots[slotIndex];                                       \
        if (slot->generation != generation)                                         \
            return SYR_RESULT_FAILED;                                               \
                                                                                    \
        uint32_t denseIndex = slot->denseIndex;                                     \
        if (denseIndex >= s->denseCount || s->denseToSlot[denseIndex] != slotIndex) \
        {                                                                           \
            return SYR_RESULT_FAILED;                                               \
        }                                                                           \
                                                                                    \
        name##_SwapAndPop(s, denseIndex);                                           \
        name##_Recycle(s, slotIndex);                                               \
        return SYR_RESULT_SUCCESS;                                                  \
    }
