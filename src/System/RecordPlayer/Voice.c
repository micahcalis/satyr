#include "Voice.h"
#include "VoiceInternal.h"

void SyrVoice_SetVolume(SyrVoice* voice, float volume)
{
    voice->volume = volume;
}

void SyrVoice_SetPitch(SyrVoice* voice, float pitch)
{
    voice->pitch = pitch;
}

void SyrVoice_SetPlaying(SyrVoice* voice, bool playing)
{
    voice->isPlaying = playing;
}

float SyrVoice_CalculateSampleLerp(const SyrVoice* voice, const uint8_t channel)
{
    if (!voice || !voice->pcmData || voice->totalFrames == 0 || voice->channels == 0)
        return 0.0f;

    double cursor = voice->cursorFrame;
    if (cursor < 0.0 || isnan(cursor))
    {
        cursor = 0.0;
    }

    uint64_t maxFrame = voice->totalFrames - 1;
    if (voice->frameSegmentEnd > 0 && voice->frameSegmentEnd <= voice->totalFrames)
    {
        maxFrame = voice->frameSegmentEnd - 1;
    }

    uint64_t i0 = (uint64_t)cursor;
    if (i0 > maxFrame)
    {
        i0 = maxFrame;
    }

    uint64_t i1 = (i0 < maxFrame) ? (i0 + 1) : maxFrame;

    float alpha = (float)(cursor - (double)i0);
    if (alpha < 0.0f)
        alpha = 0.0f;
    if (alpha > 1.0f)
        alpha = 1.0f;

    size_t idx0 = (size_t)i0 * voice->channels + channel;
    size_t idx1 = (size_t)i1 * voice->channels + channel;

    float s0 = voice->pcmData[idx0];
    float s1 = voice->pcmData[idx1];

    return SYR_MATH_LERP(s0, s1, alpha);
}
