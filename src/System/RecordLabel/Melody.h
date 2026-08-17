#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Chord.h"
#include "System/RecordLabel/Metronome.h"

#define SYR_MAX_CHORDS 64

typedef struct SyrMelodyConfig
{
    char name[32];
    SyrChordConfig* chordConfigs;
    uint32_t chordCount;
} SyrMelodyConfig;

typedef struct SyrMelody SyrMelody;

SyrResult SyrMelody_Initialize(const char name[32], SyrMelody** melody);
void SyrMelody_AddChords(SyrMelody* melody, SyrChord** chords, size_t count);
void SyrMelody_PrintChords(SyrMelody* melody);
SyrChord* SyrMelody_GetChord(const SyrMelody* melody, const uint32_t index);
size_t SyrMelody_GetChordCount(const SyrMelody* melody);
const char* SyrMelody_GetName(const SyrMelody* melody);
SyrMetronomeConfig* SyrMelody_GetMetronomeConfigBody(const SyrMelody* melody);

SyrResult SyrMelody_PlayChord(SyrMelody* melody,
    SyrCommandBuffer* commandBuffer,
    const uint32_t index,
    const uint32_t dispatchSamples);

void SyrMelody_Destroy(SyrMelody* melody);

