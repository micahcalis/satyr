#pragma once

#include "Core/SatyrCore.h"
#include "Core/Vulkan/CommandBuffer.h"
#include "System/RecordLabel/Instrument.h"
#include "System/RecordLabel/Melody.h"

#define SYR_MAX_INSTRUMENTS 31

typedef struct SyrSong SyrSong;

SyrResult SyrSong_Initialize(SyrMelody* melody, SyrSong** song);
void SyrSong_AddInstruments(SyrSong* song, SyrInstrument** instruments, size_t count);
void SyrSong_Record(SyrSong* song, SyrCommandBuffer* commandBuffer);
void SyrSong_Destroy(SyrSong* song);
