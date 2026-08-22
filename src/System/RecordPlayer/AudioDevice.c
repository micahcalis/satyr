#include "AudioDevice.h"
#include "System/RecordPlayer/AudioAsset.h"

typedef ma_result MaResult;
typedef ma_device_config MaDeviceConfig;

typedef struct SyrAudioDevice
{
    MaDevice deviceHandle;
    uint32_t sampleRate;
    uint8_t channels;
} SyrAudioDevice;

static SyrResult SyrAudioDevice_CreateDevice(SyrAudioDevice* audioDevice,
    const SyrConfig* config,
    MaDeviceCallback callback,
    void* pUserData)
{
    MaDeviceConfig deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = config->playbackStereoEnabled ? 2 : 1;
    deviceConfig.sampleRate = config->overrideSampleRate ? config->overrideStandardSampleRate : SYR_AUDIO_SAMPLE_RATE;
    deviceConfig.dataCallback = callback;
    deviceConfig.pUserData = pUserData;

    if (ma_device_init(NULL, &deviceConfig, &audioDevice->deviceHandle) != MA_SUCCESS)
    {
        SYR_ERROR("Failed to create Mini Audio Device!");
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    audioDevice->sampleRate = (uint32_t)deviceConfig.sampleRate;
    audioDevice->channels = (uint8_t)deviceConfig.playback.channels;

    if (ma_device_start(&audioDevice->deviceHandle) != MA_SUCCESS)
    {
        SYR_ERROR("Failed to start audio playback device!");
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioDevice_Initialize(const SyrConfig* config,
    MaDeviceCallback callback,
    void* pUserData,
    SyrAudioDevice** audioDevice)
{
    *audioDevice = SYR_NEW(*audioDevice);

    if (SyrAudioDevice_CreateDevice(*audioDevice,
            config,
            callback,
            pUserData)
        != SYR_RESULT_SUCCESS)
    {
        SyrAudioDevice_Destroy(*audioDevice);
        *audioDevice = NULL;
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

uint32_t SyrAudioDevice_GetSampleRate(const SyrAudioDevice* audioDevice)
{
    return audioDevice->sampleRate;
}

uint32_t SyrAudioDevice_GetChannels(const SyrAudioDevice* audioDevice)
{
    return audioDevice->channels;
}

void SyrAudioDevice_Destroy(SyrAudioDevice* audioDevice)
{
    if (audioDevice == NULL)
        return;

    ma_device_uninit(&audioDevice->deviceHandle);

    SYR_FREE(audioDevice);
}
