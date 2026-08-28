#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Pipeline.h"
#include "Core/Vulkan/Descriptor.h"
#include "System/RecordLabel/Notes.h"
#include "System/RecordLabel/Instrument.h"

#define SYR_SETTINGS_MASTER_SSBO_COUNT 4
#define SYR_MASTER_SSBO_COUNT 2
#define SYR_NOTES_DESCRIPTOR_BINDING 0
#define SYR_INSTRUMENT_DATA_DESCRIPTOR_BINDING 1
#define SYR_MASTER_TIME_DESCRIPTOR_BINDING 2
#define SYR_MASTER_FREQUENCY_DESCRIPTOR_BINDING 3
#define SYR_MASTER_INSTRUMENT_DATA_INDEX 0
#define SYR_MASTER_INSTRUMENT_DATA_COUNT 1

typedef enum
{
    SYR_THREAD_GROUP_SIZE_S = 8,
    SYR_THREAD_GROUP_SIZE_M = 16,
    SYR_THREAD_GROUP_SIZE_L = 32,
    SYR_THREAD_GROUP_SIZE_XL = 64
} SyrThreadGroupSize;

typedef struct SyrInstrumentData
{
    uint64_t totalSamples;
    uint32_t sampleRate;
    uint32_t sampleMode;
} SyrInstrumentData;

typedef struct SyrInstrumentBuffer SyrInstrumentBuffer;

SyrResult SyrInstrumentBuffer_Initialize(uint32_t dataCount,
    SyrBufferAllocation* bufferAllocation,
    SyrInstrumentBuffer** instrumentBuffer);

void SyrInstrumentBuffer_SetInstrumentData(SyrInstrumentBuffer* instrumentBuffer,
    const SyrAudioBuffer* audioBuffer,
    const uint32_t index);

SyrResult SyrInstrumentBuffer_UploadData(SyrInstrumentBuffer* instrumentBuffer,
    const size_t size);

void SyrInstrumentBuffer_Destroy(SyrInstrumentBuffer* instrumentBuffer);

typedef struct SyrChordConfig
{
    char name[32];
    SyrNotesData notesData;
    void* noteBufferData;
    uint32_t instrumentCount;
    char* shaderPath;
    uint32_t kernelIndex;
    SyrThreadGroupSize threadGroupSize;
} SyrChordConfig;

typedef struct SyrChord SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    SyrInstrumentBuffer* instrumentBuffer,
    const char name[32],
    const uint32_t instrumentCount,
    const SyrThreadGroupSize threadGroupSize,
    SyrChord** chord);

SyrResult SyrChord_WriteNotes(SyrChord* chord,
    const void* data,
    const size_t size,
    const size_t offset);

SyrResult SyrChord_WriteInstrumentData(SyrChord* chord);

SyrResult SyrChord_WriteMaster(SyrChord* chord,
    const SyrAudioBuffer* masterBuffer);

SyrResult SyrChord_WriteInstrument(SyrChord* chord,
    const SyrInstrument* instrument,
    const uint32_t instrumentSlot);

const char* SyrChord_GetName(const SyrChord* chord);
uint32_t SyrChord_GetInstrumentCount(const SyrChord* chord);
SyrThreadGroupSize SyrChord_GetThreadGroupSize(const SyrChord* chord);
const SyrDescriptor* SyrChord_GetDescriptor(const SyrChord* chord);
const SyrPipeline* SyrChord_GetPipeline(const SyrChord* chord);
void SyrChord_Destroy(SyrChord* chord);
