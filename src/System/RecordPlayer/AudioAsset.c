#include "AudioAsset.h"
#include "dr_mp3.h"
#include "dr_wav.h"

SyrResult SyrAudioAsset_LoadWAV(const char* filePath,
    const char name[64],
    SyrAudioAsset** audioAsset)
{
    if (filePath == NULL || audioAsset == NULL)
        return SYR_RESULT_FAILED;

    *audioAsset = SYR_NEW(*audioAsset);

    (*audioAsset)->pcmData = drwav_open_file_and_read_pcm_frames_f32(filePath,
        &(*audioAsset)->channels,
        &(*audioAsset)->sampleRate,
        &(*audioAsset)->totalFrames,
        NULL);

    if ((*audioAsset)->pcmData == NULL)
    {
        SYR_ERROR("Failed to decode WAV file: %s", filePath);
        SyrAudioAsset_Destroy(*audioAsset);
        *audioAsset = NULL;
        return SYR_RESULT_FAILED;
    }

    const char* assetName = (name && name[0] != '\0') ? name : filePath;
    SYR_STR_COPY((*audioAsset)->name, assetName);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioAsset_LoadMP3(const char* filePath,
    const char name[64],
    SyrAudioAsset** audioAsset)
{
    if (filePath == NULL || audioAsset == NULL)
        return SYR_RESULT_FAILED;

    *audioAsset = SYR_NEW(*audioAsset);

    drmp3_config config = {0};

    (*audioAsset)->pcmData = drmp3_open_file_and_read_pcm_frames_f32(filePath,
        &config,
        &(*audioAsset)->totalFrames,
        NULL);

    if ((*audioAsset)->pcmData == NULL)
    {
        SYR_ERROR("Failed to decode MP3 file: %s", filePath);
        SyrAudioAsset_Destroy(*audioAsset);
        *audioAsset = NULL;
        return SYR_RESULT_FAILED;
    }

    (*audioAsset)->channels = config.channels;
    (*audioAsset)->sampleRate = config.sampleRate;
    const char* assetName = (name && name[0] != '\0') ? name : filePath;
    SYR_STR_COPY((*audioAsset)->name, assetName);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioAsset_ExportWAV(const SyrAudioAsset* audioAsset, const char* filePath)
{
    if (audioAsset == NULL || audioAsset->pcmData == NULL || filePath == NULL)
        return SYR_RESULT_FAILED;

    drwav_data_format format = {0};
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = audioAsset->channels;
    format.sampleRate = audioAsset->sampleRate;
    format.bitsPerSample = 32;

    drwav wav;
    if (!drwav_init_file_write(&wav, filePath, &format, NULL))
    {
        SYR_ERROR("Failed to open file for WAV export: %s", filePath);
        return SYR_RESULT_FAILED;
    }

    drwav_write_pcm_frames(&wav, audioAsset->totalFrames, audioAsset->pcmData);
    drwav_uninit(&wav);

    return SYR_RESULT_SUCCESS;
}

uint32_t SyrAudioAsset_GetTotalSamples(const SyrAudioAsset* audioAsset, SyrAudioAssetSampleMode sampleMode)
{
    if (audioAsset == NULL)
        return 0;

    switch (sampleMode)
    {
    case SYR_AUDIO_ASSET_SAMPLE_MODE_MONO: return (uint32_t)audioAsset->totalFrames;
    case SYR_AUDIO_ASSET_SAMPLE_MODE_STEREO: return (uint32_t)(audioAsset->totalFrames * 2);
    }
}

bool SyrAudioAsset_IsStereo(const SyrAudioAsset* audioAsset)
{
    return audioAsset->channels == 2;
}

void SyrAudioAsset_Destroy(SyrAudioAsset* audioAsset)
{
    if (audioAsset == NULL)
        return;

    if (audioAsset->pcmData != NULL)
    {
        free(audioAsset->pcmData);
    }

    free(audioAsset);
}
