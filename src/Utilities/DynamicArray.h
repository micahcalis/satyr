#pragma once
#include <stdlib.h>
#include <string.h>
#include "Utilities/SatyrDebug.h"

typedef struct SyrListHeader
{
    size_t count;
    size_t capacity;
} SyrListHeader;

#define SyrList(type) type*
#define SyrList_Header(l) ((SyrListHeader*)(l) - 1)
#define SyrList_Count(l) ((l) ? SyrList_Header(l)->count : 0)
#define SyrList_Capacity(l) ((l) ? SyrList_Header(l)->capacity : 0)

#define SyrList_Free(l)                  \
    do                                   \
    {                                    \
        if (l)                           \
        {                                \
            SYR_FREE(SyrList_Header(l)); \
            (l) = NULL;                  \
        }                                \
    } while (0)

#define SyrList_Reserve(l, capacity)                                                      \
    do                                                                                    \
    {                                                                                     \
        if (SyrList_Capacity(l) < (capacity))                                             \
        {                                                                                 \
            (l) = SyrList_SetCapacity((l), (capacity), sizeof(*(l)), __FILE__, __LINE__); \
        }                                                                                 \
    } while (0)

#define SyrList_Push(l, value)                     \
    do                                             \
    {                                              \
        SyrList_MayGrow(l, 1);                     \
        (l)[SyrList_Header(l)->count++] = (value); \
    } while (0)

#define SyrList_PushRange(l, items, n)                                               \
    do                                                                               \
    {                                                                                \
        size_t __syr_n = (n);                                                        \
        if ((items) != NULL && __syr_n > 0)                                          \
        {                                                                            \
            SyrList_MayGrow((l), __syr_n);                                           \
            memcpy(&(l)[SyrList_Header(l)->count], (items), sizeof(*(l)) * __syr_n); \
            SyrList_Header(l)->count += __syr_n;                                     \
        }                                                                            \
    } while (0)

#define SyrList_MayGrow(l, n)                                                                \
    (((l) == NULL || SyrList_Count(l) + (n) > SyrList_Capacity(l))                           \
            ? ((l) = SyrList_GrowImplementation((l), (n), sizeof(*(l)), __FILE__, __LINE__)) \
            : 0)

#define SyrList_RemoveAt(l, index)                                                                                    \
    do                                                                                                                \
    {                                                                                                                 \
        if ((l) && (size_t)(index) < SyrList_Header(l)->count)                                                        \
        {                                                                                                             \
            size_t __syr_idx = (size_t)(index);                                                                       \
            SyrList_Header(l)->count--;                                                                               \
            if (__syr_idx < SyrList_Header(l)->count)                                                                 \
            {                                                                                                         \
                memmove(&(l)[__syr_idx], &(l)[__syr_idx + 1], (SyrList_Header(l)->count - __syr_idx) * sizeof(*(l))); \
            }                                                                                                         \
        }                                                                                                             \
    } while (0)

#define SyrList_Clear(l)                  \
    do                                    \
    {                                     \
        if (l)                            \
        {                                 \
            SyrList_Header(l)->count = 0; \
        }                                 \
    } while (0)

static inline void* SyrList_SetCapacity(void* list, size_t new_cap, size_t item_size, const char* file, int line)
{
    if (new_cap == 0)
        return list;

    size_t total_size = sizeof(SyrListHeader) + new_cap * item_size;
    SyrListHeader* old_header = list ? SyrList_Header(list) : NULL;

    SyrListHeader* new_header = (SyrListHeader*)SYR_REALLOC_DBG(old_header, total_size, file, line);
    if (!new_header)
        return list;

    if (!list)
    {
        new_header->count = 0;
    }

    new_header->capacity = new_cap;
    return (void*)(new_header + 1);
}

static inline void* SyrList_GrowImplementation(void* list, size_t increment, size_t item_size, const char* file, int line)
{
    size_t double_cap = list ? SyrList_Capacity(list) * 2 : 0;
    size_t min_cap = SyrList_Count(list) + increment;
    size_t new_cap = double_cap > min_cap ? double_cap : min_cap;
    if (new_cap < 8)
        new_cap = 8;

    return SyrList_SetCapacity(list, new_cap, item_size, file, line);
}
