#pragma once

#include "Core./SatyrCore.h"
#include "miniaudio.h"

#define SYR_STANDARD_SAMPLE_RATE 48000

typedef ma_device MaDevice;
typedef struct SyrAudioDevice SyrAudioDevice;

SyrResult SyrAudioDevice_Initialize(SyrAudioDevice** audioDevice,
    const SyrConfig* config);

void SyrAudioDevice_Destroy(SyrAudioDevice* audioDevice);
