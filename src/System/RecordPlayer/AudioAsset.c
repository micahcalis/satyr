#include "AudioAsset.h"
#include "miniaudio.h"

typedef ma_decoder MaDecoder;
typedef ma_decoder_config MaDecoderConfig;
typedef ma_uint64 MaUint64;
typedef ma_uint32 MaUint32;

SyrResult SyrAudioAsset_Load(const SyrAudioAssetLoadConfig* config,
    SyrAudioAsset** audioAsset)
{
    if (config->filePath == NULL || audioAsset == NULL)
    {
        SYR_ERROR("Invalid Arguments for Loading Audio File: %s", config->filePath);
        return SYR_RESULT_FAILED;
    }

    MaDecoder decoder;

    MaDecoderConfig decoderConfig = ma_decoder_config_init(SYR_MA_FORMAT,
        (MaUint32)config->sampleMode,
        config->sampleRate);

    if (ma_decoder_init_file(config->filePath,
            &decoderConfig,
            &decoder)
        != MA_SUCCESS)
    {
        SYR_ERROR("Failed to open Audio File: %s!", config->filePath);
        return SYR_RESULT_FAILED;
    }

    MaUint64 totalFrames64 = 0;
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
    MaUint64 framesRead = 0;

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

typedef ma_encoder MaEncoder;
typedef ma_encoder_config MaEncoderConfig;

SyrResult SyrAudioAsset_ExportWAV(SyrAudioAsset* audioAsset,
    const SyrAudioAssetExportConfig* config)
{
    if (config->filePath == NULL || audioAsset == NULL)
    {
        SYR_ERROR("Invalid Arguments for Exporting Audio File: %s", config->filePath);
        return SYR_RESULT_FAILED;
    }

    MaEncoder encoder;

    MaEncoderConfig encoderConfig = ma_encoder_config_init(ma_encoding_format_wav,
        SYR_MA_FORMAT,
        (MaUint32)config->sampleMode,
        config->sampleRate);

    if (ma_encoder_init_file(config->filePath,
            &encoderConfig,
            &encoder)
        != MA_SUCCESS)
    {
        SYR_ERROR("Failed to open Audio File: %s!", config->filePath);
        ma_encoder_uninit(&encoder);
        return SYR_RESULT_FAILED;
    }

    MaUint64 framesWritten = 0;

    if (ma_encoder_write_pcm_frames(&encoder,
            audioAsset->pcmData,
            audioAsset->totalFrames,
            &framesWritten)
        != MA_SUCCESS)
    {
        SYR_ERROR("Failed to write PCM Frames from Audio File: %s!", config->filePath);
        ma_encoder_uninit(&encoder);
        return SYR_RESULT_FAILED;
    }

    ma_encoder_uninit(&encoder);

    return SYR_RESULT_SUCCESS;
}

uint64_t SyrAudioAsset_GetTotalSamples(const SyrAudioAsset* audioAsset, SyrAudioAssetSampleMode sampleMode)
{
    if (audioAsset == NULL)
        return 0;

    switch (sampleMode)
    {
    case SYR_AUDIO_ASSET_SAMPLE_MODE_MONO: return audioAsset->totalFrames;
    case SYR_AUDIO_ASSET_SAMPLE_MODE_STEREO: return audioAsset->totalFrames * 2;
    default: return audioAsset->totalFrames;
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
