#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Allocations.h"

#define SYR_NOTE_BUFFER_UB_COUNT 1

typedef struct SyrNotesData
{
    size_t size;
    char name[32];
} SyrNotesData;

typedef struct SyrNoteBuffer SyrNoteBuffer;

SyrResult SyrNoteBuffer_Initialize(const SyrNotesData notesData,
    SyrBufferAllocation* bufferAllocation,
    SyrNoteBuffer** noteBuffer);

SyrResult SyrNoteBuffer_Upload(SyrNoteBuffer* noteBuffer,
    const void* newNotes,
    const size_t size,
    const size_t offset);

SyrNotesData SyrNoteBuffer_GetNotesData(SyrNoteBuffer* noteBuffer);
SyrBufferAllocation* SyrNoteBuffer_GetBufferAllocation(SyrNoteBuffer* noteBuffer);

void SyrNoteBuffer_Destroy(SyrNoteBuffer* noteBuffer);
