#pragma once

#include "Core/SatyrCore.h"

typedef struct SyrVoice
{
    const float* pcmData;
    uint64_t totalFrames;
    uint64_t frameSegmentBegin;
    uint64_t frameSegmentEnd;
    uint32_t sampleRate;
    double cursorFrame;
    float volume;
    float pitch;
    uint8_t channels;
    bool isLooping;
    atomic_bool isPlaying;
    atomic_uint generation;
} SyrVoice;
