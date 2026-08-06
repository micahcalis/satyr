#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Chord.h"

#define SYR_MAX_CHORDS 64

typedef struct SyrMelody SyrMelody;

SyrResult SyrMelody_Initialize(SyrMelody** melody);
void SyrMelody_AddChords(SyrMelody* melody, SyrChord** chords, size_t count);
void SyrMelody_Destroy(SyrMelody* melody);

