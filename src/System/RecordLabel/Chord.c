#include "Chord.h"

typedef struct SyrChord
{
    SyrPipeline* pipeline;
    SyrDescriptor* descriptor;
    SyrNoteBuffer* noteBuffer;
    char name[32];
} SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    const char name[32],
    SyrChord** chord)
{
    *chord = SYR_NEW(*chord);
    (*chord)->pipeline = pipeline;
    (*chord)->descriptor = descriptor;
    (*chord)->noteBuffer = noteBuffer;
    SYR_STR_COPY((*chord)->name, name);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrChord_WriteNotes(SyrChord* chord,
    const void* data,
    const size_t size,
    const size_t offset)
{
    SyrResult uploadResult = SyrNoteBuffer_Upload(chord->noteBuffer,
        data,
        size,
        offset);

    if (uploadResult != SYR_RESULT_SUCCESS)
        return uploadResult;

    SyrDescriptor_WriteBuffer(chord->descriptor,
        SyrNoteBuffer_GetBufferAllocation(chord->noteBuffer),
        SYR_NOTES_DESCRIPTOR_BINDING,
        SYR_BUFFER_TYPE_UNIFORM,
        size,
        offset);

    return SYR_RESULT_SUCCESS;
}

const char* SyrChord_GetName(SyrChord* chord)
{
    return chord->name;
}

void SyrChord_Destroy(SyrChord* chord)
{
    if (chord == NULL)
        return;

    if (chord->pipeline != NULL)
    {
        SyrPipeline_Destroy(chord->pipeline);
    }

    if (chord->descriptor != NULL)
    {
        SyrDescriptor_Destroy(chord->descriptor);
    }

    if (chord->noteBuffer != NULL)
    {
        SyrNoteBuffer_Destroy(chord->noteBuffer);
    }

    free(chord);
}
