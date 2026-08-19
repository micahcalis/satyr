#include "AudioDevice.h"

typedef ma_result MaResult;
typedef ma_device_config MaDeviceConfig;

typedef struct SyrAudioDevice
{
    MaDevice* deviceHandle;
} SyrAudioDevice;

static SyrResult SyrAudioDevice_CreateDevice(SyrAudioDevice* audioDevice,
    const SyrConfig* config)
{
    MaDeviceConfig deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = config->playbackStereoEnabled ? 2 : 1;
    deviceConfig.sampleRate = config->overrideSampleRate ? config->overrideStandardSampleRate : SYR_STANDARD_SAMPLE_RATE;
    deviceConfig.dataCallback = NULL;
    deviceConfig.pUserData = audioDevice;

    if (ma_device_init(NULL, &deviceConfig, audioDevice->deviceHandle) != MA_SUCCESS)
    {
        SYR_ERROR("Failed to create Mini Audio Device!");
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioDevice_Initialize(SyrAudioDevice** audioDevice,
    const SyrConfig* config)
{
    *audioDevice = SYR_NEW(*audioDevice);

    if (SyrAudioDevice_CreateDevice(*audioDevice, config) != SYR_RESULT_SUCCESS)
    {
        SyrAudioDevice_Destroy(*audioDevice);
        *audioDevice = NULL;
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

void SyrAudioDevice_Destroy(SyrAudioDevice* audioDevice)
{
    if (audioDevice == NULL)
        return;

    if (audioDevice->deviceHandle != NULL)
    {
        ma_device_uninit(audioDevice->deviceHandle);
    }

    SYR_FREE(audioDevice);
}
