#include "RecordPlayer.h"
#include "System/RecordPlayer/VinylInternal.h"
#include "System/RecordPlayer/VoiceInternal.h"
#include "System/RecordPlayer/AudioDevice.h"

SYR_DEFINE_SLOT_MAP(SyrVinyl, SyrVinylSlotMap, SYR_MAX_VINYLS)
SYR_DEFINE_SLOT_MAP(SyrVoice, SyrVoiceSlotMap, SYR_MAX_VOICES)

typedef struct SyrRecordPlayer
{
    SyrAudioDevice* audioDevice;
    SyrVinylSlotMap vinylSlotMap;
    SyrVoice voices[SYR_MAX_VOICES];
} SyrRecordPlayer;

static void SyrAudioDevice_VoiceUpdate(SyrVoice* voice,
    float* outputBuffer,
    const ma_uint32 frameCount,
    const uint8_t deviceChannels,
    const double deviceSampleRate)
{
    double sampleRatio = (double)voice->sampleRate / deviceSampleRate;
    double cursorStep = (double)voice->pitch * sampleRatio;

    for (ma_uint32 f = 0; f < frameCount; f++)
    {
        if (voice->cursorFrame >= (double)voice->frameSegmentEnd)
        {
            if (voice->isLooping)
            {
                double segmentLength = (double)(voice->frameSegmentEnd - voice->frameSegmentBegin);
                voice->cursorFrame -= (segmentLength > 0.0 ? segmentLength : 1.0);
            } else
            {
                atomic_store_explicit(&voice->voiceState, SYR_VOICE_STATE_FREE, memory_order_release);
                break;
            }
        }

        if (voice->cursorFrame > (double)voice->frameSegmentEnd)
        {
            voice->cursorFrame = (double)voice->frameSegmentEnd;
        }
        for (uint8_t dc = 0; dc < deviceChannels; dc++)
        {
            uint8_t vc = (dc < voice->channels) ? dc : 0;
            float rawSample = SyrVoice_CalculateSampleLerp(voice, vc);
            outputBuffer[f * deviceChannels + dc] += rawSample * voice->volume;
        }

        voice->cursorFrame += cursorStep;
    }
}

static void SyrAudioDevice_DataCallback(MaDevice* deviceHandle, void* output, const void* input, ma_uint32 frameCount)
{
    SyrRecordPlayer* recordPlayer = (SyrRecordPlayer*)deviceHandle->pUserData;

    if (recordPlayer == NULL)
        return;

    double deviceSampleRate = (double)SyrAudioDevice_GetSampleRate(recordPlayer->audioDevice);
    uint8_t deviceChannels = SyrAudioDevice_GetChannels(recordPlayer->audioDevice);
    float* outputBuffer = (float*)output;
    memset(outputBuffer, 0, frameCount * deviceChannels * sizeof(float));

    for (int i = 0; i < SYR_MAX_VOICES; i++)
    {
        SyrVoice* voice = &recordPlayer->voices[i];
        SyrVoiceState voiceState = atomic_load_explicit(&voice->voiceState, memory_order_acquire);
        bool playing = voiceState == SYR_VOICE_STATE_PLAYING;

        if (playing && voice->pcmData != NULL)
        {
            SyrAudioDevice_VoiceUpdate(voice,
                outputBuffer,
                frameCount,
                deviceChannels,
                deviceSampleRate);
        }

        if (voiceState == SYR_VOICE_STATE_STOPPING)
        {
            atomic_store_explicit(&voice->voiceState, SYR_VOICE_STATE_FREE, memory_order_release);
        }
    }

    (void)input;
}

static SyrResult SyrRecordPlayer_InitializeDevice(SyrRecordPlayer* recordPlayer,
    const SyrConfig* config)
{
    if (SyrAudioDevice_Initialize(config,
            SyrAudioDevice_DataCallback,
            recordPlayer,
            &recordPlayer->audioDevice)
        != SYR_RESULT_SUCCESS)
    {
        SYR_ERROR("Failed to Initialize Audio Device for Record Player!");
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrResult SyrRecordPlayer_Initialize(const SyrConfig* config,
    SyrRecordPlayer** recordPlayer)
{
    *recordPlayer = SYR_NEW(*recordPlayer);

    SyrVinylSlotMap_Initialize(&(*recordPlayer)->vinylSlotMap);
    memset((*recordPlayer)->voices, 0, sizeof(SyrVoice) * SYR_MAX_VOICES);

    if (SyrRecordPlayer_InitializeDevice(*recordPlayer, config) != SYR_RESULT_SUCCESS)
    {
        SyrRecordPlayer_Destroy(*recordPlayer);
        *recordPlayer = NULL;
        return SYR_RESULT_MINIAUDIO_FAILED;
    }

    return SYR_RESULT_SUCCESS;
}

SyrVinylId SyrRecordPlayer_CreateVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylConfig* config)
{
    SyrVinyl vinyl = {0};
    vinyl.mode = config->mode;
    vinyl.ownership = config->ownership;
    vinyl.audioAsset = config->audioAsset;
    vinyl.frameSegmentBegin = config->frameSegmentBegin;
    vinyl.frameSegmentEnd = config->frameSegmentEnd;
    SYR_STR_COPY(vinyl.name, config->name);

    return SyrVinylSlotMap_Insert(&recordPlayer->vinylSlotMap, vinyl);
}

SyrVinyl* SyrRecordPlayer_GetVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId id)
{
    return SyrVinylSlotMap_Get(&recordPlayer->vinylSlotMap, id);
}

SyrResult SyrRecordPlayer_DestroyVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId id)
{
    SyrVinyl* vinyl = SyrRecordPlayer_GetVinyl(recordPlayer, id);

    if (vinyl == NULL)
    {
        SYR_ERROR("Record Player can't destroy Vinyl that is already destroyed or doesn't exist!");
        return SYR_RESULT_FAILED;
    }

    if (vinyl->ownership == SYR_VINYL_ASSET_OWNERSHIP_STRICT && vinyl->audioAsset != NULL)
    {
        SyrAudioAsset_Destroy(vinyl->audioAsset);
    }

    return SyrVinylSlotMap_Remove(&recordPlayer->vinylSlotMap, id);
}

static SyrVoiceId SyrRecordPlayer_TryGetVoiceId(SyrRecordPlayer* recordPlayer)
{
    for (int i = 0; i < SYR_MAX_VOICES; i++)
    {
        SyrVoice* voice = &recordPlayer->voices[i];
        SyrVoiceState expectedState = SYR_VOICE_STATE_FREE;

        if (atomic_compare_exchange_strong_explicit(
                &voice->voiceState,
                &expectedState,
                SYR_VOICE_STATE_PLAYING,
                memory_order_acq_rel,
                memory_order_relaxed))
        {
            uint32_t newGen = atomic_load_explicit(&voice->generation, memory_order_relaxed) + 1;
            atomic_store_explicit(&voice->generation, newGen, memory_order_release);

            return SyrSlotId_Create(i, newGen);
        }
    }

    return SYR_INVALID_SLOT_ID;
}

static void SyrRecordPlayer_CreateVoiceFromVinyl(SyrRecordPlayer* recordPlayer,
    SyrVoice* voice,
    const SyrVinyl* vinyl,
    const uint32_t generation,
    const float volume,
    const float pitch)
{
    voice->pcmData = vinyl->audioAsset->pcmData;
    voice->totalFrames = vinyl->audioAsset->totalFrames;
    voice->channels = vinyl->audioAsset->channels;
    voice->sampleRate = vinyl->audioAsset->sampleRate;
    voice->volume = volume;
    voice->pitch = pitch;

    switch (vinyl->mode)
    {
    case SYR_VINYL_MODE_LOOP_SEGMENT:
    case SYR_VINYL_MODE_PLAY_ONCE_SEGMENT:
        voice->frameSegmentBegin = SYR_MATH_MIN(vinyl->frameSegmentBegin, vinyl->audioAsset->totalFrames);
        voice->frameSegmentEnd = SYR_MATH_MIN(vinyl->frameSegmentEnd, vinyl->audioAsset->totalFrames);

        if (voice->frameSegmentBegin >= voice->frameSegmentEnd)
        {
            voice->frameSegmentBegin = 0;
            voice->frameSegmentEnd = vinyl->audioAsset->totalFrames;
        }

        voice->isLooping = (vinyl->mode == SYR_VINYL_MODE_LOOP_SEGMENT);
        voice->cursorFrame = (double)voice->frameSegmentBegin;
        break;

    case SYR_VINYL_MODE_LOOP:
    case SYR_VINYL_MODE_PLAY_ONCE:
    default:
        voice->frameSegmentBegin = 0;
        voice->frameSegmentEnd = vinyl->audioAsset->totalFrames;
        voice->isLooping = (vinyl->mode == SYR_VINYL_MODE_LOOP);
        voice->cursorFrame = 0.0;
        break;
    }
}

SyrVoiceId SyrRecordPlayer_PlayVinyl(SyrRecordPlayer* recordPlayer,
    const SyrVinylId vinylId,
    const float volume,
    const float pitch)
{
    SyrVinyl* vinyl = SyrVinylSlotMap_Get(&recordPlayer->vinylSlotMap, vinylId);

    if (vinyl == NULL || vinyl->audioAsset == NULL || vinyl->audioAsset->pcmData == NULL)
    {
        SYR_ERROR("Can't play Vinyl: Invalid handle or uninitialized audio asset!");
        return SYR_INVALID_SLOT_ID;
    }

    if (vinyl->audioAsset->totalFrames == 0)
    {
        SYR_ERROR("Can't play Vinyl (name: %s) with 0 Total Frames!", vinyl->name);
        return SYR_INVALID_SLOT_ID;
    }

    SyrVoiceId voiceId = SyrRecordPlayer_TryGetVoiceId(recordPlayer);

    if (voiceId == SYR_INVALID_SLOT_ID)
    {
        SYR_ERROR("Can't play Vinyl (name: %s), no available Voices (max: %u)!",
            vinyl->name,
            SYR_MAX_VOICES);

        return SYR_INVALID_SLOT_ID;
    }

    uint32_t index = SyrSlotId_GetIndex(voiceId);
    uint32_t generation = SyrSlotId_GetGeneration(voiceId);
    SyrVoice* voice = &recordPlayer->voices[index];

    SyrRecordPlayer_CreateVoiceFromVinyl(recordPlayer,
        voice,
        vinyl,
        generation,
        volume,
        pitch);

    return voiceId;
}

SyrVoice* SyrRecordPlayer_GetVoice(SyrRecordPlayer* recordPlayer,
    const SyrVoiceId voiceId)
{
    uint32_t index = SyrSlotId_GetIndex(voiceId);
    uint32_t generation = SyrSlotId_GetGeneration(voiceId);

    if (index >= SYR_MAX_VOICES)
    {
        SYR_ERROR("Voice Index out of range (%u), max is %u!",
            index,
            SYR_MAX_VOICES);

        return NULL;
    }

    SyrVoice* voice = &recordPlayer->voices[index];
    uint32_t currentGeneration = atomic_load_explicit(&voice->generation, memory_order_relaxed);

    if (currentGeneration != generation)
    {
        return NULL;
    }

    bool isPlaying = atomic_load_explicit(&voice->voiceState, memory_order_acquire) == SYR_VOICE_STATE_PLAYING;

    if (!isPlaying)
    {
        return NULL;
    }

    return voice;
}

SyrResult SyrRecordPlayer_StopVoice(SyrRecordPlayer* recordPlayer,
    const SyrVoiceId voiceId)
{
    SyrVoice* voice = SyrRecordPlayer_GetVoice(recordPlayer, voiceId);

    if (voice == NULL)
    {
        return SYR_RESULT_FAILED;
    }

    atomic_store_explicit(&voice->voiceState, SYR_VOICE_STATE_STOPPING, memory_order_release);

    return SYR_RESULT_SUCCESS;
}

void SyrRecordPlayer_Destroy(SyrRecordPlayer* recordPlayer)
{
    if (recordPlayer == NULL)
        return;

    if (recordPlayer->audioDevice != NULL)
    {
        SyrAudioDevice_Destroy(recordPlayer->audioDevice);
    }

    for (int i = recordPlayer->vinylSlotMap.denseCount - 1; i >= 0; i--)
    {
        SyrVinyl* vinyl = &recordPlayer->vinylSlotMap.dense[i];
        uint32_t slotIndex = recordPlayer->vinylSlotMap.denseToSlot[i];
        uint32_t generation = recordPlayer->vinylSlotMap.slots[slotIndex].generation;
        SyrVinylId vinylId = SyrSlotId_Create(slotIndex, generation);

        SyrRecordPlayer_DestroyVinyl(recordPlayer, vinylId);
    }

    SYR_FREE(recordPlayer);
}
