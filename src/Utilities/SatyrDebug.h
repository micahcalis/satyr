#pragma once

#include <stdio.h>
#include <stdlib.h>

#if defined(SYR_DEBUG)

typedef struct SyrMemoryRecord
{
    void* address;
    size_t size;
    const char* file;
    int line;
    struct SyrMemoryRecord* next;
} SyrMemoryRecord;

    #if defined(_MSC_VER)
        #define SYR_WEAK __declspec(selectany)
    #elif defined(__GNUC__) || defined(__clang__)
        #define SYR_WEAK __attribute__((weak))
    #else
        #define SYR_WEAK static
    #endif

SYR_WEAK SyrMemoryRecord* g_SyrAllocHead = NULL;

static inline void* Syr_DebugMalloc(size_t size, const char* file, int line)
{
    void* ptr = malloc(size);
    if (!ptr)
        return NULL;

    SyrMemoryRecord* record = (SyrMemoryRecord*)malloc(sizeof(SyrMemoryRecord));
    if (!record)
        return ptr;

    record->address = ptr;
    record->size = size;
    record->file = file;
    record->line = line;
    record->next = g_SyrAllocHead;
    g_SyrAllocHead = record;

    return ptr;
}

static inline void* Syr_DebugCalloc(size_t num, size_t size, const char* file, int line)
{
    void* ptr = calloc(num, size);
    if (!ptr)
        return NULL;

    SyrMemoryRecord* record = (SyrMemoryRecord*)malloc(sizeof(SyrMemoryRecord));
    if (!record)
        return ptr;

    record->address = ptr;
    record->size = num * size;
    record->file = file;
    record->line = line;
    record->next = g_SyrAllocHead;
    g_SyrAllocHead = record;

    return ptr;
}

static inline void* Syr_DebugRealloc(void* ptr, size_t new_size, const char* file, int line)
{
    if (!ptr)
        return Syr_DebugMalloc(new_size, file, line);

    SyrMemoryRecord** current = &g_SyrAllocHead;
    while (*current)
    {
        if ((*current)->address == ptr)
        {
            void* new_ptr = realloc(ptr, new_size);
            if (!new_ptr)
                return NULL;

            (*current)->address = new_ptr;
            (*current)->size = new_size;
            (*current)->file = file;
            (*current)->line = line;
            return new_ptr;
        }
        current = &(*current)->next;
    }

    return realloc(ptr, new_size);
}

static inline void Syr_DebugFree(void* ptr)
{
    if (!ptr)
        return;

    SyrMemoryRecord** current = &g_SyrAllocHead;
    while (*current)
    {
        if ((*current)->address == ptr)
        {
            SyrMemoryRecord* toFree = *current;
            *current = (*current)->next;
            free(toFree);
            free(ptr);
            return;
        }
        current = &(*current)->next;
    }

    free(ptr);
}

static inline void Syr_ReportMemoryLeaks(void)
{
    if (!g_SyrAllocHead)
    {
        printf("[Satyr Tracker] No memory leaks detected!\n");
        return;
    }

    printf("\n================ MEMORY LEAK REPORT ================\n");
    size_t totalBytes = 0;
    size_t leakCount = 0;

    SyrMemoryRecord* curr = g_SyrAllocHead;
    while (curr)
    {
        printf("LEAK [%zu]: %zu bytes | File: %s | Line: %d | Addr: %p\n",
            ++leakCount,
            curr->size,
            curr->file,
            curr->line,
            curr->address);
        totalBytes += curr->size;
        curr = curr->next;
    }

    printf("===================================================\n");
    printf("TOTAL: %zu leak(s), %zu bytes total\n\n", leakCount, totalBytes);
}

    #define SYR_LOG(format, ...) printf("[Satyr] " format "\n", ##__VA_ARGS__)
    #define SYR_ERROR(format, ...) fprintf(stderr, "[Satyr Error] " format "\n", ##__VA_ARGS__)
    #define SYR_ALLOC(size) Syr_DebugMalloc((size), __FILE__, __LINE__)
    #define SYR_NEW(var) ((__typeof__(var))Syr_DebugMalloc(sizeof(*(var)), __FILE__, __LINE__))
    #define SYR_ALLOC_ARRAY(type, count) ((type*)Syr_DebugCalloc((count), sizeof(type), __FILE__, __LINE__))
    #define SYR_REALLOC_DBG(ptr, size, file, line) Syr_DebugRealloc((ptr), (size), (file), (line))
    #define SYR_FREE(ptr) Syr_DebugFree((ptr))

#else

    #define SYR_LOG(format, ...)
    #define SYR_ERROR(format, ...)
    #define SYR_ALLOC(size) malloc(size)
    #define SYR_NEW(var) ((__typeof__(var))malloc(sizeof(*(var))))
    #define SYR_ALLOC_ARRAY(type, count) ((type*)calloc((count), sizeof(type)))
    #define SYR_REALLOC_DBG(ptr, size, file, line) realloc((ptr), (size))
    #define SYR_FREE(ptr) free(ptr)
    #define Syr_ReportMemoryLeaks()

#endif
