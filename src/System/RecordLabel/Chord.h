#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Pipeline.h"
#include "Core/Vulkan/Descriptor.h"
#include "System/RecordLabel/Notes.h"
#include "System/RecordLabel/Instrument.h"

#define SYR_MASTER_SSBO_COUNT 2
#define SYR_NOTES_DESCRIPTOR_BINDING 0
#define SYR_MASTER_TIME_DESCRIPTOR_BINDING 1
#define SYR_MASTER_FREQUENCY_DESCRIPTOR_BINDING 2

typedef struct SyrChordConfig
{
    char name[32];
    SyrNotesData notesData;
    void* noteBufferData;
    uint32_t instrumentCount;
    char* shaderPath;
    uint32_t kernelIndex;
} SyrChordConfig;

typedef struct SyrChord SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    const char name[32],
    const uint32_t instrumentCount,
    SyrChord** chord);

SyrResult SyrChord_WriteNotes(SyrChord* chord,
    const void* data,
    const size_t size,
    const size_t offset);

SyrResult SyrChord_WriteMaster(SyrChord* chord,
    const SyrAudioBuffer* masterBuffer);

SyrResult SyrChord_WriteInstrument(SyrChord* chord,
    const SyrInstrument* instrument,
    const uint32_t instrumentSlot);

const char* SyrChord_GetName(const SyrChord* chord);
uint32_t SyrChord_GetInstrumentCount(const SyrChord* chord);
void SyrChord_Destroy(SyrChord* chord);
