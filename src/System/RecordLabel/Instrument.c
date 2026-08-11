#include "Instrument.h"

static SyrResult SyrAudioBuffer_CreateTimeAlloc(SyrAudioBuffer* audioBuffer,
    SyrAllocator* allocator)
{
    SyrBufferAllocParams allocParams = {0};
    allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocParams.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    allocParams.size = sizeof(SyrTimeAudioSample) * audioBuffer->samples;

    audioBuffer->timeAllocation = SyrAllocator_AllocateBuffer(allocParams, allocator);

    if (audioBuffer->timeAllocation == NULL)
    {
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static SyrResult SyrAudioBuffer_CreateFrequencyAlloc(SyrAudioBuffer* audioBuffer,
    SyrAllocator* allocator)
{
    SyrBufferAllocParams allocParams = {0};
    allocParams.createFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocParams.memoryFlags = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocParams.usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    allocParams.size = sizeof(SyrFrequencyAudioSample) * audioBuffer->samples;

    audioBuffer->frequencyAllocation = SyrAllocator_AllocateBuffer(allocParams, allocator);

    if (audioBuffer->frequencyAllocation == NULL)
    {
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrAudioBuffer_Initialize(const uint32_t samples,
    SyrAllocator* allocator,
    SyrAudioBuffer** audioBuffer)
{
    if (samples == 0)
    {
        SYR_ERROR("Can't create Audio Buffer with 0 Samples!");
        return SYR_RESULT_FAILED;
    }

    *audioBuffer = SYR_NEW(*audioBuffer);
    (*audioBuffer)->samples = samples;

    if (SyrAudioBuffer_CreateTimeAlloc(*audioBuffer, allocator) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Time Allocation for Audio Buffer, samples: %u", samples);
        SyrAudioBuffer_Destroy(*audioBuffer);
        *audioBuffer = NULL;
        return SYR_RESULT_FAILED;
    }

    if (SyrAudioBuffer_CreateFrequencyAlloc(*audioBuffer, allocator) != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Frequency Allocation for Audio Buffer, samples: %u", samples);
        SyrAudioBuffer_Destroy(*audioBuffer);
        *audioBuffer = NULL;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

void SyrAudioBuffer_Destroy(SyrAudioBuffer* audioBuffer)
{
    if (audioBuffer == NULL)
        return;

    if (audioBuffer->timeAllocation != NULL)
    {
        SyrBufferAllocation_Destroy(audioBuffer->timeAllocation);
    }

    if (audioBuffer->frequencyAllocation != NULL)
    {
        SyrBufferAllocation_Destroy(audioBuffer->frequencyAllocation);
    }

    free(audioBuffer);
}

typedef struct SyrInstrument
{
    SyrAudioBuffer* audioBuffer;
    char name[32];
} SyrInstrument;

SyrResult SyrInstrument_Initialize(const uint32_t samples,
    const char name[32],
    SyrAllocator* allocator,
    SyrInstrument** instrument)
{
    *instrument = SYR_NEW(*instrument);
    SYR_STR_COPY((*instrument)->name, name);

    if (SyrAudioBuffer_Initialize(samples,
            allocator,
            &(*instrument)->audioBuffer)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to create Audio Buffer for Instrument: %s", name);
        SyrInstrument_Destroy(*instrument);
        *instrument = NULL;
        return SYR_RESULT_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

static float* SyrInstrument_DownmixStereo(const SyrAudioAsset* audioAsset)
{
    uint32_t assetSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO);
    float* downmixSamples = SYR_ALLOC_ARRAY(float, assetSamples);

    for (uint32_t i = 0; i < assetSamples; i++)
    {
        downmixSamples[i] = (audioAsset->pcmData[i * 2] + audioAsset->pcmData[i * 2 + 1]) * 0.5f;
    }

    return downmixSamples;
}

SyrResult SyrInstrument_UploadAsset(SyrInstrument* instrument,
    const SyrAudioAsset* audioAsset)
{
    uint32_t assetSamples = SyrAudioAsset_GetTotalSamples(audioAsset, SYR_AUDIO_ASSET_SAMPLE_MODE_MONO);

    if (assetSamples > instrument->audioBuffer->samples)
    {
        SYR_ERROR("Asset Samples (name: %s, samples: %u) are greater than Instrument allocated Samples (name %s, samples: %u)",
            audioAsset->name,
            assetSamples,
            instrument->name,
            instrument->audioBuffer->samples);

        return SYR_RESULT_FAILED;
    }

    if (assetSamples == 0)
    {
        SYR_ERROR("Audio Asset (name: %s) has 0 Samples, can't upload to Instrument (name: %s)",
            audioAsset->name,
            instrument->name);

        return SYR_RESULT_FAILED;
    }

    bool isStereo = SyrAudioAsset_IsStereo(audioAsset);
    float* uploadPcmData = NULL;

    if (isStereo)
    {
        uploadPcmData = SyrInstrument_DownmixStereo(audioAsset);
        if (uploadPcmData == NULL)
        {
            SYR_ERROR("Failed to downmix stereo asset: %s", audioAsset->name);
            return SYR_RESULT_FAILED;
        }
    } else
    {
        uploadPcmData = audioAsset->pcmData;
    }

    SyrResult result = SyrBufferAllocation_Upload(instrument->audioBuffer->timeAllocation,
        (void*)uploadPcmData,
        sizeof(float) * assetSamples,
        0);

    if (isStereo)
    {
        free(uploadPcmData);
    }

    if (result != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Upload Audio Asset (name: %s) to Instrument (name: %s)",
            audioAsset->name,
            instrument->name);

        return SYR_RESULT_RUNTIME_ERROR;
    }

    return SYR_RESULT_SUCCESS;
}

const SyrAudioBuffer* SyrInstrument_GetAudioBuffer(const SyrInstrument* instrument)
{
    return instrument->audioBuffer;
}

const char* SyrInstrument_GetName(const SyrInstrument* instrument)
{
    return instrument->name;
}

void SyrInstrument_Destroy(SyrInstrument* instrument)
{
    if (instrument == NULL)
        return;

    if (instrument->audioBuffer != NULL)
    {
        SyrAudioBuffer_Destroy(instrument->audioBuffer);
    }

    free(instrument);
}
