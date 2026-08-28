#include "Chord.h"

typedef struct SyrInstrumentBuffer
{
    SyrList(SyrInstrumentData) instrumentData;
    SyrBufferAllocation* bufferAllocation;
} SyrInstrumentBuffer;

SyrResult SyrInstrumentBuffer_Initialize(uint32_t dataCount,
    SyrBufferAllocation* bufferAllocation,
    SyrInstrumentBuffer** instrumentBuffer)
{
    if (dataCount > SYR_MAX_INSTRUMENTS + SYR_MASTER_INSTRUMENT_DATA_COUNT)
    {
        SYR_ERROR("Instrument Count higher than max (%u), can't create Instrument Buffer!",
            SYR_MAX_INSTRUMENTS);

        return SYR_RESULT_FAILED;
    }

    *instrumentBuffer = SYR_NEW(*instrumentBuffer);
    (*instrumentBuffer)->bufferAllocation = bufferAllocation;
    (*instrumentBuffer)->instrumentData = NULL;

    SyrInstrumentData emptyData = {0};

    for (uint32_t i = 0; i < dataCount; i++)
    {
        SyrList_Push((*instrumentBuffer)->instrumentData, emptyData);
    }

    return SYR_RESULT_SUCCESS;
}

void SyrInstrumentBuffer_SetInstrumentData(SyrInstrumentBuffer* instrumentBuffer,
    const SyrAudioBuffer* audioBuffer,
    const uint32_t dataIndex)
{
    int32_t dataCount = (int32_t)SyrList_Count(instrumentBuffer->instrumentData);
    if ((int32_t)dataIndex >= dataCount)
    {
        SYR_ERROR("Instrument Buffer Index (%u) out of range, max is %u!",
            dataIndex,
            dataCount - 1);

        return;
    }

    SyrInstrumentData* data = &instrumentBuffer->instrumentData[dataIndex];
    data->totalSamples = audioBuffer->totalSamples;
    data->sampleRate = audioBuffer->sampleRate;
    data->sampleMode = (uint32_t)audioBuffer->sampleMode;
}

SyrResult SyrInstrumentBuffer_UploadData(SyrInstrumentBuffer* instrumentBuffer,
    const size_t size)
{
    SyrResult uploadResult = SyrBufferAllocation_Upload(instrumentBuffer->bufferAllocation,
        (void*)instrumentBuffer->instrumentData,
        size,
        0);

    if (uploadResult != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Upload Data on Instrument Buffer!");
        return uploadResult;
    }

    return SYR_RESULT_SUCCESS;
}

static uint32_t SyrInstrumentBuffer_GetDataIndex(const uint32_t instrumentSlot)
{
    return instrumentSlot + SYR_MASTER_INSTRUMENT_DATA_COUNT;
}

void SyrInstrumentBuffer_Destroy(SyrInstrumentBuffer* instrumentBuffer)
{
    if (instrumentBuffer == NULL)
        return;

    if (instrumentBuffer->instrumentData != NULL)
    {
        SyrList_Free(instrumentBuffer->instrumentData);
    }

    if (instrumentBuffer->bufferAllocation != NULL)
    {
        SyrBufferAllocation_Destroy(instrumentBuffer->bufferAllocation);
    }

    SYR_FREE(instrumentBuffer);
}

typedef struct SyrChord
{
    SyrPipeline* pipeline;
    SyrDescriptor* descriptor;
    SyrNoteBuffer* noteBuffer;
    SyrInstrumentBuffer* instrumentBuffer;
    uint32_t instrumentCount;
    SyrThreadGroupSize threadGroupSize;
    char name[32];
} SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    SyrInstrumentBuffer* instrumentBuffer,
    const char name[32],
    const uint32_t instrumentCount,
    const SyrThreadGroupSize threadGroupSize,
    SyrChord** chord)
{
    *chord = SYR_NEW(*chord);
    (*chord)->pipeline = pipeline;
    (*chord)->descriptor = descriptor;
    (*chord)->noteBuffer = noteBuffer;
    (*chord)->instrumentBuffer = instrumentBuffer;
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

SyrResult SyrChord_WriteInstrumentData(SyrChord* chord)
{
    size_t size = sizeof(SyrInstrumentData) * SyrList_Count(chord->instrumentBuffer->instrumentData);
    SyrResult uploadResult = SyrInstrumentBuffer_UploadData(chord->instrumentBuffer, size);

    if (uploadResult != SYR_RESULT_SUCCESS)
        return uploadResult;

    SyrDescriptor_WriteBuffer(chord->descriptor,
        chord->instrumentBuffer->bufferAllocation,
        SYR_INSTRUMENT_DATA_DESCRIPTOR_BINDING,
        SYR_BUFFER_TYPE_SSBO,
        size,
        0);

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

    uint32_t instrumentTimeBinding = SYR_SETTINGS_MASTER_SSBO_COUNT + (instrumentSlot * SYR_INSTRUMENT_SSBO_COUNT);
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

    SyrInstrumentBuffer_SetInstrumentData(chord->instrumentBuffer,
        audioBuffer,
        SyrInstrumentBuffer_GetDataIndex(instrumentSlot));

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

    SyrInstrumentBuffer_SetInstrumentData(chord->instrumentBuffer,
        masterBuffer,
        SYR_MASTER_INSTRUMENT_DATA_INDEX);

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

    if (chord->instrumentBuffer != NULL)
    {
        SyrInstrumentBuffer_Destroy(chord->instrumentBuffer);
    }

    SYR_FREE(chord);
}
