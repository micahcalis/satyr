#include "AudioAsset.h"
#include "miniaudio.h"

typedef ma_decoder MaDecoder;
typedef ma_decoder_config MaDecoderConfig;

SyrResult SyrAudioAsset_Load(const SyrAudioAssetLoadConfig* config,
    SyrAudioAsset** audioAsset)
{
    if (config->filePath == NULL || audioAsset == NULL)
        return SYR_RESULT_FAILED;

    MaDecoder decoder;

    MaDecoderConfig decoderConfig = ma_decoder_config_init(ma_format_f32,
        config->sampleMode == SYR_AUDIO_ASSET_SAMPLE_MODE_MONO ? 1 : 2,
        config->sampleRate);

    if (ma_decoder_init_file(config->filePath,
            &decoderConfig,
            &decoder)
        != MA_SUCCESS)
    {
        SYR_ERROR("Failed to open Audio File: %s!", config->filePath);
        return SYR_RESULT_FAILED;
    }

    ma_uint64 totalFrames64 = 0;
    if (ma_decoder_get_available_frames(&decoder, &totalFrames64) != MA_SUCCESS || totalFrames64 == 0)
    {
        SYR_ERROR("0 Total Frames to read from Audio File: %s!", config->filePath);
        ma_decoder_uninit(&decoder);
        return SYR_RESULT_FAILED;
    }

    *audioAsset = SYR_NEW(*audioAsset);
    (*audioAsset)->sampleRate = decoder.outputSampleRate;
    (*audioAsset)->channels = decoder.outputChannels;
    (*audioAsset)->totalFrames = (uint64_t)totalFrames64;
    SYR_STR_COPY((*audioAsset)->name, config->name);

    size_t totalSamples = (size_t)totalFrames64 * decoder.outputChannels;
    (*audioAsset)->pcmData = (float*)SYR_ALLOC_ARRAY(float, totalSamples);
    ma_uint64 framesRead = 0;

    if (ma_decoder_read_pcm_frames(&decoder,
            (*audioAsset)->pcmData,
            (*audioAsset)->totalFrames,
            &framesRead)
        != MA_SUCCESS)
    {
        SYR_ERROR("Failed to read PCM Frames from Audio File: %s!", config->filePath);
        SyrAudioAsset_Destroy(*audioAsset);
        *audioAsset = NULL;
        ma_decoder_uninit(&decoder);
        return SYR_RESULT_FAILED;
    }

    (*audioAsset)->totalFrames = (uint64_t)framesRead;
    ma_decoder_uninit(&decoder);

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioAsset_Export(const SyrAudioAssetExportConfig* config,
    SyrAudioAsset** audioAsset)
{
    return SYR_RESULT_FAILED;
}

// SyrResult SyrAudioAsset_ExportWAV(const SyrAudioAsset* audioAsset, const char* filePath)
// {
//     if (audioAsset == NULL || audioAsset->pcmData == NULL || filePath == NULL)
//         return SYR_RESULT_FAILED;

//     // drwav_data_format format = {0};
//     // format.container = drwav_container_riff;
//     // format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
//     // format.channels = audioAsset->channels;
//     // format.sampleRate = audioAsset->sampleRate;
//     // format.bitsPerSample = 32;

//     // drwav wav;
//     // if (!drwav_init_file_write(&wav, filePath, &format, NULL))
//     // {
//     //     SYR_ERROR("Failed to open file for WAV export: %s", filePath);
//     //     return SYR_RESULT_FAILED;
//     // }

//     // drwav_write_pcm_frames(&wav, audioAsset->totalFrames, audioAsset->pcmData);
//     // drwav_uninit(&wav);

//     return SYR_RESULT_SUCCESS;
// }

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
        SYR_FREE(audioAsset->pcmData);
    }

    SYR_FREE(audioAsset);
}
