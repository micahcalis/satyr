#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Instrument.h"
#include "System/RecordLabel/Chord.h"

typedef struct SyrInstrumentBinding
{
    uint32_t instrumentIndex;
    uint32_t instrumentSlot;
} SyrInstrumentBinding;

typedef struct SyrMetronomePhase
{
    SyrList(SyrInstrumentBinding) instrumentBindings;
    uint32_t instrumentCount;
    bool requiresMaster;
} SyrMetronomePhase;

typedef struct SyrMetronome
{
    SyrList(SyrMetronomePhase) phases;
} SyrMetronome;

SyrResult SyrMetronome_Initialize(SyrMetronome** metronome);

SyrResult SyrMetronome_RecordPhaseBarriers(SyrMetronome* metronome,
    SyrCommandBuffer* commandBuffer,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    const SyrList(SyrInstrument*) instruments);

SyrResult SyrMetronome_BindPhaseBuffers(SyrMetronome* metronome,
    SyrCommandBuffer* commandBuffer,
    SyrChord* chord,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    const SyrList(SyrInstrument*) instruments);

void SyrMetronome_Clear(SyrMetronome* metronome);
void SyrMetronome_Destroy(SyrMetronome* metronome);

