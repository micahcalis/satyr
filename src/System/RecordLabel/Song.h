#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/CommandBuffer.h"
#include "System/RecordLabel/Instrument.h"
#include "System/RecordLabel/Melody.h"
#include "System/RecordLabel/Metronome.h"

typedef struct SyrSongConfig
{
    char name[32];
    uint32_t masterSamples;
    SyrInstrumentConfig* instrumentConfigs;
    uint32_t instrumentCount;
    SyrMelody* melody;
    SyrMetronome* metronome;
} SyrSongConfig;

typedef struct SyrSong SyrSong;

SyrResult SyrSong_Initialize(SyrAudioBuffer* masterBuffer,
    SyrMelody* melody,
    SyrMetronome* metronome,
    const char name[32],
    SyrSong** song);

void SyrSong_AddInstruments(SyrSong* song, SyrInstrument** instruments, size_t count);

SyrResult SyrSong_Record(SyrSong* song, SyrCommandBuffer* commandBuffer);
void SyrSong_Destroy(SyrSong* song);
