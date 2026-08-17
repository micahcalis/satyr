#include "Chord.h"

typedef struct SyrChord
{
    SyrPipeline* pipeline;
    SyrDescriptor* descriptor;
    SyrNoteBuffer* noteBuffer;
    uint32_t instrumentCount;
    SyrThreadGroupSize threadGroupSize;
    char name[32];
} SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    const char name[32],
    const uint32_t instrumentCount,
    const SyrThreadGroupSize threadGroupSize,
    SyrChord** chord)
{
    *chord = SYR_NEW(*chord);
    (*chord)->pipeline = pipeline;
    (*chord)->descriptor = descriptor;
    (*chord)->noteBuffer = noteBuffer;
    (*chord)->instrumentCount = instrumentCount;
    (*chord)->threadGroupSize = threadGroupSize;
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

SyrResult SyrChord_WriteInstrument(SyrChord* chord,
    const SyrInstrument* instrument,
    const uint32_t instrumentSlot)
{
    if (chord->instrumentCount == 0)
    {
        SYR_ERROR("Can't write Instrument to Chord (name: %s) which has 0 Instrument slots!", chord->name);
        return SYR_RESULT_RUNTIME_ERROR;
    }

    if (instrumentSlot >= chord->instrumentCount)
    {
        SYR_ERROR("Tried writing Instrument (name: %s) to Chord (name: %s) to slot %u, while max slot index is %u",
            SyrInstrument_GetName(instrument),
            chord->name,
            instrumentSlot,
            chord->instrumentCount - 1);

        return SYR_RESULT_RUNTIME_ERROR;
    }

    uint32_t instrumentTimeBinding = SYR_NOTE_BUFFER_UB_COUNT + SYR_MASTER_SSBO_COUNT + (instrumentSlot * SYR_INSTRUMENT_SSBO_COUNT);
    uint32_t instrumentFrequencyBinding = instrumentTimeBinding + 1;
    const SyrAudioBuffer* audioBuffer = SyrInstrument_GetAudioBuffer(instrument);

    SyrDescriptor_WriteBuffer(chord->descriptor,
        audioBuffer->timeAllocation,
        instrumentTimeBinding,
        SYR_BUFFER_TYPE_SSBO,
        audioBuffer->timeAllocation->info.size,
        0);

    SyrDescriptor_WriteBuffer(chord->descriptor,
        audioBuffer->frequencyAllocation,
        instrumentFrequencyBinding,
        SYR_BUFFER_TYPE_SSBO,
        audioBuffer->frequencyAllocation->info.size,
        0);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrChord_WriteMaster(SyrChord* chord,
    const SyrAudioBuffer* masterBuffer)
{
    SyrDescriptor_WriteBuffer(chord->descriptor,
        masterBuffer->timeAllocation,
        SYR_MASTER_TIME_DESCRIPTOR_BINDING,
        SYR_BUFFER_TYPE_SSBO,
        masterBuffer->timeAllocation->info.size,
        0);

    SyrDescriptor_WriteBuffer(chord->descriptor,
        masterBuffer->frequencyAllocation,
        SYR_MASTER_FREQUENCY_DESCRIPTOR_BINDING,
        SYR_BUFFER_TYPE_SSBO,
        masterBuffer->frequencyAllocation->info.size,
        0);

    return SYR_RESULT_SUCCESS;
}

const char* SyrChord_GetName(const SyrChord* chord)
{
    return chord->name;
}

uint32_t SyrChord_GetInstrumentCount(const SyrChord* chord)
{
    return chord->instrumentCount;
}

SyrThreadGroupSize SyrChord_GetThreadGroupSize(const SyrChord* chord)
{
    return chord->threadGroupSize;
}

const SyrDescriptor* SyrChord_GetDescriptor(const SyrChord* chord)
{
    return chord->descriptor;
}

const SyrPipeline* SyrChord_GetPipeline(const SyrChord* chord)
{
    return chord->pipeline;
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

    SYR_FREE(chord);
}
