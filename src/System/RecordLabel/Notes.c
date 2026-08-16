#include "Notes.h"

typedef struct SyrNoteBuffer
{
    SyrNotesData notesData;
    SyrBufferAllocation* bufferAllocation;
} SyrNoteBuffer;

SyrResult SyrNoteBuffer_Initialize(const SyrNotesData notesData,
    SyrBufferAllocation* bufferAllocation,
    SyrNoteBuffer** noteBuffer)
{
    if (bufferAllocation->info.size < notesData.size)
    {
        SYR_ERROR("Note Buffer size (%zu) is smaller than Note Data size (%zu)!",
            bufferAllocation->info.size,
            notesData.size);

        return SYR_RESULT_RUNTIME_ERROR;
    }

    *noteBuffer = SYR_NEW(*noteBuffer);
    (*noteBuffer)->notesData = notesData;
    (*noteBuffer)->bufferAllocation = bufferAllocation;

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrNoteBuffer_Upload(SyrNoteBuffer* noteBuffer,
    const void* newNotes,
    const size_t size,
    const size_t offset)
{
    SyrResult uploadResult = SyrBufferAllocation_Upload(noteBuffer->bufferAllocation,
        newNotes,
        size,
        offset);

    if (uploadResult != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Upload Data on Note Buffer: %s", noteBuffer->notesData.name);
        return uploadResult;
    }

    return SYR_RESULT_SUCCESS;
}

SyrNotesData SyrNoteBuffer_GetNotesData(SyrNoteBuffer* noteBuffer)
{
    return noteBuffer->notesData;
}

SyrBufferAllocation* SyrNoteBuffer_GetBufferAllocation(SyrNoteBuffer* noteBuffer)
{
    return noteBuffer->bufferAllocation;
}

void SyrNoteBuffer_Destroy(SyrNoteBuffer* noteBuffer)
{
    if (noteBuffer == NULL)
        return;

    if (noteBuffer->bufferAllocation != NULL)
    {
        SyrBufferAllocation_Destroy(noteBuffer->bufferAllocation);
    }

    SYR_FREE(noteBuffer);
}
