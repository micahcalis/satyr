#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/Pipeline.h"
#include "Core/Vulkan/Descriptor.h"
#include "System/RecordLabel/Notes.h"

#define SYR_NOTES_DESCRIPTOR_BINDING 0

typedef struct SyrChordConfig
{
    char name[32];
    SyrNotesData notesData;
    uint32_t instrumentCount;
    char* shaderPath;
    uint32_t kernelIndex;
} SyrChordConfig;

typedef struct SyrChord SyrChord;

SyrResult SyrChord_Initialize(SyrPipeline* pipeline,
    SyrDescriptor* descriptor,
    SyrNoteBuffer* noteBuffer,
    const char name[32],
    SyrChord** chord);

SyrResult SyrChord_WriteNotes(SyrChord* chord,
    const void* data,
    const size_t size,
    const size_t offset);

void SyrChord_Destroy(SyrChord* chord);
