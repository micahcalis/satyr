#pragma once

#include "Core/SatyrCore.h"

typedef enum
{
    SYR_VOICE_STATE_FREE = 0,
    SYR_VOICE_STATE_PLAYING = 1,
    SYR_VOICE_STATE_STOPPING = 2
} SyrVoiceState;

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
    _Atomic(SyrVoiceState) voiceState;
    atomic_uint generation;
} SyrVoice;
