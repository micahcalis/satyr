#pragma once

#include "Core/SatyrCore.h"
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

SyrResult SyrSong_Release(SyrSong* song,
    SyrCommandBuffer* commandBuffer,
    SyrBufferAllocation* releaseBuffer,
    const size_t offset);

size_t SyrSong_GetMasterSize(const SyrSong* song);
const char* SyrSong_GetName(const SyrSong* song);
uint64_t SyrSong_GetTotalFrames(const SyrSong* song);
void SyrSong_Destroy(SyrSong* song);
