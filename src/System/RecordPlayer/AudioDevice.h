#pragma once

#include "Core./SatyrCore.h"
#include "miniaudio.h"

typedef ma_device MaDevice;
typedef ma_device_data_proc MaDeviceCallback;
typedef struct SyrAudioDevice SyrAudioDevice;

SyrResult SyrAudioDevice_Initialize(const SyrConfig* config,
    MaDeviceCallback callback,
    void* pUserData,
    SyrAudioDevice** audioDevice);

uint32_t SyrAudioDevice_GetSampleRate(const SyrAudioDevice* audioDevice);
uint32_t SyrAudioDevice_GetChannels(const SyrAudioDevice* audioDevice);
void SyrAudioDevice_Destroy(SyrAudioDevice* audioDevice);
