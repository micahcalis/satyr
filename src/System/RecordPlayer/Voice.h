#pragma once

#include "Core/SatyrCore.h"

#define SYR_MAX_VOICES 256

typedef SyrSlotId SyrVoiceId;
typedef struct SyrVoice SyrVoice;

void SyrVoice_SetVolume(SyrVoice* voice, float volume);
void SyrVoice_SetPitch(SyrVoice* voice, float pitch);
void SyrVoice_SetPlaying(SyrVoice* voice, bool playing);

float SyrVoice_CalculateSampleLerp(const SyrVoice* voice,
    const uint8_t channel);
