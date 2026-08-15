#pragma once

#include "Core/SatyrCore.h"
#include "System/RecordLabel/Instrument.h"
#include "System/RecordLabel/Chord.h"

typedef struct SyrInstrumentBinding
{
    uint32_t instrumentIndex;
    enum SyrInstrumentSlot instrumentSlot;
} SyrInstrumentBinding;

typedef struct SyrMetronomePhase
{
    SyrList(SyrInstrumentBinding) instrumentBindings;
    uint32_t instrumentCount;
    bool requiresMaster;
} SyrMetronomePhase;

typedef struct SyrMetronomePhaseConfig
{
    SyrList(SyrInstrumentBinding) bindings;
    uint32_t bindingCount;
    bool requiresMaster;
} SyrMetronomePhaseConfig;

typedef struct SyrMetronomeConfig
{
    SyrList(SyrMetronomePhaseConfig) phaseConfigs;
    uint32_t phaseCount;
} SyrMetronomeConfig;

typedef struct SyrMetronome
{
    SyrList(SyrMetronomePhase) phases;
} SyrMetronome;

SyrResult SyrMetronome_Initialize(SyrMetronomeConfig* config,
    SyrMetronome** metronome);

SyrMetronomePhase* SyrMetronome_AddPhase(SyrMetronome* metronome,
    const SyrMetronomePhaseConfig* config);

SyrResult SyrMetronome_Configure(SyrMetronome* metronome,
    SyrMetronomeConfig* config);

SyrResult SyrMetronome_RecordPhaseBarriers(SyrMetronome* metronome,
    SyrCommandBuffer* commandBuffer,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    SyrList(SyrInstrument*) instruments);

SyrResult SyrMetronome_BindPhaseBuffers(SyrMetronome* metronome,
    SyrChord* chord,
    const uint32_t phaseIndex,
    const SyrAudioBuffer* masterBuffer,
    SyrList(SyrInstrument*) instruments);

void SyrMetronome_Clear(SyrMetronome* metronome);
void SyrMetronome_Destroy(SyrMetronome* metronome);
void SyrMetronomeConfig_Destroy(SyrMetronomeConfig* metronomeConfig);

