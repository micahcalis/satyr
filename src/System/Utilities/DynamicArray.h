#pragma once
#include <stdlib.h>
#include <string.h>

typedef struct SyrListHeader
{
    size_t count;
    size_t capacity;
} SyrListHeader;

#define SyrList(type) type*
#define SyrList_Header(l) ((SyrListHeader*)(l) - 1)
#define SyrList_Count(l) ((l) ? SyrList_Header(l)->count : 0)
#define SyrList_Capacity(l) ((l) ? SyrList_Header(l)->capacity : 0)

#define SyrList_Free(l)              \
    do                               \
    {                                \
        if (l)                       \
        {                            \
            free(SyrList_Header(l)); \
            (l) = NULL;              \
        }                            \
    } while (0)

#define SyrList_Reserve(l, capacity)                                  \
    do                                                                \
    {                                                                 \
        if (SyrList_Capacity(l) < (capacity))                         \
        {                                                             \
            (l) = SyrList_SetCapacity((l), (capacity), sizeof(*(l))); \
        }                                                             \
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

#define SyrList_MayGrow(l, n)                                            \
    (((l) == NULL || SyrList_Count(l) + (n) > SyrList_Capacity(l))       \
            ? ((l) = SyrList_GrowImplementation((l), (n), sizeof(*(l)))) \
            : 0)

#define SyrList_Clear(l)                  \
    do                                    \
    {                                     \
        if (l)                            \
        {                                 \
            SyrList_Header(l)->count = 0; \
        }                                 \
    } while (0)

static inline void* SyrList_SetCapacity(void* list, size_t new_cap, size_t item_size)
{
    if (new_cap == 0)
        return list;

    size_t total_size = sizeof(SyrListHeader) + new_cap * item_size;
    SyrListHeader* header = NULL;

    if (list)
    {
        header = (SyrListHeader*)realloc(SyrList_Header(list), total_size);
    } else
    {
        header = (SyrListHeader*)malloc(total_size);
        header->count = 0;
    }

    header->capacity = new_cap;
    return (void*)(header + 1);
}

static inline void* SyrList_GrowImplementation(void* list, size_t increment, size_t item_size)
{
    size_t double_cap = list ? SyrList_Capacity(list) * 2 : 0;
    size_t min_cap = SyrList_Count(list) + increment;
    size_t new_cap = double_cap > min_cap ? double_cap : min_cap;
    if (new_cap < 8)
        new_cap = 8;

    return SyrList_SetCapacity(list, new_cap, item_size);
}
